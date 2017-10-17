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
#include "PcapCsvMerge.h"
#include "CaptureSummary.h"

int usage()
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
    fprintf(stderr, "                     the IANA site, using the �CSV� link provided by IANA.\n");
    fprintf(stderr, "                     The file name must be the IANA specified name.\n");
    fprintf(stderr, "  -n number	      Number of strings in the list of leaking domains(M4).\n");
    fprintf(stderr, "  -m number	      Total number of TLD strings that will be retained in\n");
    fprintf(stderr, "                     summaries at extraction points.\n");
    fprintf(stderr, "  -t tld-file.txt    Text file containing a list of registered TLD, one per line.\n");
    fprintf(stderr, "  -u tld-file.txt	  Text file containing special usage TLD (RFC6761).\n");

    return -1;
}


int main(int argc, char ** argv)
{
    int exec_mode = -1; /* capture extraction= 0, summary aggregation = 1 */
    bool do_address_filtering = false;
    int exit_code = 0;

    DnsStats stats;
    char const * default_inputFile = "smalltest.pcap";
    char const * inputFile = default_inputFile;
    char const * default_csv_file = "smalltest.csv";
    char const * out_file = default_csv_file;
    char const * root_address_file = NULL;
    char const * allowed_addr_file = NULL;
    char const * excluded_addr_file = NULL;
    char const * table_version_addr_file = NULL;
    int nb_names_in_tld = 2048;
    char * extract_file = NULL;
    int extract_by_error_type[512] = { 0 };

    /* Get the parameters */
    int opt;
    while (exit_code == 0 && (opt = getopt(argc, argv, "o:r:a:x:v:n:m:t:u:hcsf")) != -1)
    {
        switch (opt)
        {
        case 'c':
            if (exec_mode != -1 && exec_mode != 0)
            {
                fprintf(stderr, "Can only specify one mode, -c (capture) or -s (summary)\n");
                exit_code = -1;
            }
            else
            {
                exec_mode = 0;
            }
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
        {
            int nb_names_in_m4;

            if ((nb_names_in_m4 = atoi(optarg)) <= 0)
            {
                fprintf(stderr, "Invalid number of names: %s\n", optarg);
                exit_code = usage();
            }
            else
            {
                stats.max_tld_leakage_count = (uint32_t) nb_names_in_m4;
            }
            break;
        }
        case 'm':
        {
            int nb_names_in_m4_table;
            if ((nb_names_in_m4_table = atoi(optarg)) <= 0)
            {
                fprintf(stderr, "Invalid number of names in table: %s\n", optarg);
                exit_code = usage();
            }
            else
            {
                stats.max_tld_leakage_table_count = (uint32_t)nb_names_in_m4_table;
            }
            break;
        }
        case 'f':
            do_address_filtering = true;
            fprintf(stderr, "The address filtering option is not yet implemented.\n");
            break;
        case 'h':
            exit_code = usage();
            break;
        case 's':
            if (exec_mode != -1 && exec_mode != 1)
            {
                fprintf(stderr, "Can only specify one mode, -c (capture) or -s (summary)\n");
                exit_code = -1;
            }
            else
            {
                exec_mode = 1;
            }
            break;
        }
    }

    if (exit_code != 0)
    {
        return exit_code;
    }

    if (exec_mode == -1)
    {
        /* Exec mode was not specified. Setting it to "capture" */
        exec_mode = 0;
    }

    if (exec_mode == 0)
    {
        /* Simplified style params */
        bool atLeastOne = false;
        int nb_records_read = 0;
        int nb_extracted = 0;
        int nb_udp_dns = 0;
        int nb_udp_dns_frag = 0;
        int nb_udp_dns_extract = 0;

        do {
            pcap_reader reader;

            if (optind < argc)
            {
                inputFile = argv[optind++];
            }

            if (!reader.Open(inputFile, extract_file))
            {
                fprintf(stderr, "Could not open PCAP file <%s>.\n", inputFile);
                exit_code = -1;
            }
            else
            {
                printf("Open <%s> succeeds.\n    magic = %x, v =  %d/%d, lmax = %d, net = %x\n",
                    inputFile,
                    reader.header.magic_number,
                    reader.header.version_major,
                    reader.header.version_minor,
                    reader.header.snaplen,
                    reader.header.network
                );

                while (reader.ReadNext())
                {
                    nb_records_read++;

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
            }
        } while (optind < argc && exit_code == 0);

        if (exit_code == 0)
        {
            if (stats.ExportToCsv(out_file))
            {
                printf("Exported results to <%s>\n", out_file);
            }
            else
            {
                printf("Could not write to %s\n", out_file);
                exit_code = -1;
            }
        }
    }
    else if (exec_mode == 1)
    {
        if (optind >= argc)
        {
            fprintf(stderr, "No file to merge!\n");
            exit_code = usage();
        }
        else
        {
#if 1
            CaptureSummary cs;

            if (!cs.Merge(argc - optind, (char const **)(argv + optind)))
            {
                fprintf(stderr, "Cannot merge the input files.\n");
                exit_code = -1;
            }
            else
            {
                if (!cs.Save(out_file))
                {
                    fprintf(stderr, "Cannot save the merged summary on <%s>.\n",
                        out_file);
                    exit_code = -1;
                }
                else
                {
                    printf("Merge succeeded.\n");
                }
            }
#else
            FILE * F_out = NULL;

#ifdef _WINDOWS
            errno_t err = fopen_s(&F_out, out_file, "w");
            if (err != 0)
            {
                F_out = NULL;
            }
#else
            bool ret;
            F_out = fopen(filename, "w");
#endif
            if (F_out == NULL)
            {
                fprintf(stderr, "Cannot open output file <%s>\n", out_file);
                exit_code = -1;
            }
            else
            {
                PcapCsvMerge merger;

                if (!merger.DoMerge(argc - optind, argv + optind, F_out))
                {
                    fprintf(stderr, "Could not merge the input files!\n");
                    exit_code = -1;
                }
                else
                {
                    printf("Merge succeeded.\n");
                }
            }
#endif
        }
    }
    else
    {
        fprintf(stderr, "Unexpected exec mode: %d\n", exec_mode);
        exit_code = -1;
    }

    return exit_code;
}
