cmake_minimum_required(VERSION 3.1)
project(AlpinoCorpus)

set (ALPINOCORPUS_VERSION "2.6.0")

set (CMAKE_CXX_STANDARD 11)

if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release)
endif (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

if (NOT WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -pedantic -Wno-long-long")
endif(NOT WIN32)

LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
  SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
  SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
ENDIF("${isSystemDir}" STREQUAL "-1")

include(CheckFunctionExists)

if(APPLE)
  set (CMAKE_OSX_ARCHITECTURES "x86_64")
endif(APPLE)

list(APPEND CMAKE_MODULE_PATH "${AlpinoCorpus_SOURCE_DIR}/cmake")

include_directories(
  ${AlpinoCorpus_SOURCE_DIR}/include
  ${AlpinoCorpus_BINARY_DIR}/include
)

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

if(BOOST_ROOT)
  set(Boost_NO_SYSTEM_PATHS ON)
endif()

include(GNUInstallDirs REQUIRED)

find_package(Boost 1.50 COMPONENTS system chrono date_time filesystem
  thread regex REQUIRED)

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
  include/AlpinoCorpus/CorpusInfo.hh
  include/AlpinoCorpus/CorpusReader.hh
  include/AlpinoCorpus/CorpusReaderFactory.hh
  include/AlpinoCorpus/CorpusWriter.hh
  include/AlpinoCorpus/DirectoryCorpusReader.hh
  include/AlpinoCorpus/DLLDefines.hh
  include/AlpinoCorpus/Error.hh
  include/AlpinoCorpus/IterImpl.hh
  include/AlpinoCorpus/CompactCorpusReader.hh
  include/AlpinoCorpus/LexItem.hh
  include/AlpinoCorpus/capi.h
  include/AlpinoCorpus/macros.hh
  include/AlpinoCorpus/util/Either.hh
  src/CompactCorpusWriterPrivate.hh
  src/DirectoryCorpusReaderPrivate.hh
  src/DzIstream.hh
  src/DzIstreamBuf.hh
  src/DzOstream.hh
  src/DzOstreamBuf.hh
  src/CompactCorpusReaderPrivate.hh
  src/FilterIter.hh
  src/StylesheetIter.hh
  src/XSLTransformer.hh
  src/parseMacros.hh
  src/util/InfInt.h
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
  src/CorpusInfo.cpp
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
  src/IterImpl.cpp
  src/StylesheetIter.cpp
  src/XSLTransformer.cpp
  src/capi.cpp
  src/macros.cpp
  src/parseMacros.cpp
  src/util/NameCompare.cpp
  src/util/textfile.cpp
  src/util/url.cpp
)

if(USE_DBXML)
  list(APPEND HEADERS
    include/AlpinoCorpus/DbCorpusReader.hh
    include/AlpinoCorpus/DbCorpusWriter.hh
    include/AlpinoCorpus/MultiCorpusReader.hh
    include/AlpinoCorpus/RecursiveCorpusReader.hh
    src/DbCorpusReaderPrivate.hh
    src/MultiCorpusReaderPrivate.hh
  )

  list(APPEND SOURCES
    src/DbCorpusReader.cpp
    src/DbCorpusReaderPrivate.cpp
    src/DbCorpusWriter.cpp
    src/MultiCorpusReader.cpp
    src/MultiCorpusReaderPrivate.cpp
    src/RecursiveCorpusReader.cpp
  )
endif()

add_library(alpino_corpus SHARED
	${SOURCES}
	${HEADERS}
)

set_target_properties(alpino_corpus PROPERTIES VERSION ${ALPINOCORPUS_VERSION}
  SOVERSION 2.0)

target_link_libraries(alpino_corpus ${Boost_LIBRARIES})
target_link_libraries(alpino_corpus ${ZLIB_LIBRARIES})
target_link_libraries(alpino_corpus ${XERCESC_LIBRARY})
target_link_libraries(alpino_corpus ${XQILLA_LIBRARY})
if(USE_DBXML)
  target_link_libraries(alpino_corpus ${DBXML_LIBRARY})
endif(USE_DBXML)
target_link_libraries(alpino_corpus ${LIBXML2_LIBRARIES})

target_link_libraries(alpino_corpus ${LIBXSLT_LIBRARIES})
target_link_libraries(alpino_corpus ${LIBEXSLT_LIBRARIES})

install(TARGETS alpino_corpus
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/AlpinoCorpus
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING REGEX "^[^.].*\\.(hh|h)$")

install(FILES ${AlpinoCorpus_BINARY_DIR}/alpinocorpus.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

file(GLOB MANPAGES "${AlpinoCorpus_SOURCE_DIR}/man/*.1")
install(FILES ${MANPAGES} DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

# Unit tests

if(USE_DBXML)
  add_executable(writer_test test/writer.cpp)
  target_link_libraries(writer_test alpino_corpus)
  #add_test(writer writer_test)
endif()

add_executable(readers_available test/readers_available.cpp)
target_link_libraries(readers_available alpino_corpus)

add_subdirectory(util)

add_subdirectory(resources)
