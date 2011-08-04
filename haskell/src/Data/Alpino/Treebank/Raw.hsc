{-# LANGUAGE ForeignFunctionInterface, DoAndIfThenElse #-}

#include <AlpinoCorpus/capi.h>

module Data.Alpino.Treebank.Raw (
  c_alpinocorpus_open,
  CCorpusIter(..),
  c_alpinocorpus_entry_iter,
  c_alpinocorpus_iter_value,
  c_alpinocorpus_iter_next
) where

import Foreign.C.String (CString)
import Foreign.C.Types (CChar)
import Foreign.ForeignPtr (ForeignPtr, newForeignPtr)
import Foreign.Ptr (FunPtr, Ptr, nullPtr)


data CCorpusIter = Next (Ptr ()) | End

foreign import ccall "stdlib.h &free"
  p_free :: FunPtr (Ptr a -> IO ())

foreign import ccall unsafe "AlpinoCorpus/capi.h alpinocorpus_open"
  c_alpinocorpus_open_ :: CString -> IO (Ptr ())

foreign import ccall unsafe "AlpinoCorpus/capi.h &alpinocorpus_close"
  p_alpinocorpus_close_ :: FunPtr (Ptr () -> IO ())

-- |
-- Open a corpus, the corpus handle is an opaque pointer. 
c_alpinocorpus_open :: CString -> IO (Maybe (ForeignPtr ()))
c_alpinocorpus_open filename = do
  ptr <- c_alpinocorpus_open_ filename
  if ptr /= nullPtr then do
    fPtr <- newForeignPtr p_alpinocorpus_close_ ptr
    return $ Just fPtr
  else
    return Nothing

foreign import ccall unsafe "AlpinoCorpus/capi.h alpinocorpus_entry_iter"
  c_alpinocorpus_entry_iter_ :: Ptr () -> IO (Ptr ())

-- |
-- Create an iterator over the corpus.
c_alpinocorpus_entry_iter :: Ptr () -> IO CCorpusIter
c_alpinocorpus_entry_iter corpus = do
  ptr <- c_alpinocorpus_entry_iter_ corpus
  if ptr /= nullPtr then
    return $ Next ptr
  else
    return End

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
-- Increment the iterator, returns `Nothing` when iteration is finished.
c_alpinocorpus_iter_next :: Ptr () -> CCorpusIter -> IO CCorpusIter
c_alpinocorpus_iter_next corpus (Next iter) = do
  newIter <- c_alpinocorpus_iter_next_ corpus iter
  if newIter /= nullPtr then
    return $ Next newIter
  else
    return End
c_alpinocorpus_iter_next _      End         = return End
