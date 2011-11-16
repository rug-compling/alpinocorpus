cmake_minimum_required(VERSION 2.6)
project(AlpinoCorpus)

set (ALPINOCORPUS_VERSION "0.9.9")

if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release)
endif (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

if (NOT WIN32)
  set(CMAKE_CXX_FLAGS "-pthread -pedantic")
endif(NOT WIN32)

LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
  SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
ENDIF("${isSystemDir}" STREQUAL "-1")

include(CheckFunctionExists)

if(APPLE)
  set (CMAKE_OSX_ARCHITECTURES "x86_64")
  set (CMAKE_OSX_DEPLOYMENT_TARGET "10.5")
endif(APPLE)

list(APPEND CMAKE_MODULE_PATH "${AlpinoCorpus_SOURCE_DIR}/cmake")

include_directories(
  ${AlpinoCorpus_SOURCE_DIR}/include
  ${AlpinoCorpus_BINARY_DIR}/include
)

if (APPLE OR WIN32)
  set(Boost_USE_STATIC_LIBS ON)
else(APPLE OR WIN32)
  set(Boost_USE_STATIC_LIBS OFF)
endif(APPLE OR WIN32)

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

option(WITH_PYTHON
  "Compile Python bindings" OFF)

option(WITH_RUBY
  "Compile Ruby bindings" OFF)

find_package(Boost 1.46.1 COMPONENTS system date_time
  filesystem thread REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
  include_directories(${ZLIB_INCLUDE_DIRS})
endif()

option(USE_DBXML
  "Enable the Berkeley DB XML backend" ON)

if(USE_DBXML)
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
endif()

find_package(Iconv REQUIRED)
if(ICONV_FOUND)
  include_directories(${ICONV_INCLUDE_DIR})
endif()

find_package(LibXml2 REQUIRED)
if(LIBXML2_FOUND)
  include_directories(${LIBXML2_INCLUDE_DIR})
endif()

find_package(OpenSSL)
if (OPENSSL_FOUND)
  set(WITH_SSL ON)
  option(WITH_SSL_STRICT
	"With SSL, enable domain verification" OFF)
endif()

CHECK_FUNCTION_EXISTS(strlcpy HAVE_STRLCPY)

configure_file (
  "${AlpinoCorpus_SOURCE_DIR}/include/config.hh.in"
  "${AlpinoCorpus_BINARY_DIR}/include/config.hh"
)

set(HEADERS
  include/AlpinoCorpus/CompactCorpusWriter.hh
  include/AlpinoCorpus/CorpusReader.hh
  include/AlpinoCorpus/CorpusWriter.hh
  include/AlpinoCorpus/DirectoryCorpusReader.hh
  include/AlpinoCorpus/DLLDefines.hh
  include/AlpinoCorpus/Error.hh
  include/AlpinoCorpus/CompactCorpusReader.hh
  include/AlpinoCorpus/MultiCorpusReader.hh
  include/AlpinoCorpus/RecursiveCorpusReader.hh
  include/AlpinoCorpus/capi.h
  include/AlpinoCorpus/util/GetUrl.hh
  include/AlpinoCorpus/util/NonCopyable.hh
  include/util/base64.hh
  include/util/bufutil.hh
  include/util/textfile.hh
  include/util/url.hh
  include/util/vs_stdint.h
  src/CompactCorpusWriterPrivate.hh
  src/DirectoryCorpusReaderPrivate.hh
  src/DzIstream.hh
  src/DzIstreamBuf.hh
  src/DzOstream.hh
  src/DzOstreamBuf.hh
  src/CompactCorpusReaderPrivate.hh
  src/MultiCorpusReaderPrivate.hh
  src/gzip.hh
)

set(SOURCES
  src/CompactCorpusWriter.cpp
  src/CompactCorpusWriterPrivate.cpp
  src/CorpusReader.cpp
  src/CorpusWriter.cpp
  src/DirectoryCorpusReader.cpp
  src/DirectoryCorpusReaderPrivate.cpp
  src/DzIstream.cpp
  src/DzIstreamBuf.cpp
  src/DzOstream.cpp
  src/DzOstreamBuf.cpp
  src/Error.cpp
  src/CompactCorpusReader.cpp
  src/CompactCorpusReaderPrivate.cpp
  src/util/GetUrl.cpp
  src/MultiCorpusReader.cpp
  src/MultiCorpusReaderPrivate.cpp
  src/RecursiveCorpusReader.cpp
  src/capi.cpp
  src/util/base64.cpp
  src/util/textfile.cpp
  src/util/url.cpp
)

if(USE_DBXML)
  list(APPEND HEADERS
    include/AlpinoCorpus/DbCorpusReader.hh
    include/AlpinoCorpus/DbCorpusWriter.hh
    src/DbCorpusReaderPrivate.hh
  )

  list(APPEND SOURCES
    src/DbCorpusReader.cpp
    src/DbCorpusReaderPrivate.cpp
    src/DbCorpusWriter.cpp
  )
endif()

add_library(alpino_corpus SHARED 
	${SOURCES}
	${HEADERS}
)

set_target_properties(alpino_corpus PROPERTIES VERSION ${ALPINOCORPUS_VERSION}
  SOVERSION 1.0)

target_link_libraries(alpino_corpus ${Boost_LIBRARIES})
target_link_libraries(alpino_corpus ${ZLIB_LIBRARIES})
if(USE_DBXML)
  target_link_libraries(alpino_corpus ${XERCESC_LIBRARY})
  target_link_libraries(alpino_corpus ${XQILLA_LIBRARY})
  target_link_libraries(alpino_corpus ${DBXML_LIBRARY})
endif(USE_DBXML)
target_link_libraries(alpino_corpus ${LIBXML2_LIBRARIES})
if(WITH_SSL)
  target_link_libraries(alpino_corpus ${OPENSSL_LIBRARIES})
endif(WITH_SSL)


install(TARGETS alpino_corpus
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(DIRECTORY include/AlpinoCorpus
  DESTINATION include
  FILES_MATCHING REGEX "^[^.].*\\.(hh|h)$")

# Unit tests

if(USE_DBXML)
  add_executable(writer_test test/writer.cpp)
  target_link_libraries(writer_test alpino_corpus)
  #add_test(writer writer_test)
endif()

add_executable(geturl_test test/geturl.cpp)
target_link_libraries(geturl_test alpino_corpus)

add_subdirectory(util)

if(WITH_PYTHON)
  add_subdirectory(bindings/python)
endif()

if(WITH_RUBY)
  add_subdirectory(bindings/ruby)
endif()

add_subdirectory(resources)
