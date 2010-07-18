ifeq "$(shell ls ../../Makefile.include)" "../../Makefile.include"
include ../../Makefile.include
endif

ifeq ($(PLATFORM),darwin)
	DYLIBLINKFLAGS=-fPIC -shared -lz
else
	DYLIBLINKFLAGS=-fPIC -shared -lz
endif

CXXFLAGS=-O3 -Wall -I. -fPIC -I$(LIBALPINO_PATH)

SOURCES=\
	src/ActCorpusReader/ActCorpusReader.cpp \
	src/DzIstream/DzIstream.cpp \
	src/DzOstream/DzOstream.cpp \
	src/DzOstreamBuf/DzOstreamBuf.cpp \
	src/DzIstreamBuf/DzIstreamBuf.cpp \
	src/IndexNamePair/IndexNamePair.cpp \
	src/IndexedCorpusReader/IndexedCorpusReader.cpp \
	src/IndexedCorpusWriter/IndexedCorpusWriter.cpp \
	src/util/base64.cpp \
	src/util/textfile/textfile.cpp
OBJECTS=$(SOURCES:.cpp=.o)


ifeq "$(PROLOG)" "sicstus"
PROLOGLIBS=prolog/corpusreader$(MODULEEXT) prolog/corpusreader.s.o
else
PROLOGLIBS=prolog/corpusreader$(MODULEEXT) 
endif


PYTHONLIBS=python/indexedcorpus$(PYMODULEEXT)

all: libcorpus$(DYLIBEXT) libcorpus.a $(PROLOGLIBS) $(PYTHONLIBS)
	cp python/indexedcorpus$(PYMODULEEXT) ../python-lib

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

libcorpus.so: $(OBJECTS)
	g++ $(DYLIBLINKFLAGS) -Wl,-rpath,$(LIBALPINO_PATH) \
		-L$(LIBALPINO_PATH) -lalpino -o $@ $(OBJECTS)

libcorpus.dylib: $(OBJECTS)
	g++ $(DYLIBLINKFLAGS) -Wl,-rpath,$(LIBALPINO_PATH) \
		-L$(LIBALPINO_PATH) -lalpino -o $@ $(OBJECTS)
	install_name_tool -change libalpino.dylib @rpath/libalpino.dylib \
		$@

libcorpus.a: $(OBJECTS)
	ar cr $@ $(OBJECTS)

prolog/corpusreader.so: prolog/corpusreader.pl prolog/corpusreader.cpp libcorpus$(DYLIBEXT)
	$(SPLFR) prolog/corpusreader.pl prolog/corpusreader.cpp \
		--cflag="$(CXXFLAGS)" -O2 -LD -Wall \
		-Wl,-rpath,$(PWD) -Wl,-rpath,$(LIBALPINO_PATH) \
		-L. -L$(LIBALPINO_PATH) -lcorpus -lalpino -lstdc++ -lz
ifeq "$(PROLOG)" "sicstus"
	mv corpusreader.so $@
endif

prolog/corpusreader.bundle prolog/corpusreader.dylib: prolog/corpusreader.pl prolog/corpusreader.cpp libcorpus$(DYLIBEXT)
	$(SPLFR) prolog/corpusreader.pl prolog/corpusreader.cpp \
		--cflag="$(CXXFLAGS)" -O2 -LD -Wall \
		-Wl,-rpath,$(PWD) -Wl,-rpath,$(LIBALPINO_PATH) \
		-L. -L$(LIBALPINO_PATH) -lcorpus -lalpino -lstdc++ -lz
ifeq "$(PROLOG)" "sicstus"
	mv corpusreader.bundle $@
endif
	install_name_tool -change libalpino.dylib @rpath/libalpino.dylib \
		$@
	install_name_tool -change libcorpus.dylib @rpath/libcorpus.dylib \
		$@

prolog/corpusreader.s.o: prolog/corpusreader.pl prolog/corpusreader.cpp libcorpus$(DYLIBEXT)
	splfr prolog/corpusreader.pl prolog/corpusreader.cpp \
		 --static --cflag="$(CXXFLAGS)" -O2 -LD -Wall \
		-Wl,-rpath,$(PWD) -Wl,-rpath,$(LIBALPINO_PATH) \
		-L. -L$(LIBALPINO_PATH) -lcorpus -lalpino -lstdc++ -lz
	mv corpusreader.s.o $@

python/indexedcorpus$(PYMODULEEXT): python/indexedcorpus.cpp libcorpus.a
	( cd python ; python setup.py install --install-platlib=. )

test/test: libcorpus.a test/test.cpp
	g++ -Wall -pedantic -I. -lz -o $@ test/test.cpp libcorpus.a

test: test/test
	( cd test ; ./test )

clean:
	find . -name '*.o' -exec rm -f {} \;
	rm -rf python/build

realclean: clean
	rm -f libcorpus$(DYLIBEXT)
	rm -f prolog/corpusreader$(MODULEEXT)
	rm -f python/indexedcorpus$(DYLIBEXT)
	rm -f libcorpus.a
	rm -f test/test

install:
