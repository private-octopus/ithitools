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
#include "CaptureSummary.h"
#include "ithimetrics.h"

int usage()
{
    fprintf(stderr, "ITHITOOLS -- a tool for ITHI data extraction and metric computation.\n");
    fprintf(stderr, "Usage: ithitools <options> <input-files>\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Options used in capture mode:\n");
    fprintf(stderr, "  -c                 process capture files in PCAP format.\n");
    fprintf(stderr, "  -s                 process summary files, from previous captures.\n");
    fprintf(stderr, "  -o file.csv        output file containing the computed summary.\n");
    fprintf(stderr, "  -r root-addr.txt   text file containing the list of root server addresses.\n");
    fprintf(stderr, "  -a res-addr.txt      allowed list of resolver addresses. Traffic to or from\n");
    fprintf(stderr, "                     addresses in this list will not be filtered out by the\n");
    fprintf(stderr, "                     excessive traffic filtering mechanism.\n");
    fprintf(stderr, "  -x res-addr.txt      excluded list of resolver addresses. Traffic to or from\n");
    fprintf(stderr, "                     these addresses will be ignored when extracting traffic.\n");
    fprintf(stderr, "  -f              Filter out address sources that generate too much traffic.\n");
    fprintf(stderr, "  -t tld-file.txt    Text file containing a list of registered TLD, one per line.\n");
    fprintf(stderr, "  -u tld-file.txt      Text file containing special usage TLD (RFC6761).\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Options used in metric computation mode:\n");
    fprintf(stderr, "  -i ithi/folder     File path of the ITHI folder (ITHI).\n");
    fprintf(stderr, "                     If not specified, use the current directory.\n");
    fprintf(stderr, "  -d yyyy-mm-dd      Date for which the metrics shall be computed.");
    fprintf(stderr, "                     If not specified, use current day.\n");
    fprintf(stderr, "  -y accuracy.csv    CSV file containing accuracy data needed for M1.\n");
    fprintf(stderr, "                     If not specified, M1 data is read from (ITHI)/input/M1/\n");
    fprintf(stderr, "  -b abuse.csv       CSV file containing abuse data needed for M2.\n");
    fprintf(stderr, "                     If not specified, M2 data is read from (ITHI)/input/M2/\n");
    fprintf(stderr, "  -p root-cap.csv    CSV file containing summary of root traffic for M3.\n");
    fprintf(stderr, "                     If not specified, M3 data is read from (ITHI)/input/M3/\n");
    fprintf(stderr, "  -e r-res-cap.csv   CSV file containing summary of recursive resolver traffic for M4\n");
    fprintf(stderr, "                     and M6. If not specified, data is read from (ITHI)/input/M46/\n");
    fprintf(stderr, "  -l lies.csv        CSV file containing abuse data needed for M5.\n");
    fprintf(stderr, "                     If not specified, M5 data is read from (ITHI)/input/M5/\n");
    fprintf(stderr, "  -z root.zone       Root zone file used computing M7.\n");
    fprintf(stderr, "  -n number          Number of strings in the list of leaking domains(M4).\n");
    fprintf(stderr, "  -v table-file.csv  Use the definition from the csv file for the specified\n");
    fprintf(stderr, "                     parameter table when computing M6. The CSV file should be\n");
    fprintf(stderr, "                     downloaded from the IANA site, using the CSV link provided\n");
    fprintf(stderr, "                     by IANA. The file name must be the IANA specified name.\n");
    fprintf(stderr, "  -1 m1.csv          Output file where to write the computed metric M1.\n");
    fprintf(stderr, "  -2 m2.csv          Output file where to write the computed metric M2.\n");
    fprintf(stderr, "  -3 m3.csv          Output file where to write the computed metric M3.\n");
    fprintf(stderr, "  -4 m4.csv          Output file where to write the computed metric M4.\n");
    fprintf(stderr, "  -5 m5.csv          Output file where to write the computed metric M5.\n");
    fprintf(stderr, "  -6 m6.csv          Output file where to write the computed metric M6.\n");
    fprintf(stderr, "  -7 m7.csv          Output file where to write the computed metric M7.\n");
    fprintf(stderr, "                     If these options are not specified, metric are written\n");
    fprintf(stderr, "                     in the folder (ITHI)/output/Mx/, with x = [1234567],\n");
    fprintf(stderr, "                     with file name MX-yyyy-mm-dd.csv.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Option used in test mode:\n");
    fprintf(stderr, "  -m metric.csv      output file containing all the computed metric.\n");
    fprintf(stderr, "  -?                 Print this page.\n");

    return -1;
}


