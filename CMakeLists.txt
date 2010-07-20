cmake_minimum_required(VERSION 2.6)
project(INDEXEDCORPUS)

if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release)
endif (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

include_directories(${INDEXEDCORPUS_SOURCE_DIR})

find_package(Qt4 COMPONENTS QtCore REQUIRED)
INCLUDE(${QT_USE_FILE})

find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
  include_directories(${ZLIB_INCLUDE_DIRS})
endif()

add_library(corpus SHARED 
  src/ActCorpusReader/ActCorpusReader.cpp
  src/DictZipFile/DictZipFile.cpp
  src/IndexedCorpusReader/IndexedCorpusReader.cpp
  #src/IndexedCorpusWriter/IndexedCorpusWriter.cpp
  src/util/textfile/textfile.cpp
  src/util/base64.cpp
  src/IndexNamePair/IndexNamePair.cpp
)

target_link_libraries(corpus ${QT_LIBRARIES})
target_link_libraries(corpus ${ZLIB_LIBRARIES})

