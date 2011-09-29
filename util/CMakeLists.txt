include_directories(${AlpinoCorpus_SOURCE_DIR}/util/common)

set(COMMON_HEADERS
  common/ProgramOptions.hh
)

set(COMMON_SOURCES
  common/ProgramOptions.cpp
)

set(ACT_SOURCES
  act/main.cpp
)

add_executable(alpinocorpus-act
	${ACT_SOURCES}
	${COMMON_HEADERS}
	${COMMON_SOURCES}
)

target_link_libraries(alpinocorpus-act alpino_corpus)

install(TARGETS alpinocorpus-act
	RUNTIME DESTINATION bin
)
