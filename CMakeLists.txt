CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
CMAKE_POLICY(SET CMP0003 NEW)

PROJECT(ithitools C CXX)

SET(CMAKE_CXX_STANDARD 11)

SET(CMAKE_C_FLAGS "-std=c99 -Wall -O2 -g ${CC_WARNING_FLAGS} ${CMAKE_C_FLAGS}")
SET(CMAKE_C_FLAGS "-Wall -O2 -g ${CMAKE_CXX_FLAGS}")

INCLUDE_DIRECTORIES(src lib test include)

SET(ITHITOOLS_LIBRARY_FILES
   lib/AddressFilter.cpp
   lib/CaptureSummary.cpp
   lib/DnsStats.cpp
   lib/ithimetrics.cpp
   lib/ithipublisher.cpp
   lib/pcap_reader.cpp
   lib/UsefulTransaction.cpp
   lib/NamePattern.cpp
   lib/getopt.c
   lib/DnscapPlugIn.cpp
   lib/M7Getter.cpp
   lib/CsvHelper.cpp
   lib/M1Data.cpp
   lib/M2Data.cpp
   lib/ComputeMetric.cpp
   lib/ComputeM34.cpp
   lib/ComputeM6.cpp
)

SET(ITHITOOLS_TEST_LIBRARY_FILES
   test/CaptureTest.cpp 
   test/DnsPrefixTest.cpp
   test/hashtest.cpp
   test/GetNameTest.cpp
   test/LoadTest.cpp
   test/MergeTest.cpp
   test/MetricTest.cpp
   test/M1DataTest.cpp
   test/M2DataTest.cpp
   test/SaveTest.cpp
   test/testRfc6761.cpp
   test/PatternTest.cpp
   test/PluginTest.cpp
   test/PublishTest.cpp
   test/CsvTest.cpp
   test/TldCountTest.cpp
   test/ithi_test_class.cpp
)

ADD_EXECUTABLE(ithitools 
   src/ithitools.cpp
   lib/AddressFilter.cpp
   lib/CaptureSummary.cpp
   lib/DnsStats.cpp
   lib/ithimetrics.cpp
   lib/ithipublisher.cpp
   lib/pcap_reader.cpp
   lib/UsefulTransaction.cpp
   lib/NamePattern.cpp
   lib/M7Getter.cpp
   lib/getopt.c
   lib/CsvHelper.cpp
   lib/M1Data.cpp
   lib/M2Data.cpp
   lib/ComputeMetric.cpp
   lib/ComputeM34.cpp
   lib/ComputeM6.cpp
)

ADD_EXECUTABLE(ithitest
   src/ithitest.cpp 
   lib/AddressFilter.cpp
   lib/CaptureSummary.cpp
   lib/DnsStats.cpp
   lib/ithimetrics.cpp
   lib/ithipublisher.cpp
   lib/pcap_reader.cpp
   lib/UsefulTransaction.cpp
   lib/NamePattern.cpp
   lib/DnscapPlugIn.cpp
   lib/M7Getter.cpp
   lib/CsvHelper.cpp
   lib/M1Data.cpp
   lib/M2Data.cpp
   lib/ComputeMetric.cpp
   lib/ComputeM34.cpp
   lib/ComputeM6.cpp
   lib/getopt.c
   test/CaptureTest.cpp
   test/DnsPrefixTest.cpp
   test/hashtest.cpp
   test/GetNameTest.cpp
   test/LoadTest.cpp
   test/MergeTest.cpp
   test/MetricTest.cpp
   test/M1DataTest.cpp
   test/M2DataTest.cpp
   test/SaveTest.cpp
   test/testRfc6761.cpp
   test/PatternTest.cpp
   test/PluginTest.cpp
   test/PublishTest.cpp
   test/CsvTest.cpp
   test/TldCountTest.cpp
   test/ithi_test_class.cpp
)

ADD_LIBRARY(ithicap SHARED
   lib/AddressFilter.cpp
   lib/CaptureSummary.cpp
   lib/DnsStats.cpp
   lib/pcap_reader.cpp
   lib/UsefulTransaction.cpp
   lib/NamePattern.cpp
   lib/CsvHelper.cpp
   lib/DnscapPlugIn.cpp
   lib/getopt.c
)

SET_TARGET_PROPERTIES(ithicap PROPERTIES
                              VERSION 1.02
                              PUBLIC_HEADER include/ithicap.h)

SET_TARGET_PROPERTIES(ithitools PROPERTIES VERSION 1.02)

SET(TEST_EXES ithitest)

INSTALL(TARGETS ithitools ithicap
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include)

