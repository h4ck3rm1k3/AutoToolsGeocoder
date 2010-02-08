prefix=/usr/local
libdir=${prefix}/lib

CXX_TARGET = libgeocoder.so.7.10.0

# Uncomment for GNU gcc/g++ - Linux, FreeBSD, OpenBSD, Solaris, etc.. 
CXX=g++
CXXFLAGS=-DUNIX -fPIC -O2 -Wall
LDFLAGS=-L/usr/local/lib -fPIC -lxerces-c
LDFLAGS_SHLIB=$(LDFLAGS) -shared

# Uncomment for Solaris, Sun Force C++
#CXX=CC
#CXXFLAGS=-fast -library=stlport4 -DUNIX -w -noex
#LDFLAGS=-L/usr/local/lib -L. -library=stlport4 -lxerces-c
#LDFLAGS_SHLIB=$(LDFLAGS) -G

D_GLOBAL=./global
D_GEOCODER=./geocoder
D_GEOCOMMON=./geocommon

OBJ_FILES = $(D_GLOBAL)/SetAssocCache.o $(D_GLOBAL)/Soundex.o $(D_GEOCODER)/Geocoder_Headers.o $(D_GEOCODER)/GeocoderD.o $(D_GEOCOMMON)/GeoBitPtr.o \
$(D_GLOBAL)/RawFile.o $(D_GLOBAL)/AddressParserLastLine.o $(D_GLOBAL)/BitSet.o $(D_GLOBAL)/RegularExprLexer.o $(D_GLOBAL)/AddressParserFirstLine.o \
$(D_GLOBAL)/RegularExprOr.o $(D_GLOBAL)/RegularExprSequence.o $(D_GEOCOMMON)/GeoBitStream.o $(D_GLOBAL)/File.o $(D_GLOBAL)/RawFileLinux.o \
$(D_GLOBAL)/Lexicon.o $(D_GEOCODER)/Geocoder.o $(D_GLOBAL)/RegExp.o $(D_GLOBAL)/FreeList.o $(D_GEOCODER)/GeoQuery.o $(D_GLOBAL)/RegularExprLiteralRange.o \
$(D_GEOCODER)/GeoAddressTemplate.o $(D_GLOBAL)/XmlToDataItem.o $(D_GLOBAL)/DomHelper.o $(D_GLOBAL)/RegularExprAction.o $(D_GLOBAL)/RegularExprWildcard.o \
$(D_GLOBAL)/RegularExprCounted.o $(D_GLOBAL)/RegularExprOptional.o $(D_GLOBAL)/RegularExprOneOrMore.o $(D_GLOBAL)/RegularExprZeroOrMore.o \
$(D_GLOBAL)/RegularExprLiteral.o $(D_GLOBAL)/RegularExprSet.o $(D_GLOBAL)/BulkAllocator.o $(D_GLOBAL)/Utility.o $(D_GLOBAL)/AddressTokenizer.o \
$(D_GLOBAL)/LookupTable.o $(D_GLOBAL)/StringSet.o $(D_GEOCOMMON)/GeoUtil.o $(D_GLOBAL)/DataItem.o $(D_GLOBAL)/StringToIntMap.o $(D_GLOBAL)/ListenerFIFO.o \
$(D_GLOBAL)/StringTorefMap.o $(D_GLOBAL)/RegularExprSimple.o $(D_GLOBAL)/RegularExprNFA.o $(D_GLOBAL)/Filesys.o $(D_GLOBAL)/RegularExprWrapper.o \
$(D_GLOBAL)/RegularExprSymbolizer.o $(D_GLOBAL)/RegularExprEngine.o $(D_GLOBAL)/RegularExprParser.o $(D_GLOBAL)/RegularExprTokenizer.o \
$(D_GLOBAL)/RegularExprPatternMatcher.o $(D_GLOBAL)/AddressParserLastLineImp.o $(D_GLOBAL)/AddressParserFirstLineImp.o $(D_GEOCODER)/GeocoderImp.o \
$(D_GEOCODER)/GeoQueryImp.o 

