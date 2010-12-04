cmake_minimum_required(VERSION 2.6)
project(AlpinoCorpus)

if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release)
endif (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

if(APPLE)
  set (CMAKE_OSX_ARCHITECTURES "i386;x86_64")
  set (CMAKE_OSX_DEPLOYMENT_TARGET "10.5")
endif(APPLE)

list(APPEND CMAKE_MODULE_PATH "${AlpinoCorpus_SOURCE_DIR}/cmake")

include_directories(${AlpinoCorpus_SOURCE_DIR}/include)

find_package(Qt4 COMPONENTS QtCore REQUIRED)
INCLUDE(${QT_USE_FILE})

find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
  include_directories(${ZLIB_INCLUDE_DIRS})
endif()

find_package(QDictZip REQUIRED)
if(QDICTZIP_FOUND)
  include_directories(${QDICTZIP_INCLUDE_DIR})
endif()

find_package(DBXML REQUIRED)
if (DBXML_FOUND)
  include_directories(${DBXML_INCLUDE_DIR})
endif()

set (HEADERS
  include/AlpinoCorpus/CorpusReader.hh
  include/AlpinoCorpus/DbCorpusReader.hh
  include/AlpinoCorpus/DirectoryCorpusReader.hh
  include/AlpinoCorpus/DLLDefines.hh
  include/AlpinoCorpus/DzChunk.hh
  include/AlpinoCorpus/gzip.hh
  include/AlpinoCorpus/IndexedCorpusReader.hh
  include/AlpinoCorpus/IndexedCorpusWriter.hh
  include/AlpinoCorpus/IndexNamePair.hh
  include/AlpinoCorpus/NullStream.hh
  include/AlpinoCorpus/util/base64.hh
  include/AlpinoCorpus/util/bufutil.hh
  include/AlpinoCorpus/util/stringutil.hh
  include/AlpinoCorpus/util/textfile.hh
)

set (SOURCES
  src/CorpusReader.cpp
  src/DbCorpusReader.cpp
  src/DirectoryCorpusReader.cpp
  src/IndexedCorpusReader.cpp
  #src/IndexedCorpusWriter.cpp
  src/util/textfile.cpp
  src/util/base64.cpp
  src/IndexNamePair.cpp
)

add_library(alpino_corpus SHARED 
	${SOURCES}
	${HEADERS}
)

target_link_libraries(alpino_corpus ${QT_LIBRARIES})
target_link_libraries(alpino_corpus ${QDICTZIP_LIBRARIES})

install(TARGETS alpino_corpus
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(DIRECTORY include/AlpinoCorpus
  DESTINATION include)
