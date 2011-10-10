find_package(PythonLibs REQUIRED)
if(PYTHONLIBS_FOUND)
  include_directories(${PYTHON_INCLUDE_DIRS})
endif()

set(PYTHON_HEADERS
  src/alpinocorpus.h
)

set(PYTHON_SOURCES
  src/alpinocorpus.cpp
)

add_library(alpinocorpus-py SHARED
  ${PYTHON_SOURCES}
)

set_target_properties(alpinocorpus-py PROPERTIES LINKER_LANGUAGE C)
set_target_properties(alpinocorpus-py PROPERTIES PREFIX "")
set_target_properties(alpinocorpus-py PROPERTIES OUTPUT_NAME "alpinocorpus")

if(APPLE)
  set_target_properties(alpinocorpus-py PROPERTIES SUFFIX ".so")
endif()

target_link_libraries(alpinocorpus-py ${PYTHON_LIBRARIES})
target_link_libraries(alpinocorpus-py alpino_corpus)
