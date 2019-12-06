/*
* Author: Christian Huitema
* Copyright (c) 2017, Private Octopus, Inc.
* All rights reserved.
*
* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Private Octopus, Inc. BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * This file implements the interface functions for the DNSCAP plugin. The
 * plugin is implemented as a Linux Dynamic Library, which in our case will
 * take the name "libithicap.so". The plugin is loaded in DNSCAP using
 * the option "-P /some/file/path/libithicap.so <plugin options..>".
 * 
 * After the plug in is loaded, DNSCAP will attempt to locate a set of
 * functions, all defined by a C interface: libithicap_start, 
 * libithicap_stop, libithicap_open, libithicap_close, libithicap_output,
 * libithicap_usage, libithicap_extension, and libithicap_getopt. Not
 * all of these functions are mandatory -- we will not implement 
 * libithicap_extension.
 *
 * This code provides an implementation of the required functions, which
 * will link to a static set of C++ objects for doing the capture.
 */
#include "config.h"
#include "Version.h"

#ifdef HAVE_GETOPT
#include <unistd.h>
#else
#include "ithi_getopt.h"
#endif
#include "dnscap_common.h"
#ifndef _WINDOWS
#include <sys/socket.h>
#endif

#include "DnsStats.h"

/*
 * Common static variables. They have to be initialized and deleted as captures 
 * start and stop.
 */

static char const * default_csv_file = "smalltest.csv";
static char const * libithicap_out_file = default_csv_file;
static char const * libithicap_allowed = NULL;
static char const * libithicap_banned = NULL;
static int libithicap_nb_names_in_m4 = -1;
static bool libithicap_compute_nx_domain_cache = false;
static bool libithicap_enable_filtering = false;
static bool libithicap_enable_tld_list = false;
#ifdef PRIVACY_CONSCIOUS
char const* libithicap_address_list = NULL;
char const* libithicap_name_list = NULL;
#endif

static DnsStats* libithicap_stats = NULL;
static logerr_t* logerr = NULL;

