{-# LANGUAGE DeriveDataTypeable #-}

-- |
-- Module      : Data.Alpino.Treebank
-- Copyright   : (c) 2011 Daniël de Kok
-- License     : LGPL
--
-- Maintainer  : Daniël de Kok <me@danieldk.eu>
-- Stability   : experimental
--
-- This module provides functions for processing Alpino three types
-- of Alpino treebanks:
--
-- * Directory-based treebanks
--
-- * Compact corpora
--
-- * DBXML-backed treebanks

module Data.Alpino.Treebank (
  -- * Types
  Treebank,
  MarkerQuery(..),
  TreebankException(..),

  -- * Opening treebanks
  open,

  -- * Entry enumerators
  enumEntries,
  enumQueryEntries,

  -- * Entry reading
  readEntry,
  readEntryMarkQuery
) where

import Control.Exception.Base (Exception)
import Data.ByteString (ByteString, packCString)
import Data.Alpino.Treebank.Raw
import Data.List (genericLength)
import Data.Typeable (Typeable)
import Foreign.C.String (newCString, peekCString)
import Foreign.ForeignPtr (ForeignPtr, unsafeForeignPtrToPtr, withForeignPtr)
import Foreign.Marshal.Alloc (free)
import Foreign.Marshal.Array (newArray)

import Control.Monad.IO.Class (MonadIO, liftIO)
import Data.Enumerator (Enumerator, Iteratee(..), Step(..), Stream(..),
  throwError, tryIO)

-- |
-- A treebank, the constructor for this type is opaque to hide its internals.
data Treebank = Treebank (ForeignPtr ())

-- |
-- A marker query is used in conjunction with 'readEntryMarkQuery'. The
-- 'MarkerQuery' data structure is used to specify which nodes should be
-- marked and how. A node is marked by adding an attribute-value pair.
data MarkerQuery = MarkerQuery {
  mqQuery     :: String, -- ^ XPath query specifying the nodes to mark
  mqAttribute :: String, -- ^ Attribute of the marker
  mqValue     :: String  -- ^ Value of the marker
} deriving (Show, Eq)

-- |
-- Exceptions that can occur while processing a treebank.
data TreebankException =
  IteratorException String -- ^ Iteration error
  deriving (Show, Typeable)

instance Exception TreebankException

-- |
-- Open a treebank.
open :: String -> IO (Either String Treebank)
open fn = do
  cFn <- newCString fn
  r   <- c_alpinocorpus_open cFn
  free cFn
  return $ fmap Treebank r

-- |
-- Enumerate the entries in a treebank.
enumEntries :: MonadIO m => Treebank -> Enumerator String m b
enumEntries (Treebank fPtr) step = do
  iterEither <- tryIO $ withForeignPtr fPtr c_alpinocorpus_entry_iter
  case iterEither of
    Right iter -> entries_ fPtr iter step
    Left  err  -> throwError $ IteratorException err

-- |
-- Enumerate the entries in a treebank matching the given query.
enumQueryEntries :: MonadIO m => Treebank -> String -> Enumerator String m b
enumQueryEntries (Treebank fPtr) query step = do
  queryC     <- liftIO $ newCString query
  iterEither <- tryIO $ withForeignPtr fPtr (flip c_alpinocorpus_query_iter $ queryC)
  tryIO $ free queryC
  case iterEither of
    Right iter -> entries_ fPtr iter step
    Left  err  -> throwError $ IteratorException err


entries_ :: MonadIO m => ForeignPtr () -> CCorpusIter -> Enumerator String m b
entries_ fPtr iter =
  Iteratee . loop fPtr iter
  where
    loop ptr next (Continue k) = do
      case next of
        (Next _) -> do
          val  <- liftIO $ iterToString next
          nnext <- liftIO $ c_alpinocorpus_iter_next (unsafeForeignPtrToPtr ptr) next
          runIteratee (k (Chunks [val])) >>= loop ptr nnext
        End      -> return $ Continue k
    loop _ next step           = do
      liftIO $ c_alpinocorpus_iter_destroy next
      return step

iterToString :: CCorpusIter -> IO String
iterToString iter = do
  cStr <- c_alpinocorpus_iter_value iter
  withForeignPtr cStr peekCString

-- |
-- Read an entry from a treebank.
readEntry :: Treebank -> String -> IO (Either String ByteString)
readEntry (Treebank t) entry = do
  entryC         <- newCString entry
  contentsEither <- withForeignPtr t (flip c_alpinocorpus_read $ entryC)
  free entryC
  case contentsEither of
    Left err       -> return $ Left err
    Right contents -> do
      bs <- withForeignPtr contents packCString
      return $ Right bs

-- |
-- Retrieve an entry from a treebank, marking nodes that match one of the
-- given queries.
readEntryMarkQuery :: Treebank -> String -> [MarkerQuery] ->
  IO (Either String ByteString)
readEntryMarkQuery (Treebank t) entry queries = do
  entryC   <- newCString entry
  cQueries <- mapM queryToCQuery queries
  cQueriesPtr <- newArray cQueries
  contentsEither <- withForeignPtr t (\tb ->
    c_alpinocorpus_read_mark_queries tb entryC cQueriesPtr (genericLength cQueries))
  mapM_ freeCQuery cQueries
  free cQueriesPtr
  free entryC
  case contentsEither of
    Left err       -> return $ Left err
    Right contents -> do
      bs <- withForeignPtr contents packCString
      return $ Right bs
  where
    queryToCQuery (MarkerQuery q a v) = do
      qC <- newCString q
      qA <- newCString a
      qV <- newCString v
      return $ CMarkerQuery qC qA qV
    freeCQuery (CMarkerQuery q a v) = do
      free q
      free a
      free v
