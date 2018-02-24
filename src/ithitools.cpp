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
#include "ithipublisher.h"

int usage()
{
    fprintf(stderr, "ITHITOOLS -- a tool for ITHI data extraction and metric computation.\n");
    fprintf(stderr, "Usage: ithitools <options> -[csmw] <input-files>\n");
    fprintf(stderr, "  -? -h              Print this page.\n");
    fprintf(stderr, "  -c                 process DNS traffic capture files in PCAP format,\n");
    fprintf(stderr, "                     PCAP files listed the input files arguments.\n");
    fprintf(stderr, "  -s                 process summary files, from previous captures.\n");
    fprintf(stderr, "                     CSV files listed the input files arguments,\n");
    fprintf(stderr, "                     or in a text file, see -S argument description.\n");
    fprintf(stderr, "  -m                 compute the ITHI metrics.\n");
    fprintf(stderr, "  -p                 publish the HTML pages for the metrics.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Options used in capture mode:\n");
    fprintf(stderr, "  -o file.csv        output file containing the computed summary.\n");
    fprintf(stderr, "  -R root-addr.txt   text file containing the list of root server addresses.\n");
    fprintf(stderr, "  -a res-addr.txt    allowed list of resolver addresses. Traffic to or from\n");
    fprintf(stderr, "                     addresses in this list will not be filtered out by the\n");
    fprintf(stderr, "                     excessive traffic filtering mechanism.\n");
    fprintf(stderr, "  -x res-addr.txt    excluded list of resolver addresses. Traffic to or from\n");
    fprintf(stderr, "                     these addresses will be ignored when extracting traffic.\n");
    fprintf(stderr, "  -f                 Filter out address sources that generate too much traffic.\n");
    fprintf(stderr, "  -t tld-file.txt    Text file containing a list of registered TLD, one per line.\n");
    fprintf(stderr, "  -u tld-file.txt    Text file containing special usage TLD (RFC6761).\n");
    fprintf(stderr, "  -n number          Number of strings in the list of leaking domains(M4).\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Options used in summary mode:\n");
    fprintf(stderr, "  -o file.csv        output file containing the computed summary.\n");
    fprintf(stderr, "  -S filelist.txt    process summary files listed in file list.\n");
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
    fprintf(stderr, "  -r summary.csv     CSV file containing summary of root server traffic\n");
    fprintf(stderr, "                     for M3. If not specified, data is read from\n");
    fprintf(stderr, "                     (ITHI)/input/M3/\n");
    fprintf(stderr, "  -k summary.csv     CSV file containing summary of recursive resolver traffic\n");
    fprintf(stderr, "                     for M4 and M6. If not specified, data is read from\n");
    fprintf(stderr, "                     (ITHI)/input/M46/\n");
    fprintf(stderr, "  -l lies.csv        CSV file containing abuse data needed for M5.\n");
    fprintf(stderr, "                     If not specified, M5 data is read from (ITHI)/input/M5/\n");
    fprintf(stderr, "  -z root.zone       Root zone file used computing M7.\n");
    fprintf(stderr, "                     If not specified, M7 data is read from (ITHI)/input/M7/\n");
    fprintf(stderr, "  -v table-file.csv  Use the definition from the csv file for the specified\n");
    fprintf(stderr, "                     parameter table when computing M6. The CSV file should be\n");
    fprintf(stderr, "                     downloaded from the IANA site, using the CSV link provided\n");
    fprintf(stderr, "                     by IANA. The file name must be the IANA specified name.\n");
    fprintf(stderr, "                     If these options are not specified, metric are written\n");
    fprintf(stderr, "                     in the folder (ITHI)/output/Mx/, with x = [1234567],\n");
    fprintf(stderr, "                     with file name MX-yyyy-mm-dd.csv.\n");
    fprintf(stderr, "Options used in to test the metric computation:\n");
    fprintf(stderr, "  -M metric.csv      output file containing all the computed metric.\n");
    fprintf(stderr, "  -1 m1.csv          Output file where to write the computed metric M1.\n");
    fprintf(stderr, "  -2 m2.csv          Output file where to write the computed metric M2.\n");
    fprintf(stderr, "  -3 m3.csv          Output file where to write the computed metric M3.\n");
    fprintf(stderr, "  -4 m4.csv          Output file where to write the computed metric M4.\n");
    fprintf(stderr, "  -5 m5.csv          Output file where to write the computed metric M5.\n");
    fprintf(stderr, "  -6 m6.csv          Output file where to write the computed metric M6.\n");
    fprintf(stderr, "  -7 m7.csv          Output file where to write the computed metric M7.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Options used in web publishing computation mode:\n");
    fprintf(stderr, "  -i ithi/folder     File path of the ITHI folder (ITHI).\n");
    fprintf(stderr, "                     If not specified, use the current directory.\n");
    fprintf(stderr, "  -w web/folder      File path where to write web pages.\n");
    fprintf(stderr, "                     If not specified, use (ITHI)/www.\n");

    return -1;
}

