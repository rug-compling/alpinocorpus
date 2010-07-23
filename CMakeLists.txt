cmake_minimum_required(VERSION 2.6)
project(IndexedCorpus)

if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release)
endif (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

include_directories(${IndexedCorpus_SOURCE_DIR})

find_package(Qt4 COMPONENTS QtCore REQUIRED)
INCLUDE(${QT_USE_FILE})

find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
  include_directories(${ZLIB_INCLUDE_DIRS})
endif()

set (HEADERS
  IndexedCorpus/CorpusReader.hh
  IndexedCorpus/dictzip.hh
  IndexedCorpus/DictZipFile.hh
  IndexedCorpus/DirectoryCorpusReader.hh
  IndexedCorpus/DLLDefines.hh
  IndexedCorpus/DzChunk.hh
  IndexedCorpus/fdstream.hh
  IndexedCorpus/gzip.hh
  IndexedCorpus/IndexedCorpusReader.hh
  IndexedCorpus/IndexedCorpusWriter.hh
  IndexedCorpus/IndexNamePair.hh
  IndexedCorpus/NullStream.hh
  IndexedCorpus/util/base64.hh
  IndexedCorpus/util/bufutil.hh
  IndexedCorpus/util/stringutil.hh
  IndexedCorpus/util/textfile.hh
)

set (SOURCES
  src/CorpusReader/CorpusReader.cpp
  src/DictZipFile/DictZipFile.cpp
  src/DirectoryCorpusReader/DirectoryCorpusReader.cpp
  src/IndexedCorpusReader/IndexedCorpusReader.cpp
  #src/IndexedCorpusWriter/IndexedCorpusWriter.cpp
  src/util/textfile/textfile.cpp
  src/util/base64.cpp
  src/IndexNamePair/IndexNamePair.cpp
)

add_library(corpus SHARED 
	${SOURCES}
	${HEADERS}
)

target_link_libraries(corpus ${QT_LIBRARIES})
target_link_libraries(corpus ${ZLIB_LIBRARIES})

install(TARGETS corpus
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(DIRECTORY IndexedCorpus 
  DESTINATION include)
