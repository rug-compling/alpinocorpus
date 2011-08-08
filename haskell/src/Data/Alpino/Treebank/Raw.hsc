{-# LANGUAGE ForeignFunctionInterface, DoAndIfThenElse #-}

-- |
-- Module      : Data.Alpino.Treebank.Raw
-- Copyright   : (c) 2011 Daniël de Kok
-- License     : LGPL
--
-- Maintainer  : Daniël de Kok <me@danieldk.eu>
-- Stability   : experimental
--
-- Low-level wrapper for the alpinocorpus library. This library provides
-- access to various types of Alpino treebanks:
--
-- * Directory-based treebanks
--
-- * Compact corpora
--
-- * DBXML-backed treebanks
--
-- It is not recommended to use this module directly, please consider
-- using the "Data.Alpino.Treebank" module instead.

#include <AlpinoCorpus/capi.h>

#let alignment t = "%lu", (unsigned long)offsetof(struct {char x__; t (y__); }, y__)

module Data.Alpino.Treebank.Raw (
  c_alpinocorpus_open,
  CCorpusIter(..),
  CMarkerQuery(..),
  c_alpinocorpus_entry_iter,
  c_alpinocorpus_query_iter,
  c_alpinocorpus_iter_destroy,
  c_alpinocorpus_iter_value,
  c_alpinocorpus_iter_next,
  c_alpinocorpus_read,
  c_alpinocorpus_read_mark_queries
) where

import Foreign.C.String (CString)
import Foreign.C.Types (CChar, CSize)
import Foreign.ForeignPtr (ForeignPtr, newForeignPtr)
import Foreign.Ptr (FunPtr, Ptr, nullPtr)
import Foreign.Storable (Storable(..))

-- |
-- Iterator over a corpus.
data CCorpusIter =
    Next (Ptr ()) -- ^ Valid iterator
  | End           -- ^ End iterator

data CMarkerQuery = CMarkerQuery {
  cMqQuery      :: CString,
  cMqAttribute  :: CString,
  cMqvalue      :: CString
} deriving Show

instance Storable CMarkerQuery where
  sizeOf _    = #{size marker_query_t}
  alignment _ = #{alignment marker_query_t}
  peek ptr    = do
    query     <- (#peek marker_query_t, query) ptr
    attribute <- (#peek marker_query_t, attr) ptr
    value     <- (#peek marker_query_t, value) ptr
    return $ CMarkerQuery query attribute value
  poke ptr (CMarkerQuery query attribute value) = do
    (#poke marker_query_t, query) ptr query
    (#poke marker_query_t, attr) ptr attribute
    (#poke marker_query_t, value) ptr value

foreign import ccall "stdlib.h &free"
  p_free :: FunPtr (Ptr a -> IO ())

foreign import ccall unsafe "AlpinoCorpus/capi.h alpinocorpus_open"
  c_alpinocorpus_open_ :: CString -> IO (Ptr ())

foreign import ccall unsafe "AlpinoCorpus/capi.h &alpinocorpus_close"
  p_alpinocorpus_close_ :: FunPtr (Ptr () -> IO ())

-- |
-- Open a corpus, the corpus handle is an opaque pointer. 
c_alpinocorpus_open :: CString -> IO (Either String (ForeignPtr ()))
c_alpinocorpus_open filename = do
  ptr <- c_alpinocorpus_open_ filename
  if ptr /= nullPtr then do
    fPtr <- newForeignPtr p_alpinocorpus_close_ ptr
    return $ Right fPtr
  else
    return $ Left "Could not open treebank." 

foreign import ccall unsafe "AlpinoCorpus/capi.h alpinocorpus_entry_iter"
  c_alpinocorpus_entry_iter_ :: Ptr () -> IO (Ptr ())

-- |
-- Create an iterator over the corpus.
c_alpinocorpus_entry_iter :: Ptr () -> IO (Either String CCorpusIter)
c_alpinocorpus_entry_iter corpus = do
  ptr <- c_alpinocorpus_entry_iter_ corpus
  if ptr /= nullPtr then
    return $ Right $ Next ptr
  else
    return $ Left "Could not iterate over the corpus."

foreign import ccall unsafe "AlpinoCorpus/capi.h alpinocorpus_query_iter"
  c_alpinocorpus_query_iter_ :: Ptr () -> CString -> IO (Ptr ())

-- |
-- Get an iterator over the entries in a corpus.
c_alpinocorpus_query_iter :: Ptr () -> CString -> IO (Either String CCorpusIter)
c_alpinocorpus_query_iter corpus query = do
  ptr <- c_alpinocorpus_query_iter_ corpus query
  if ptr /= nullPtr then
    return $ Right $ Next ptr
  else
    return $ Left "Could not execute query, or invalid iterator."

foreign import ccall unsafe "AlpinoCorpus/capi.h alpinocorpus_iter_destroy"
  c_alpinocorpus_iter_destroy_ :: Ptr () -> IO ()

-- |
-- Destroy an iterator, freeing its resources.
c_alpinocorpus_iter_destroy :: CCorpusIter -> IO ()
c_alpinocorpus_iter_destroy (Next iter) = do
  c_alpinocorpus_iter_destroy_ iter
c_alpinocorpus_iter_destroy (End)       =
  return ()

foreign import ccall unsafe "AlpinoCorpus/capi.h alpinocorpus_iter_value"
  c_alpinocorpus_iter_value_ :: Ptr () -> IO CString

-- |
-- Get the value of an iterator.
c_alpinocorpus_iter_value :: CCorpusIter -> IO (ForeignPtr CChar)
c_alpinocorpus_iter_value (Next iter) = do
  strp <- c_alpinocorpus_iter_value_ iter
  newForeignPtr p_free strp
c_alpinocorpus_iter_value (End)       =
  error "Cannot get the value of an invalid iterator"

foreign import ccall unsafe "AlpinoCorpus/capi.h alpinocorpus_iter_next"
  c_alpinocorpus_iter_next_ :: Ptr () -> Ptr () -> IO (Ptr ())

-- |
-- Increment the iterator, returns `End` when iteration is finished.
c_alpinocorpus_iter_next :: Ptr () -> CCorpusIter -> IO CCorpusIter
c_alpinocorpus_iter_next corpus (Next iter) = do
  newIter <- c_alpinocorpus_iter_next_ corpus iter
  if newIter /= nullPtr then
    return $ Next newIter
  else
    return End
c_alpinocorpus_iter_next _      End         = return End

foreign import ccall unsafe "AlpinoCorpus/capi.h alpinocorpus_read"
  c_alpinocorpus_read_ :: Ptr () -> CString -> IO CString

-- |
-- Read an entry from a corpus.
c_alpinocorpus_read :: Ptr () -> CString -> IO (Either String (ForeignPtr CChar))
c_alpinocorpus_read corpus entry = do
  strp <- c_alpinocorpus_read_ corpus entry 
  if strp /= nullPtr then do
    strfp <- newForeignPtr p_free strp
    return $ Right strfp 
  else
    return $ Left "Could not read entry."

foreign import ccall unsafe "AlpinoCorpus/capi.h> alpinocorpus_read_mark_queries"
  c_alpinocorpus_read_mark_queries_ :: Ptr () -> CString ->
    Ptr (CMarkerQuery) -> CSize -> IO CString

c_alpinocorpus_read_mark_queries :: Ptr () -> CString -> Ptr (CMarkerQuery) ->
  CSize -> IO (Either String (ForeignPtr CChar))
c_alpinocorpus_read_mark_queries corpus entry queries nQueries = do
  strp <- c_alpinocorpus_read_mark_queries_ corpus entry queries nQueries
  if strp /= nullPtr then do
    strfp <- newForeignPtr p_free strp
    return $ Right strfp
  else
    return $ Left "Could not read entry or process query."
