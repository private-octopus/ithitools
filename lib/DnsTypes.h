/*
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

#ifndef DNSTYPE_H
#define DNSTYPE_H

enum DnsRtype {
    DnsRtype_A = 1, /* a host address */
    DnsRtype_NS = 2, /* an authoritative name server */
    DnsRtype_MD = 3, /* a mail destination (Obsolete - use MX) */
    DnsRtype_MF = 4, /* a mail forwarder (Obsolete - use MX) */
    DnsRtype_CNAME = 5, /* the canonical name for an alias */
    DnsRtype_SOA = 6, /* marks the start of a zone of authority */
    DnsRtype_MB = 7, /* a mailbox domain name (EXPERIMENTAL) */
    DnsRtype_MG = 8, /* a mail group member (EXPERIMENTAL) */
    DnsRtype_MR = 9, /* a mail rename domain name (EXPERIMENTAL) */
    DnsRtype_NULL = 10, /* a null RR (EXPERIMENTAL) */
    DnsRtype_WKS = 11, /* a well known service description */
    DnsRtype_PTR = 12, /* a domain name pointer */
    DnsRtype_HINFO = 13, /* host information */
    DnsRtype_MINFO = 14, /* mailbox or mail list information */
    DnsRtype_MX = 15, /* mail exchange */
    DnsRtype_TXT = 16, /* text strings */
    DnsRtype_AAAA = 28, /* Service record */
    DnsRtype_SRV = 33, /* Service record */
    DnsRtype_OPT = 41, /* EDNS0 OPT record */
    DnsRtype_DS = 43, /* DNSSEC DS */
    DnsRtype_RRSIG = 46, /* DNSSEC RRSIG */
    DnsRtype_DNSKEY = 48, /* DNSSEC KEY */
    DnsRtype_TLSA = 52, /* DANE Certificate */ 
    DnsRtype_TSIG = 250, /* Transaction Signature */
    DnsRtype_ANY = 255, /*Not a DNS type, but a DNS query type, meaning "all types"*/
    DnsRtype_UNEXPECTED = 0 /*Not a DNS type, indicates a parsing error */
};

#endif /* DNSTYPE_H */

