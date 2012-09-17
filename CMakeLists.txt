cmake_minimum_required(VERSION 2.6)
project(AlpinoCorpus)

set (ALPINOCORPUS_VERSION "1.2.0")

if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release)
endif (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

if (NOT WIN32)
  set(CMAKE_CXX_FLAGS "-pthread -pedantic -Wno-long-long")
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

if(BOOST_ROOT)
  set(Boost_NO_SYSTEM_PATHS ON)
endif()

option(WITH_RUBY
  "Compile Ruby bindings" OFF)

option(USE_REMOTE_CORPUS "Enable remote corpus" ON)

if(USE_REMOTE_CORPUS)
find_package(Boost 1.46.1 COMPONENTS system date_time iostreams
  filesystem thread regex REQUIRED)
else()
find_package(Boost 1.46.1 COMPONENTS system date_time filesystem
  thread regex REQUIRED)
endif()
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

if(USE_REMOTE_CORPUS)
  find_package(OpenSSL)
  if (OPENSSL_FOUND)
    set(ALPINOCORPUS_WITH_SSL ON)
    option(ALPINOCORPUS_WITH_SSL_STRICT
 	"With SSL, enable domain verification" OFF)
  endif()
endif()

find_package(LibXslt REQUIRED)
if(LIBXSLT_FOUND)
  include_directories(${LIBXSLT_INCLUDE_DIR})
endif()

find_package(LibExslt REQUIRED)
if(LIBEXSLT_FOUND)
  include_directories(${LIBEXSLT_INCLUDE_DIR})
endif()


configure_file (
  "${AlpinoCorpus_SOURCE_DIR}/include/config.hh.in"
  "${AlpinoCorpus_BINARY_DIR}/include/config.hh"
)

configure_file (
  "${AlpinoCorpus_SOURCE_DIR}/alpinocorpus.pc.in"
  "${AlpinoCorpus_BINARY_DIR}/alpinocorpus.pc"
  @ONLY
)

set(HEADERS
  include/AlpinoCorpus/CompactCorpusWriter.hh
  include/AlpinoCorpus/CorpusReader.hh
  include/AlpinoCorpus/CorpusReaderFactory.hh
  include/AlpinoCorpus/CorpusWriter.hh
  include/AlpinoCorpus/DirectoryCorpusReader.hh
  include/AlpinoCorpus/DLLDefines.hh
  include/AlpinoCorpus/Error.hh
  include/AlpinoCorpus/IterImpl.hh
  include/AlpinoCorpus/CompactCorpusReader.hh
  include/AlpinoCorpus/LexItem.hh
  include/AlpinoCorpus/MultiCorpusReader.hh
  include/AlpinoCorpus/RecursiveCorpusReader.hh
  include/AlpinoCorpus/capi.h
  src/CompactCorpusWriterPrivate.hh
  src/DirectoryCorpusReaderPrivate.hh
  src/DzIstream.hh
  src/DzIstreamBuf.hh
  src/DzOstream.hh
  src/DzOstreamBuf.hh
  src/CompactCorpusReaderPrivate.hh
  src/FilterIter.hh
  src/MultiCorpusReaderPrivate.hh
  src/StylesheetIter.hh
  src/XSLTransformer.hh
  src/util/NameCompare.hh
  src/util/base64.hh
  src/util/bufutil.hh
  src/util/parseString.hh
  src/util/textfile.hh
  src/util/url.hh
  src/util/vs_stdint.h
)

set(SOURCES
  src/CompactCorpusWriter.cpp
  src/CompactCorpusWriterPrivate.cpp
  src/CorpusReader.cpp
  src/CorpusReaderFactory.cpp
  src/CorpusWriter.cpp
  src/DirectoryCorpusReader.cpp
  src/DirectoryCorpusReaderPrivate.cpp
  src/DzIstream.cpp
  src/DzIstreamBuf.cpp
  src/DzOstream.cpp
  src/DzOstreamBuf.cpp
  src/Error.cpp
  src/FilterIter.cpp
  src/CompactCorpusReader.cpp
  src/CompactCorpusReaderPrivate.cpp
  src/MultiCorpusReader.cpp
  src/MultiCorpusReaderPrivate.cpp
  src/RecursiveCorpusReader.cpp
  src/IterImpl.cpp
  src/StylesheetIter.cpp
  src/XSLTransformer.cpp
  src/capi.cpp
  src/util/NameCompare.cpp
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

if(USE_REMOTE_CORPUS)
  list(APPEND HEADERS
    include/AlpinoCorpus/RemoteCorpusReader.hh
    src/RemoteCorpusReaderPrivate.hh
    src/util/GetUrl.hh
  )
  list(APPEND SOURCES
    src/RemoteCorpusReader.cpp
    src/RemoteCorpusReaderPrivate.cpp
    src/util/GetUrl.cpp
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

if(USE_REMOTE_CORPUS)
  if(ALPINOCORPUS_WITH_SSL)
    target_link_libraries(alpino_corpus ${OPENSSL_LIBRARIES})
  endif(ALPINOCORPUS_WITH_SSL)
endif()

target_link_libraries(alpino_corpus ${LIBXSLT_LIBRARIES})
target_link_libraries(alpino_corpus ${LIBEXSLT_LIBRARIES})

install(TARGETS alpino_corpus
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(DIRECTORY include/AlpinoCorpus
  DESTINATION include
  FILES_MATCHING REGEX "^[^.].*\\.(hh|h)$")

install(FILES ${AlpinoCorpus_BINARY_DIR}/alpinocorpus.pc
  DESTINATION lib/pkgconfig)

# Unit tests

if(USE_DBXML)
  add_executable(writer_test test/writer.cpp)
  target_link_libraries(writer_test alpino_corpus)
  #add_test(writer writer_test)
endif()

if(USE_REMOTE_CORPUS)
  add_executable(geturl_test test/geturl.cpp)
  target_link_libraries(geturl_test alpino_corpus)
endif()

add_executable(readers_available test/readers_available.cpp)
target_link_libraries(readers_available alpino_corpus)

add_subdirectory(util)

if(WITH_RUBY)
  add_subdirectory(bindings/ruby)
endif()

add_subdirectory(resources)