enum ithi_tool_mode {
    ithi_mode_unspecified=0,
    ithi_mode_capture,
    ithi_mode_summary,
    ithi_mode_metrics,
    ithi_mode_publish,
    ithi_mode_max
};

static int check_execution_mode(ithi_tool_mode proposed, ithi_tool_mode * current)
{
    int exit_code = 0;
    if (*current != ithi_mode_unspecified && proposed != 0)
    {
        fprintf(stderr, "Can only specify one execution mode!\n");
        exit_code = usage();
    }
    else
    {
        *current = proposed;
    }

    return exit_code;
}


int main(int argc, char ** argv)
{
    ithi_tool_mode exec_mode = ithi_mode_unspecified; 
    int exit_code = 0;
    ithimetrics met;
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
    char const * summary_file = NULL;
    char const * capture_summary_list = NULL;
    char const * ithi_folder = NULL;
    char const * accuracy_file = NULL;
    char const * abuse_file = NULL;
    char const * root_capture_file = NULL;
    char const * recursive_capture_file = NULL;
    char const * lies_file = NULL;
    char const * root_zone_file = NULL;
    char const * web_root = NULL;
    char const * metric_output_files[7] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    int nb_names_in_tld = 2048;
    char * extract_file = NULL;
    int extract_by_error_type[512] = { 0 };

    /* Get the parameters */
    int opt;
    while (exit_code == 0 && (opt = getopt(argc, argv, "o:r:a:x:v:n:M:t:u:i:d:y:b:z:l:1:2:3:4:5:6:7:S:hfcsmp?")) != -1)
    {
        switch (opt)
        {
        case 'c':
            exit_code = check_execution_mode(ithi_mode_capture, &exec_mode);
            break;
        case 's':
            exit_code = check_execution_mode(ithi_mode_summary, &exec_mode);
            break;
        case 'm':
            exit_code = check_execution_mode(ithi_mode_metrics, &exec_mode);
            break;
        case 'p':
            exit_code = check_execution_mode(ithi_mode_publish, &exec_mode);
            break;

        case 'o':
            out_file = optarg;
            break;
        case 'R':
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
        case 'M':
            metric_file = optarg;
            break;
        case 'f':
            stats.enable_frequent_address_filtering = true;
            break;
        case 'h':
            exit_code = usage();
            break;
        case 'S':
            /* Summarization from list of files implies summary mode */
            capture_summary_list = optarg; 
            exit_code = check_execution_mode(ithi_mode_summary, &exec_mode);
            break;
        case 't':
            fprintf(stderr, "Sorry, update list of registered TLD not implemented yet.\n");
            break;
        case 'u':
            fprintf(stderr, "Sorry, update list of special usage names (RFC6761) not implemented yet.\n");
            break;
        case 'i':
            ithi_folder = optarg;
            if (!met.SetIthiFolder(optarg))
            {
                fprintf(stderr, "Cannot set ITHI folder name = %s\n", optarg);
            }
            break;
        case 'd':
            if (!met.SetDateString(optarg))
            {
                fprintf(stderr, "Cannot set date string = %s\n", optarg);
            }
            break;
        case 'y':
            // accuracy_file = optarg;
            fprintf(stderr, "Sorry, Metric M1 not implemented yet.\n");
            break;
        case 'b':
            abuse_file = optarg;
            if (!met.SetAbuseFileName(optarg))
            {
                fprintf(stderr, "Cannot set abuse file name = %s\n", optarg);
            }
            break;
        case 'r':
            root_capture_file = optarg;
            if (!met.SetRootCaptureFileName(optarg))
            {
                fprintf(stderr, "Cannot set root capture file name = %s\n", optarg);
            }
            break;
        case 'e':
            recursive_capture_file = optarg;
            if (!met.SetRecursiveCaptureFileName(optarg))
            {
                fprintf(stderr, "Cannot set recursive capture file name = %s\n", optarg);
            }
            break;
        case 'z':
            root_zone_file = optarg;
            if (!met.SetRootZoneFileName(optarg))
            {
                fprintf(stderr, "Cannot set root zone file name = %s\n", optarg);
            }
            break;
        case 'l':
            fprintf(stderr, "Sorry, Metric M5 not implemented yet.\n");
            // lies_file = optarg;
            break;
        case '?':
            exit_code = usage();
            break;
        case '1':
            if (!met.SetMetricFileNames(0, optarg))
            {
                fprintf(stderr, "Cannot set metric_file_name[1] = %s\n", optarg);
            }
            break;
        case '2':
            if (!met.SetMetricFileNames(1, optarg))
            {
                fprintf(stderr, "Cannot set metric_file_name[2] = %s\n", optarg);
            }
            break;
        case '3':
            if (!met.SetMetricFileNames(2, optarg))
            {
                fprintf(stderr, "Cannot set metric_file_name[3] = %s\n", optarg);
            }
            break;
        case '4':
            if (!met.SetMetricFileNames(3, optarg))
            {
                fprintf(stderr, "Cannot set metric_file_name[4] = %s\n", optarg);
            }
            break;
        case '5':
            if (!met.SetMetricFileNames(4, optarg))
            {
                fprintf(stderr, "Cannot set metric_file_name[5] = %s\n", optarg);
            }
            break;
        case '6':
            if (!met.SetMetricFileNames(5, optarg))
            {
                fprintf(stderr, "Cannot set metric_file_name[6] = %s\n", optarg);
            }
            break;
        case '7':
            if (!met.SetMetricFileNames(6, optarg))
            {
                fprintf(stderr, "Cannot set metric_file_name[7] = %s\n", optarg);
            }
            break;
        case 'w':
            web_root = optarg;
            break;
        }
    }

    if (exit_code != 0)
    {
        return exit_code;
    }

    if (exec_mode == ithi_mode_unspecified)
    {
        /* Exec mode was not specified. Complain. */
        fprintf(stderr, "ITHI execution mode was not specified!\n");
        exit_code = usage();
    }

    /* TODO: should check that arguments are used in a manner consistent with the execution mode */

    if (exec_mode == ithi_mode_capture)
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
    else if (exec_mode == ithi_mode_summary)
    {
        if (capture_summary_list == NULL && optind >= argc)  {
            fprintf(stderr, "No file to merge!\n");
            exit_code = usage();
        }  else {
            if (capture_summary_list == NULL) {
                if (!cs.Merge(argc - optind, (char const **)(argv + optind)))
                {
                    fprintf(stderr, "Cannot merge the input files.\n");
                    exit_code = -1;
                }
            } else if (!cs.Merge(capture_summary_list)) {
                fprintf(stderr, "Cannot merge the listed files.\n");
                exit_code = -1;
            }

            if (exit_code == 0) {
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
    else if (exec_mode == ithi_mode_metrics)
    {
        if (!met.GetMetrics())
        {
            fprintf(stderr, "Cannot compute the ITHI metrics.\n");
            exit_code = -1;
        }
        else
        {
            if (metric_file != NULL)
            {
                if (!met.Save(metric_file))
                {
                    fprintf(stderr, "Cannot save the ITHI metrics in <%s>.\n", metric_file);
                    exit_code = -1;
                }
                else
                {
                    printf("ITHI metrics computed and saved in <%s>\n", metric_file);
                }
            }
            else if (!met.SaveMetricFiles())
            {
                fprintf(stderr, "Cannot save the ITHI metrics.\n");
                exit_code = -1;
            }
            else
            {
                printf("ITHI metrics computed and saved.\n");
            }
        }
    }
    else if (exec_mode == ithi_mode_publish)
    {
        bool ret = true;

        if (ithi_folder == NULL)
        {
            ithi_folder = ITHI_DEFAULT_FOLDER;
        }

        if (web_root == NULL)
        {
            web_root = ITHI_DEFAULT_FOLDER;
        }

        for (int metric_id = 1; ret && metric_id <= 7; metric_id++)
        {
            if (metric_id != 1 && metric_id != 5)
            {
                ithipublisher pub(ithi_folder, metric_id);
                ret = pub.CollectMetricFiles();

                if (!ret)
                {
                    fprintf(stderr, "Cannot collect metric file <%s%sM%d>.\n", ithi_folder, ITHI_FILE_PATH_SEP, metric_id);
                }
                else
                {
                    ret = pub.Publish(web_root);
                    if (!ret)
                    {
                        fprintf(stderr, "Cannot publish json file <%s%sM%d...>.\n", web_root, ITHI_FILE_PATH_SEP, metric_id);
                    }
                }
            }
        }
    }

    return exit_code;
}