SRC_FILES = $(D_GLOBAL)/SetAssocCache.cpp $(D_GLOBAL)/Soundex.cpp $(D_GEOCODER)/Geocoder_Headers.cpp $(D_GEOCODER)/GeocoderD.cpp \
$(D_GEOCOMMON)/GeoBitPtr.cpp $(D_GLOBAL)/RawFile.cpp $(D_GLOBAL)/AddressParserLastLine.cpp $(D_GLOBAL)/BitSet.cpp $(D_GLOBAL)/RegularExprLexer.cpp \
$(D_GLOBAL)/AddressParserFirstLine.cpp $(D_GLOBAL)/RegularExprOr.cpp $(D_GLOBAL)/RegularExprSequence.cpp $(D_GLOBAL)/RawFileLinux.cpp \
$(D_GEOCOMMON)/GeoBitStream.cpp $(D_GLOBAL)/File.cpp $(D_GLOBAL)/Lexicon.cpp $(D_GEOCODER)/Geocoder.cpp $(D_GLOBAL)/RegExp.cpp \
$(D_GLOBAL)/FreeList.cpp $(D_GEOCODER)/GeoQuery.cpp $(D_GLOBAL)/RegularExprLiteralRange.cpp $(D_GEOCODER)/GeoAddressTemplate.cpp \
$(D_GLOBAL)/XmlToDataItem.cpp $(D_GLOBAL)/DomHelper.cpp $(D_GLOBAL)/RegularExprAction.cpp $(D_GLOBAL)/RegularExprWildcard.cpp \
$(D_GLOBAL)/RegularExprCounted.cpp $(D_GLOBAL)/RegularExprOptional.cpp $(D_GLOBAL)/RegularExprOneOrMore.cpp $(D_GLOBAL)/RegularExprZeroOrMore.cpp \
$(D_GLOBAL)/RegularExprLiteral.cpp $(D_GLOBAL)/RegularExprSet.cpp $(D_GLOBAL)/BulkAllocator.cpp $(D_GLOBAL)/Utility.cpp \
$(D_GLOBAL)/AddressTokenizer.cpp $(D_GLOBAL)/LookupTable.cpp $(D_GLOBAL)/StringSet.cpp $(D_GEOCOMMON)/GeoUtil.cpp $(D_GLOBAL)/DataItem.cpp \
$(D_GLOBAL)/StringToIntMap.cpp $(D_GLOBAL)/ListenerFIFO.cpp $(D_GLOBAL)/StringTorefMap.cpp $(D_GLOBAL)/RegularExprSimple.cpp \
$(D_GLOBAL)/RegularExprNFA.cpp $(D_GLOBAL)/Filesys.cpp $(D_GLOBAL)/RegularExprWrapper.cpp $(D_GLOBAL)/RegularExprSymbolizer.cpp \
$(D_GLOBAL)/RegularExprEngine.cpp $(D_GLOBAL)/RegularExprParser.cpp $(D_GLOBAL)/RegularExprTokenizer.cpp \
$(D_GLOBAL)/RegularExprPatternMatcher.cpp $(D_GLOBAL)/AddressParserLastLineImp.cpp $(D_GLOBAL)/AddressParserFirstLineImp.cpp \
$(D_GEOCODER)/GeocoderImp.cpp $(D_GEOCODER)/GeoQueryImp.cpp

all: do-it-all

ifeq (.depend,$(wildcard .depend))
include .depend
do-it-all: $(CXX_TARGET)
else
do-it-all: dep $(CXX_TARGET)
endif

$(CXX_TARGET): $(OBJ_FILES)
	$(CXX) $(LDFLAGS_SHLIB) -o $@ $^
dep:
	$(CXX) -DUNIX -MM $(SRC_FILES) >.depend

############################################################################################################################# INSTALL
install:
	cp $(CXX_TARGET) ${libdir}
	chmod 755 ${libdir}/$(CXX_TARGET)
	@echo "Note: you will likely need to run ldconfig/crle before building 'client'"

############################################################################################################################# SAMPLE PROGRAM
D_GEOCODERCLI=./GeoCoderCLI
client: $(D_GEOCODERCLI)/GeoCoderCLI.o
	$(CXX) -o client $(D_GEOCODERCLI)/GeoCoderCLI.o -L${libdir} -L. -lgeocoder $(LDFLAGS)

############################################################################################################################# CLEAN
clean:
	rm -rf *~ *.a *.o *.so \
$(D_GEOCODER)/*~ $(D_GEOCOMMON)/*~ $(D_GEOCODERCLI)/*~ $(D_GLOBAL)/*~ $(D_GEOCODERCONSOLE)/*~ $(D_GEOCODERSERVER)/*~ $(D_GEOCODERCLIENT)/*~ \
$(D_GEOCODER)/*.o $(D_GEOCOMMON)/*.o $(D_GEOCODERCLI)/*.o $(D_GLOBAL)/*.o $(D_GEOCODERCONSOLE)/*.o $(D_GEOCODERSERVER)/*.o $(D_GEOCODERCLIENT)/*.o \
$(CXX_TARGET) PortfolioExplorerLoaders cli console client server