int main(int argc, char ** argv)
{
    int exec_mode = -1; /* capture extraction= 0, summary aggregation = 1 */
    int exit_code = 0;

    CaptureSummary cs;
    DnsStats stats;
    char const * default_inputFile = "smalltest.pcap";
    char const * inputFile = default_inputFile;
    char const * default_csv_file = "smalltest.csv";
    char const * out_file = default_csv_file;
    char const * root_address_file = NULL;
    char const * allowed_addr_file = NULL;
    char const * excluded_addr_file = NULL;
    char const * table_version_addr_file = NULL;
    char const * metric_file = NULL;
    char const * root_zone_file = NULL;
    int nb_names_in_tld = 2048;
    char * extract_file = NULL;
    int extract_by_error_type[512] = { 0 };

    /* Get the parameters */
    int opt;
    while (exit_code == 0 && (opt = getopt(argc, argv, "o:r:a:x:v:n:m:t:u:7:hcsf?")) != -1)
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
            stats.allowedAddresses.AddToList(optarg);
            break;
        case 'x':
            stats.bannedAddresses.AddToList(optarg);
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
            metric_file = optarg;
            break;
        }
        case 'f':
            stats.enable_frequent_address_filtering = true;
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
        case 't':
            fprintf(stderr, "Sorry, update list of registered TLD not implemented yet.\n");
            break;
        case 'u':
            fprintf(stderr, "Sorry, update list of special usage names (RFC6761) not implemented yet.\n");
            break;
        case '7':
            root_zone_file = optarg;
            break;
        case '?':
            exit_code = usage();
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
        if (optind >= argc)
        {
            fprintf(stderr, "No capture file to analyze!\n");
            exit_code = usage();
        }
        else
        {
            if (!stats.LoadPcapFiles(argc - optind, (char const **)(argv + optind)))
            {
                fprintf(stderr, "Cannot process the input files.\n");
                exit_code = -1;
            }
            else if (!stats.ExportToCaptureSummary(&cs))
            {
                fprintf(stderr, "Cannot process the capture summary.\n");
                exit_code = -1;

            }
            else if (!cs.Save(out_file))
            {
                fprintf(stderr, "Cannot save the merged summary on <%s>.\n",
                    out_file);
                exit_code = -1;
            }
            else
            {
                printf("Capture processing succeeded.\n");
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
        }
    }
    else
    {
        fprintf(stderr, "Unexpected exec mode: %d\n", exec_mode);
        exit_code = -1;
    }

    /* Compute and save the ITHI metrics */
    if (exit_code == 0 && metric_file != NULL)
    {
        ithimetrics met;

        if (root_zone_file != NULL && !met.GetM7(root_zone_file))
        {
            fprintf(stderr, "Cannot compute the ITHI metrics M7 from <root_zone_file>.\n");
            exit_code = -1;
        }
        else if (!met.GetMetrics(&cs))
        {
            fprintf(stderr, "Cannot compute the ITHI metrics.\n");
            exit_code = -1;
        }
        else if (!met.Save(metric_file))
        {
            fprintf(stderr, "Cannot save the ITHI metrics in <%s>.\n", metric_file);
            exit_code = -1;
        }
        else
        {
            printf("ITHI metrics computed and saved in <%s>\n", metric_file);
        }
    }

    return exit_code;
}

