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

#ifndef LIB_ITHICAP_H
#define LIB_ITHICAP_H

/*
 * This is the definition of the interface functions exposed by the
 * ITHICAP plugin for DNSCAP. The definitions have a dependency on
 * "dnscap_common.h", which should really be included in front of
 * this plugin.
 */

#ifdef  __cplusplus
extern "C" {
#endif
    void libithicap_usage();

    void libithicap_getopt(int* argc, char** argv[]);

    int libithicap_start(logerr_t* a_logerr);

    void libithicap_stop();

    int libithicap_open(my_bpftimeval ts);

    int libithicap_close(my_bpftimeval ts);

    void libithicap_output(const char* descr, iaddr from, iaddr to, uint8_t proto,
        unsigned flags,
        unsigned sport, unsigned dport, my_bpftimeval ts,
        const u_char* pkt_copy, const unsigned olen,
        const u_char* payload, const unsigned payloadlen);

#ifdef  __cplusplus
}
#endif

#endif