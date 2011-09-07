cmake_minimum_required(VERSION 2.6)
project(AlpinoCorpus)

set (ALPINOCORPUS_VERSION "0.9.9")

if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release)
endif (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_FLAGS "-pthread")

include(CheckFunctionExists)

if(APPLE)
  set (CMAKE_OSX_ARCHITECTURES "x86_64")
  set (CMAKE_OSX_DEPLOYMENT_TARGET "10.5")
endif(APPLE)

list(APPEND CMAKE_MODULE_PATH "${AlpinoCorpus_SOURCE_DIR}/cmake")

include_directories(${AlpinoCorpus_SOURCE_DIR}/include)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.46.1 COMPONENTS system filesystem thread REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
  include_directories(${ZLIB_INCLUDE_DIRS})
endif()

find_package(XercesC REQUIRED)
if (XERCESC_FOUND)
  include_directories(${XERCESC_INCLUDE_DIR})
endif()

find_package(XQilla REQUIRED)
if (XQILLA_FOUND)
  include_directories(${XQILLA_INCLUDE_DIR})
endif()

find_package(DBXML REQUIRED)
if (DBXML_FOUND)
  include_directories(${DBXML_INCLUDE_DIR})
endif()

find_package(Iconv REQUIRED)
if(ICONV_FOUND)
  include_directories(${ICONV_INCLUDE_DIR})
endif()

find_package(LibXml2 REQUIRED)
if(LIBXML2_FOUND)
  include_directories(${LIBXML2_INCLUDE_DIR})
endif()

CHECK_FUNCTION_EXISTS(strlcpy HAVE_STRLCPY)

set (HEADERS
  include/AlpinoCorpus/CorpusReader.hh
  include/AlpinoCorpus/CorpusWriter.hh
  include/AlpinoCorpus/DbCorpusReader.hh
  include/AlpinoCorpus/DbCorpusWriter.hh
  include/AlpinoCorpus/DirectoryCorpusReader.hh
  include/AlpinoCorpus/DzIstream.hh
  include/AlpinoCorpus/DzIstreamBuf.hh
  include/AlpinoCorpus/DLLDefines.hh
  include/AlpinoCorpus/Error.hh
  include/AlpinoCorpus/IndexedCorpusReader.hh
  #include/AlpinoCorpus/IndexedCorpusWriter.hh
  include/AlpinoCorpus/MultiCorpusReader.hh
  include/AlpinoCorpus/util/NonCopyable.hh
  include/AlpinoCorpus/capi.h
  include/util/base64.hh
  include/util/bufutil.hh
  include/util/textfile.hh
  include/util/url.hh
  include/util/vs_stdint.h
  src/DirectoryCorpusReaderPrivate.hh
  src/IndexedCorpusReaderPrivate.hh
  src/gzip.hh
)

set(SOURCES
  src/CorpusReader.cpp
  src/CorpusWriter.cpp
  src/DbCorpusReader.cpp
  src/DbCorpusWriter.cpp
  src/DirectoryCorpusReader.cpp
  src/DirectoryCorpusReaderPrivate.cpp
  src/DzIstream.cpp
  src/DzIstreamBuf.cpp
  src/Error.cpp
  src/IndexedCorpusReader.cpp
  src/IndexedCorpusReaderPrivate.cpp
  #src/IndexedCorpusWriter.cpp
  src/MultiCorpusReader.cpp
  src/capi.cpp
  src/util/base64.cpp
  src/util/textfile.cpp
  src/util/url.cpp
)

set(ACT_SOURCES
  util/act/ProgramOptions.cpp
  util/act/main.cpp
)

set(ACT_HEADERS
  util/act/ProgramOptions.hh
)

set(MERGE_SOURCES
  src/merge/merge.cpp
)

add_library(alpino_corpus SHARED 
	${SOURCES}
	${HEADERS}
)

set_target_properties(alpino_corpus PROPERTIES VERSION ${ALPINOCORPUS_VERSION}
  SOVERSION 1)

target_link_libraries(alpino_corpus ${Boost_LIBRARIES})
target_link_libraries(alpino_corpus ${ZLIB_LIBRARIES})
target_link_libraries(alpino_corpus ${XERCESC_LIBRARY})
target_link_libraries(alpino_corpus ${XQILLA_LIBRARY})
target_link_libraries(alpino_corpus ${DBXML_LIBRARY})
target_link_libraries(alpino_corpus ${LIBXML2_LIBRARIES})

add_executable(alpinocorpus-act ${ACT_SOURCES} ${ACT_HEADERS})
target_link_libraries(alpinocorpus-act alpino_corpus)

add_executable(alpinocorpus-merge ${MERGE_SOURCES})
target_link_libraries(alpinocorpus-merge alpino_corpus)

install(TARGETS alpino_corpus
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(DIRECTORY include/AlpinoCorpus
  DESTINATION include)

# Unit tests

add_executable(writer_test test/writer.cpp ${writer_test_MOC_SRCS})
target_link_libraries(writer_test alpino_corpus)
#add_test(writer writer_test)
