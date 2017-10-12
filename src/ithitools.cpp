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

// ithitools.cpp : Defines the entry point for the console application.
//

#ifdef _WINDOWS
#include "stdafx.h"
#else
#include "stdio.h"
#endif

#include <stdlib.h>
#include "pcap_reader.h"
#include "DnsStats.h"
#include "getopt.h"

void usage()
{
    fprintf(stderr, "ITHITOOLS -- a tool for ITHI data extraction and metric computation.\n");
    fprintf(stderr, "Usage: ithitools <options> <input-files>\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -c                 process capture files in PCAP format.\n");
    fprintf(stderr, "  -s                 process summary files, from previous captures.\n");
    fprintf(stderr, "  -o file.csv        output file.\n");
    fprintf(stderr, "  -r root-addr.txt   text file containing the list of root server addresses.\n");
    fprintf(stderr, "  -a res-addr.txt	  allowed list of resolver addresses. Traffic to or from\n");
    fprintf(stderr, "                     addresses in this list will not be filtered out by the\n");
    fprintf(stderr, "                     excessive traffic filtering mechanism.\n");
    fprintf(stderr, "  -x res-addr.txt	  excluded list of resolver addresses. Traffic to or from\n");
    fprintf(stderr, "                     these addresses will be ignored when extracting traffic.\n");
    fprintf(stderr, "  -f	              Filter out address sources that generate too much traffic.\n");
    fprintf(stderr, "  -v table-file.csv  Use the definition from the csv file for the specified\n");
    fprintf(stderr, "                     parameter table.The CSV file should be downloaded from\n");
    fprintf(stderr, "                     the IANA site, using the “CSV” link provided by IANA.\n");
    fprintf(stderr, "                     The file name must be the IANA specified name.\n");
    fprintf(stderr, "  -n number	      Number of strings in the list of leaking domains(M4).\n");
    fprintf(stderr, "  -m number	      Total number of TLD strings that will be retained in\n");
    fprintf(stderr, "                     summaries at extraction points.\n");
    fprintf(stderr, "  -t tld-file.txt    Text file containing a list of registered TLD, one per line.\n");
    fprintf(stderr, "  -u tld-file.txt	  Text file containing special usage TLD (RFC6761).\n");
}


int main(int argc, char ** argv)
{
    pcap_reader reader;
    int exec_mode = 0; /* capture extraction= 0, summary aggregation = 1 */
    int nb_records_read = 0;
    int nb_extracted = 0;
    bool do_address_filtering = false;
    bool found_v4 = false;
    bool found_v6 = false;
    DnsStats stats;
    int nb_udp_dns = 0;
    int nb_udp_dns_frag = 0;
    int nb_udp_dns_extract = 0;
    char const * default_inputFile = "smalltest.pcap";
    char const * inputFile = default_inputFile;
    char const * default_csv_file = "smalltest.csv";
    char const * out_file = default_csv_file;
    char const * root_address_file = NULL;
    char const * allowed_addr_file = NULL;
    char const * excluded_addr_file = NULL;
    char const * table_version_addr_file = NULL;
    int nb_names_in_m4 = 64;
    int nb_names_in_tld = 2048;
    char * extract_file = NULL;
    int extract_by_error_type[512] = { 0 };

    /* Get the parameters */
    int opt;
    while ((opt = getopt(argc, argv, "o:r:a:x:v:n:m:t:u:hcsf")) != -1)
    {
        switch (opt)
        {
        case 'c':
            exec_mode = 0;
            break;
        case 'o':
            out_file = optarg;
            break;
        case 'r':
            root_address_file = optarg;
            fprintf(stderr, "The root addresses redefinition option is not yet implemented.\n");
            break;
        case 'a':
            allowed_addr_file = optarg;
            fprintf(stderr, "The allowed address option is not yet implemented.\n");
            break;
        case 'x':
            excluded_addr_file = optarg;
            fprintf(stderr, "The excluded address option is not yet implemented.\n");
            break;
        case 'v':
            table_version_addr_file = optarg;
            fprintf(stderr, "The table redefinition option is not yet implemented.\n");
            break;
        case 'n':
            if ((nb_names_in_m4 = atoi(optarg)) <= 0)
            {
                fprintf(stderr, "Invalid number of names: %s\n", optarg);
                usage();
            }
            break;
        case 'm':
            if ((nb_names_in_m4 = atoi(optarg)) <= 0)
            {
                fprintf(stderr, "Invalid number of names: %s\n", optarg);
                usage();
            }
            break;
        case 'f':
            do_address_filtering = true;
            fprintf(stderr, "The address filtering option is not yet implemented.\n");
            break;
        case 'h':
            usage();
            break;
        case 's':
            exec_mode = 1;
            fprintf(stderr, "The summary_aggregation option is not yet implemented.\n");
            break;
        }
    }

    /* Simplified style params */
    if (optind < argc)
    {
        inputFile = argv[optind++];
    }

    if (reader.Open(inputFile, extract_file))
    {
        printf("Open succeeds, magic = %x, v =  %d/%d, lmax = %d, net = %x\n",
            reader.header.magic_number,
            reader.header.version_major,
            reader.header.version_minor,
            reader.header.snaplen,
            reader.header.network
        );

        while (reader.ReadNext())
        {
            nb_records_read++;
            if (nb_records_read <= 10 ||
                (reader.ip_version == 4 && !found_v4) ||
                (reader.ip_version == 6 && !found_v6))
            {
                printf("Record %d, l = %d, ip: %d, tp: %d, tp_l: %d, %d:%d\n",
                    nb_records_read, reader.frame_header.incl_len,
                    reader.ip_version, reader.tp_version, reader.tp_length,
                    reader.tp_port1, reader.tp_port2);
                found_v4 |= (reader.ip_version == 4);
                found_v6 |= (reader.ip_version == 6);
            }

            if (reader.tp_version == 17 &&
                (reader.tp_port1 == 53 || reader.tp_port2 == 53))
            {

                if (reader.is_fragment)
                {
                    nb_udp_dns_frag++;
                }
                else
                {
                    stats.SubmitPacket(reader.buffer + reader.tp_offset + 8,
                        reader.tp_length - 8, reader.ip_version, reader.buffer + reader.ip_offset);
                    nb_udp_dns++;

                    /* Apply default logic here for selecting what to extract */
                    if (stats.error_flags > 0 && stats.error_flags < 512)
                    {
                        if (extract_by_error_type[stats.error_flags] < 5 ||
                            (stats.error_flags == 64 &&
                                extract_by_error_type[stats.error_flags] < 512))
                        {
                            reader.WriteExtract();
                            extract_by_error_type[stats.error_flags] += 1;
                            nb_extracted++;
                        }
                    }
                }
            }
        }

        printf("Read %d records, %d dns records.\nSkipped %d fragments.\nExtracted %d records.\n",
            nb_records_read, nb_udp_dns, nb_udp_dns_frag, nb_extracted);

        if (stats.ExportToCsv(out_file))
        {
            printf("Exported results to %s\n", out_file);
        }
        else
        {
            printf("Could not write to %s\n", out_file);
        }
    }
    return 0;
}