extern "C"
{

    void libithicap_version() {
        fprintf(stderr, "libithicap.so. Version %d.%02d.\n", ITHITOOLS_VERSION_MAJOR, ITHITOOLS_VERSION_MINOR);
    }

    void libithicap_usage()
    {
        fprintf(stderr, "ITHICAP -- a DNSCAP plugin for ITHI data extraction.\n");
        fprintf(stderr, "Usage: ithitools <options>\n");
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "  -? -h              Print this page.\n");
        fprintf(stderr, "  -v                 Print the current version number.\n");
        fprintf(stderr, "  -o file.csv        output file containing the computed summary.\n");
        fprintf(stderr, "  -r root-addr.txt   text file containing the list of root server addresses.\n");
        fprintf(stderr, "  -a res-addr.txt	  allowed list of resolver addresses. Traffic to or from\n");
        fprintf(stderr, "                     addresses in this list will not be filtered out by the\n");
        fprintf(stderr, "                     excessive traffic filtering mechanism.\n");
        fprintf(stderr, "  -x res-addr.txt	  excluded list of resolver addresses. Traffic to or from\n");
        fprintf(stderr, "                     these addresses will be ignored when extracting traffic.\n");
        fprintf(stderr, "  -e                 compute ratio of non-cached NX-Domain queries.\n");
        fprintf(stderr, "  -f	              Filter out address sources that generate too much traffic.\n");
        fprintf(stderr, "  -n number	      Number of strings in the list of leaking domains(M332).\n");
        fprintf(stderr, "  -T                 Capture a list of TLD found in user queries.\n");
        fprintf(stderr, "  -t tld-file.txt    Text file containing a list of registered TLD, one per line.\n");
        fprintf(stderr, "  -u tld-file.txt	  Text file containing special usage TLD (RFC6761).\n");
#ifdef PRIVACY_CONSCIOUS
        fprintf(stderr, "  -A addr_list.txt   List all IP addresses and their usage in specified file.\n");
        fprintf(stderr, "  -E name_list.txt   List all erroneous DNS names and their usage in specified file.\n");
        fprintf(stderr, "                     Options A and E are rather slow, and have privacy issues.\n");
        fprintf(stderr, "                     No such traces enabled by default.\n");
#endif
    }

    /*
    * The "getopt" function will be called from the parent to
    * process plugin options.
    */
    void libithicap_getopt(int* argc, char** argv[])
    {
        int opt;
        int exit_code = 0;

        while (exit_code == 0 && (opt = getopt(*argc, *argv, "o:r:a:x:n:t:u:A:E:hefT")) != -1)
        {
            switch (opt)
            {
            case 'o':
                libithicap_out_file = optarg;
                break;
            case 'r':
                // root_address_file = optarg;
                fprintf(stderr, "The root addresses redefinition option is not yet implemented.\n");
                break;
            case 'a':
                libithicap_allowed = optarg;
                break;
            case 'x':
                libithicap_banned = optarg;
                break;
            case 'n':
            {
                int nb_names_in_m4;

                if ((nb_names_in_m4 = atoi(optarg)) <= 0)
                {
                    fprintf(stderr, "Invalid number of names: %s\n", optarg);
                }
                else
                {
                    libithicap_nb_names_in_m4 = (uint32_t)nb_names_in_m4;
                }
                break;
            }
            case 'e':
                libithicap_compute_nx_domain_cache = true;
                break;
            case 'f':
                libithicap_enable_filtering = true;
                break;

#ifdef PRIVACY_CONSCIOUS
            case 'A':
                libithicap_address_list = optarg;
                break;
            case 'E':
                libithicap_name_list = optarg;
                break;
#endif
            case 'T':
                libithicap_enable_tld_list = true;
                break;
            case 't':
                fprintf(stderr, "Sorry, update list of registered TLD not implemented yet.\n");
                exit(1);
                break;
            case 'u':
                fprintf(stderr, "Sorry, update list of special usage names (RFC6761) not implemented yet.\n");
                exit(1);
                break;
            case 'v':
                libithicap_version();
                exit(1);
            case '?':
            case 'h':
            default:
                libithicap_usage();
                exit(1);
            }
        }
    }

    /*
     * The "start" function is called once, when the program
     * starts.  It is used to initialize the plugin.  If the
     * plugin wants to write debugging and or error messages,
     * it should save the a_logerr pointer passed from the
     * parent code.
     */
    int libithicap_start(logerr_t* a_logerr)
    {
        libithicap_stats = new DnsStats();
        logerr = a_logerr;
        if (libithicap_stats) {
            if (libithicap_allowed != NULL)
            {
                libithicap_stats->allowedAddresses.AddToList(libithicap_allowed);
            }

            if (libithicap_banned != NULL)
            {
                libithicap_stats->bannedAddresses.AddToList(libithicap_banned);
            }

            if (libithicap_nb_names_in_m4 >= 0)
            {
                libithicap_stats->max_tld_leakage_count = (uint32_t)libithicap_nb_names_in_m4;
            }

            libithicap_stats->capture_cache_ratio_nx_domain = libithicap_compute_nx_domain_cache;
            libithicap_stats->enable_frequent_address_filtering = libithicap_enable_filtering;

            if (libithicap_enable_tld_list)
            {
                libithicap_stats->dnsstat_flags |= dnsStateFlagCountTld;
                libithicap_stats->dnsstat_flags |= dnsStateFlagListTldUsed;
            }

#ifdef PRIVACY_CONSCIOUS
            if (libithicap_address_list != NULL)
            {
                libithicap_stats->address_report = libithicap_address_list;
            }

            if (libithicap_name_list != NULL)
            {
                libithicap_stats->name_report = libithicap_name_list;
            }
#endif
        }
        return (libithicap_stats == NULL)?-1:0;
    }


    /*
     * The "stop" function is called once, when the program
     * is exiting normally.  It might be used to clean up state,
     * free memory, etc.
     */
    void libithicap_stop()
    {
        if (libithicap_stats != NULL)
        {
            delete libithicap_stats;
            libithicap_stats = NULL;
        }
    }


    /*
     * The "open" function is called at the start of each
     * collection interval, which might be based on a period
     * of time or a number of packets.  In the original code,
     * this is where we opened an output pcap file.
     *
     * The plugin is designed to capture only the first set or
     * the first interval.
     */
    int libithicap_open(my_bpftimeval ts)
    {
        UNREFERENCED_PARAMETER(ts);

        /* Check that this is not a double open */
        if (libithicap_stats == NULL) {
            return -1;
        } else if (libithicap_stats->IsCaptureStopped()) {
            return -1;
        }
        return 0;
    }


    /*
     * The "close" function is called at the end of each
     * collection interval, which might be based on a period
     * of time or on a number of packets.  In the original code
     * this is where we closed an output pcap file.
     *
     * The first interval closes the capture. If the capture is
     * already closed, return an error.
     */
    int libithicap_close(my_bpftimeval ts)
    {
        int exit_code = 0;
        CaptureSummary cs;

        UNREFERENCED_PARAMETER(ts);

        if (libithicap_stats == NULL ||
            libithicap_stats->IsCaptureStopped()) {
            exit_code = -1;
        } else {
            libithicap_stats->StopCapture();

            if (!libithicap_stats->ExportToCaptureSummary(&cs))
            {

                if (logerr != NULL)
                {
                    logerr("libithicap cannot process the capture summary.\n");
                }
                exit_code = -1;

            }
            else if (!cs.Save(libithicap_out_file))
            {
                if (logerr != NULL)
                {
                    logerr("libithicap cannot save the capture summary on <%s>.\n",
                        libithicap_out_file);
                }
                exit_code = -1;
            }
        }
        return exit_code;
    }

    /*
     * Here you can "process" a packet.  The function is named
     * "output" because in the original code this is where
     * packets were outputted.
     *
     * if flags & PCAP_OUTPUT_ISDNS != 0 then payload is the start of a DNS message.
     */
    void libithicap_output(const char* descr, iaddr from, iaddr to, uint8_t proto, 
        unsigned flags,
        unsigned sport, unsigned dport, my_bpftimeval ts,
        const u_char* pkt_copy, const unsigned olen,
        const u_char* payload, const unsigned payloadlen)
    {
        /* Need to do a little bit of massaging between the version of 
         * addresses used by DNSCAP and that used by ITHITOOLS */
        uint8_t * source_addr;
        size_t source_addr_length;
        uint8_t * dest_addr;
        size_t dest_addr_length;

        UNREFERENCED_PARAMETER(descr);
        UNREFERENCED_PARAMETER(proto);
        UNREFERENCED_PARAMETER(sport);
        UNREFERENCED_PARAMETER(dport);
        UNREFERENCED_PARAMETER(ts);
        UNREFERENCED_PARAMETER(pkt_copy);
        UNREFERENCED_PARAMETER(olen);


        if (libithicap_stats == NULL ||
            libithicap_stats->IsCaptureStopped()) {
            return;
        }

        if ((flags & DNSCAP_OUTPUT_ISDNS) == 0)
        {
            /* A fragment, not a DNS Packet */
            return;
        }

        switch (from.af)
        {
        case AF_INET:
            source_addr = (uint8_t *) &from.u.a4;
            source_addr_length = 4;
            break;
        default:
            source_addr = (uint8_t *)&from.u.a6;
            source_addr_length = 16;
            break;
        }

        switch (to.af)
        {
        case AF_INET:
            dest_addr = (uint8_t *)&to.u.a4;
            dest_addr_length = 4;
            break;
        default:
            dest_addr = (uint8_t *)&to.u.a6;
            dest_addr_length = 16;
            break;
        }

        libithicap_stats->SubmitPacket((uint8_t *)payload, payloadlen,
            source_addr, source_addr_length, dest_addr, dest_addr_length, ts);
    }
}
