cmake_minimum_required(VERSION 2.6)
project(AlpinoCorpus)

set (ALPINOCORPUS_VERSION "0.0.1")

if (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release)
endif (NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_FLAGS "-pthread")

if(APPLE)
  set (CMAKE_OSX_ARCHITECTURES "x86_64")
  set (CMAKE_OSX_DEPLOYMENT_TARGET "10.5")
endif(APPLE)

list(APPEND CMAKE_MODULE_PATH "${AlpinoCorpus_SOURCE_DIR}/cmake")

include_directories(${AlpinoCorpus_SOURCE_DIR}/include)

find_package(Qt4 COMPONENTS QtCore QtTest REQUIRED)
INCLUDE(${QT_USE_FILE})

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

set (HEADERS
  include/AlpinoCorpus/CorpusReader.hh
  include/AlpinoCorpus/CorpusWriter.hh
  include/AlpinoCorpus/DbCorpusReader.hh
  include/AlpinoCorpus/DbCorpusWriter.hh
  include/AlpinoCorpus/DirectoryCorpusReader.hh
  include/AlpinoCorpus/DLLDefines.hh
  include/AlpinoCorpus/DzChunk.hh
  include/AlpinoCorpus/Error.hh
  include/AlpinoCorpus/IndexedCorpusReader.hh
  #include/AlpinoCorpus/IndexedCorpusWriter.hh
  include/AlpinoCorpus/QDictZipFile.hh
  include/AlpinoCorpus/util/NonCopyable.hh
  include/AlpinoCorpus/util/base64.hh
  include/AlpinoCorpus/util/textfile.hh
)

set(SOURCES
  src/CorpusReader.cpp
  src/DbCorpusReader.cpp
  src/DbCorpusWriter.cpp
  src/DirectoryCorpusReader.cpp
  src/Error.cpp
  src/IndexedCorpusReader.cpp
  #src/IndexedCorpusWriter.cpp
  src/QDictZipFile.cpp
  src/util/textfile.cpp
  src/util/base64.cpp
)

add_library(alpino_corpus SHARED 
	${SOURCES}
	${HEADERS}
)

set_target_properties(alpino_corpus PROPERTIES VERSION ${ALPINOCORPUS_VERSION}
  SOVERSION 0)

target_link_libraries(alpino_corpus ${QT_LIBRARIES})
target_link_libraries(alpino_corpus ${ZLIB_LIBRARIES})
target_link_libraries(alpino_corpus ${XERCESC_LIBRARY})
target_link_libraries(alpino_corpus ${XQILLA_LIBRARY})
target_link_libraries(alpino_corpus ${DBXML_LIBRARY})
target_link_libraries(alpino_corpus ${LIBXML2_LIBRARIES})

add_executable(alpino2db src/alpino2db/main.cpp)
target_link_libraries(alpino2db alpino_corpus)
target_link_libraries(alpino2db ${QT_LIBRARIES})

install(TARGETS alpino_corpus
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(DIRECTORY include/AlpinoCorpus
  DESTINATION include)

# Unit tests

qt4_wrap_cpp(writer_test_MOC_SRCS test/writer.hh)
add_executable(writer_test test/writer.cpp ${writer_test_MOC_SRCS})
target_link_libraries(writer_test alpino_corpus)
target_link_libraries(writer_test ${QT_LIBRARIES})
#add_test(writer writer_test)
