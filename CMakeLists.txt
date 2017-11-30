CMAKE_MINIMUM_REQUIRED(VERSION 3.1)
CMAKE_POLICY(SET CMP0003 NEW)

PROJECT(ithitools C CXX)

SET(CMAKE_CXX_STANDARD 11)

INCLUDE_DIRECTORIES(src lib test include)

SET(ITHITOOLS_LIBRARY_FILES
   lib/AddressFilter.cpp
   lib/CaptureSummary.cpp
   lib/DnsStats.cpp
   lib/ithimetrics.cpp
   lib/pcap_reader.cpp
   lib/UsefulTransaction.cpp
   lib/NamePattern.cpp
   lib/getopt.c
   lib/DnscapPlugIn.cpp
   lib/M7Getter.cpp
)

SET(ITHITOOLS_TEST_LIBRARY_FILES
   test/CaptureTest.cpp
   test/hashtest.cpp
   test/LoadTest.cpp
   test/MergeTest.cpp
   test/MetricTest.cpp
   test/SaveTest.cpp
   test/testRfc6761.cpp
   test/PatternTest.cpp
   test/PluginTest.cpp
)

ADD_EXECUTABLE(ithitools 
   src/ithitools.cpp
   lib/AddressFilter.cpp
   lib/CaptureSummary.cpp
   lib/DnsStats.cpp
   lib/ithimetrics.cpp
   lib/pcap_reader.cpp
   lib/UsefulTransaction.cpp
   lib/NamePattern.cpp
   lib/M7Getter.cpp
   lib/getopt.c
)

ADD_EXECUTABLE(ithitest
   src/ithitest.cpp 
   lib/AddressFilter.cpp
   lib/CaptureSummary.cpp
   lib/DnsStats.cpp
   lib/ithimetrics.cpp
   lib/pcap_reader.cpp
   lib/UsefulTransaction.cpp
   lib/NamePattern.cpp
   lib/DnscapPlugIn.cpp
   lib/M7Getter.cpp
   test/CaptureTest.cpp
   test/hashtest.cpp
   test/LoadTest.cpp
   test/MergeTest.cpp
   test/MetricTest.cpp
   test/SaveTest.cpp
   test/testRfc6761.cpp
   test/PatternTest.cpp
   test/PluginTest.cpp
)

ADD_LIBRARY(ithicap SHARED
   lib/AddressFilter.cpp
   lib/CaptureSummary.cpp
   lib/DnsStats.cpp
   lib/pcap_reader.cpp
   lib/UsefulTransaction.cpp
   lib/NamePattern.cpp
   lib/DnscapPlugIn.cpp
)

SET_TARGET_PROPERTIES(ithicap PROPERTIES
                              VERSION 0.1
                              PUBLIC_HEADER include/ithicap.h)

SET_TARGET_PROPERTIES(ithitools PROPERTIES VERSION 0.1)

SET(TEST_EXES ithitest)

INSTALL(TARGETS ithitools ithicap
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include)

