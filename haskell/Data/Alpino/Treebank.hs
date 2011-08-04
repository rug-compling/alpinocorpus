module Data.Alpino.Treebank (
  open,
  entries
) where

import Data.Alpino.Treebank.Raw
import Foreign.C.String (newCString, peekCString)
import Foreign.ForeignPtr (ForeignPtr, unsafeForeignPtrToPtr, withForeignPtr)

import Control.Monad.IO.Class (MonadIO, liftIO)
import Data.Enumerator (Enumerator, Iteratee(..), Step(..), Stream(..), tryIO)

data Treebank = Treebank (ForeignPtr ())

open :: String -> IO (Maybe Treebank)
open fn = do
  cFn <- newCString fn
  r   <- c_alpinocorpus_open cFn
  return $ fmap Treebank r

entries :: MonadIO m => Treebank -> Enumerator String m b
entries (Treebank fPtr) step = do
  iter <- tryIO $ withForeignPtr fPtr c_alpinocorpus_entry_iter
  entries_ fPtr iter step

entries_ :: MonadIO m => ForeignPtr () -> CCorpusIter -> Enumerator String m b
entries_ fPtr iter =
  Iteratee . loop fPtr iter
  where
    loop ptr next (Continue k) = do
      case next of
        (Next _) -> do
          liftIO $ putStrLn "debug"
          val  <- liftIO $ iterToString next
          nnext <- liftIO $ c_alpinocorpus_iter_next (unsafeForeignPtrToPtr ptr) next
          runIteratee (k (Chunks [val])) >>= loop ptr nnext
        End      -> return $ Continue k
    loop _ _ step         = return step

iterToString :: CCorpusIter -> IO String
iterToString iter = do
  cStr <- c_alpinocorpus_iter_value iter
  withForeignPtr cStr peekCString
