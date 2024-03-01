CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
CMAKE_POLICY(SET CMP0003 NEW)

PROJECT(ithitools C CXX)

OPTION(PRIVACY_CONSCIOUS "Compile extensions -Y and -E" OFF)
IF(PRIVACY_CONSCIOUS)
   ADD_DEFINITIONS(-DPRIVACY_CONSCIOUS)
ENDIF(PRIVACY_CONSCIOUS)

SET(CMAKE_CXX_STANDARD 11)

SET(CMAKE_C_FLAGS "-std=c99 -Wall -O2 -g ${CC_WARNING_FLAGS} ${CMAKE_C_FLAGS}")
SET(CMAKE_C_FLAGS "-Wall -O2 -g ${CMAKE_C_FLAGS}")
SET(CMAKE_CXX_FLAGS "-Wall -O2 -g ${CMAKE_CXX_FLAGS}")

INCLUDE_DIRECTORIES(src lib test include cdns/lib)

SET(ITHITOOLS_LIBRARY_HEADERS
   lib/AddressFilter.h
   lib/CaptureSummary.h
   lib/ComputeM34.h
   lib/ComputeM6.h
   lib/ComputeMetric.h
   lib/config.h
   lib/CsvHelper.h
   lib/dnscap_common.h
   lib/DnsStats.h
   lib/DnsTypes.h
   lib/HashBinGeneric.h
   lib/ipstats.h
   lib/ithimetrics.h
   lib/ithipublisher.h
   lib/ithiutil.h
   lib/ithi_getopt.h
   lib/linktype.h
   lib/M1Data.h
   lib/M2Data.h
   lib/M7Getter.h
   lib/NamePattern.h
   lib/OdiPublisher.h
   lib/pcap_reader.h
   lib/QNameMinimization.h
   lib/TldAsKey.h
   lib/UsefulTransaction.h
   lib/Version.h
)

SET(ITHITOOLS_LIBRARY_FILES
   lib/AddressFilter.cpp
   lib/CaptureSummary.cpp
   cdns/lib/cbor.cpp
   cdns/lib/cdns.cpp
   lib/DnscapPlugIn.cpp
   lib/DnsStats.cpp
   lib/ipstats.cpp
   lib/ithimetrics.cpp
   lib/ithipublisher.cpp
   lib/ithiutil.cpp
   lib/pcap_reader.cpp
   lib/NamePattern.cpp
   lib/getopt.c
   lib/M7Getter.cpp
   lib/CsvHelper.cpp
   lib/M1Data.cpp
   lib/M2Data.cpp
   lib/ComputeMetric.cpp
   lib/ComputeM34.cpp
   lib/ComputeM6.cpp
   lib/OdiPublisher.cpp
)

add_library(ithitools-core ${ITHITOOLS_LIBRARY_HEADERS} ${ITHITOOLS_LIBRARY_FILES})
target_include_directories(ithitools-core
    PUBLIC
        lib
        cdns/lib)

SET(ITHITOOLS_TEST_LIBRARY_HEADERS
   test/CaptureTest.h
   test/CdnsCaptureTest.h
   test/CsvTest.h
   test/DnsPrefixTest.h
   test/GetNameTest.h
   test/hashtest.h
   test/HyperLogLogTest.h
   test/IPStatsTest.h
   test/ithi_test_class.h
   test/LoadTest.h
   test/M1DataTest.h
   test/M2DataTest.h
   test/MergeTest.h
   test/MetricTest.h
   test/OdiPublishTest.h
   test/PatternTest.h
   test/PluginTest.h
   test/PublishTest.h
   test/QNameTest.h
   test/SaveTest.h
   test/StatsByIpTest.h
   test/testRfc6761.h
   test/TldCountTest.h
   cdns/lib/cbor.cpp
   cdns/lib/cdns.cpp
)

SET(ITHITOOLS_TEST_LIBRARY_FILES
   test/CaptureTest.cpp
   test/capture_fuzz.cpp
   test/CdnsCaptureTest.cpp
   test/CsvTest.cpp
   test/DnsPrefixTest.cpp
   test/GetNameTest.cpp
   test/hashtest.cpp
   test/HyperLogLogTest.cpp
   test/IPStatsTest.cpp
   test/ithitest.cpp
   test/ithi_test_class.cpp
   test/LoadTest.cpp
   test/M1DataTest.cpp
   test/M2DataTest.cpp
   test/MergeTest.cpp
   test/MetricTest.cpp
   test/OdiPublishTest.cpp
   test/PatternTest.cpp
   test/PluginTest.cpp
   test/PublishTest.cpp
   test/QNameTest.cpp
   test/SaveTest.cpp
   test/StatsByIpTest.cpp
   test/testRfc6761.cpp
   test/TldCountTest.cpp
)

add_library(ithitools-test ${ITHITOOLS_LIBRARY_HEADERS}
   ${ITHITOOLS_TEST_LIBRARY_HEADERS}
   ${ITHITOOLS_TEST_LIBRARY_FILES})
target_include_directories(ithitools-test
    PUBLIC
        lib
        test
)

ADD_EXECUTABLE(ithitools 
   src/ithitools.cpp
   ithitools-core
)

ADD_EXECUTABLE(ithitest
   src/ithitest.cpp 
   ithitools-core
   ithitools-test
)

ADD_LIBRARY(ithicap SHARED
   lib/AddressFilter.cpp
   lib/CaptureSummary.cpp
   cdns/lib/cbor.cpp
   cdns/lib/cdns.cpp
   lib/DnsStats.cpp
   lib/ipstats.cpp
   lib/ithiutil.cpp
   lib/pcap_reader.cpp
   lib/UsefulTransaction.cpp
   lib/NamePattern.cpp
   lib/CsvHelper.cpp
   lib/DnscapPlugIn.cpp
   lib/getopt.c
)

SET_TARGET_PROPERTIES(ithicap PROPERTIES
                              VERSION 2.03
                              PUBLIC_HEADER include/ithicap.h)

SET_TARGET_PROPERTIES(ithitools PROPERTIES VERSION 2.10)

SET(TEST_EXES ithitest)

INSTALL(TARGETS ithitools ithicap
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include)

