require 'ffi'

module EntriesIterate
  def entriesIterate(ptrPtr)
    while !ptrPtr.get_pointer(0).null? do
      # Retrieve iterate value as a pointer, we have to free it.
      iter = ptrPtr.get_pointer(0)
      strPtr = AlpinoCorpus::alpinocorpus_iter_value(iter)

      # Convert and yield the string.
      str = strPtr.get_string(0)
      yield str

      # Free the C string.
      AlpinoCorpusLibC::free(strPtr)

      AlpinoCorpus::alpinocorpus_iter_next(@reader, ptrPtr)
    end
  end
end


module AlpinoCorpus
  extend FFI::Library

  ffi_lib 'alpino_corpus'

  attach_function :alpinocorpus_open, [:string], :pointer
  attach_function :alpinocorpus_close, [:pointer], :void
  attach_function :alpinocorpus_read, [:pointer, :string], :pointer
  attach_function :alpinocorpus_entry_iter, [:pointer], :pointer
  attach_function :alpinocorpus_query_iter, [:pointer, :string], :pointer
  attach_function :alpinocorpus_iter_next, [:pointer, :pointer], :void
  attach_function :alpinocorpus_iter_value, [:pointer], :pointer

  class AlpinoCorpusException < Exception
  end

  class Reader
    include Enumerable
    include EntriesIterate

    def initialize(path)
      @reader = AlpinoCorpus::alpinocorpus_open(path)

      if @reader.null?
        raise AlpinoCorpusException, "Could not open corpus."
      end

      ObjectSpace.define_finalizer(self, self.class.finalize(@reader))
    end

    def each(&blk)
      iter = AlpinoCorpus::alpinocorpus_entry_iter(@reader)
      ptrPtr = FFI::MemoryPointer.new(:pointer, 1)
      ptrPtr.put_pointer(0, iter)

      entriesIterate(ptrPtr, &blk)

      self
    end

    def read(name)
      strPtr = AlpinoCorpus::alpinocorpus_read(@reader, name)
      
      if strPtr.null?
        raise AlpinoCorpusException, "Could not read entry."
      end

      str = strPtr.get_string(0)

      AlpinoCorpusLibC::free(strPtr)

      str
    end


    def query(query)
      Query.new(@reader, query)
    end

    def self.finalize(reader)
      proc { AlpinoCorpus::alpinocorpus_close(reader) }
    end
  end

  class Query
    include Enumerable
    include EntriesIterate

    def initialize(reader, query)
      @reader = reader
      @query = query
    end

    def each(&blk)
      iter = AlpinoCorpus::alpinocorpus_query_iter(@reader, @query)
      if iter.null?
        raise AlpinoCorpusException, "Could not execute query."
      end

      ptrPtr = FFI::MemoryPointer.new(:pointer, 1)
      ptrPtr.put_pointer(0, iter)

      entriesIterate(ptrPtr, &blk)

      self
    end
  end
end

private

module AlpinoCorpusLibC
    extend FFI::Library
    ffi_lib FFI::Library::LIBC

    attach_function :free, [:pointer], :void
end
