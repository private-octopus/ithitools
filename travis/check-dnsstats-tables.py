#!/usr/bin/env python

import re, sys, csv
try:
	# For Python 3.0 and later
	from urllib.request import urlopen
except ImportError:
	# Fall back to Python 2's urllib2
	from urllib2 import urlopen

if __name__ == '__main__':
	fail = False
	dnsstats = open('lib/DnsStats.cpp').read()
	cpp_comments = re.compile("(/\*.*?\*/|//.*$)",re.DOTALL)

	tlds_part         = re.sub(cpp_comments, "", dnsstats.split(
	       'RegisteredTldName[] = {')[1].split('\n};')[0])
	tlds              = set(eval('[' + tlds_part         + ']'))
	rfc6761_tlds_part = re.sub(cpp_comments, "", dnsstats.split(
	             'rfc6761_tld[] = {')[1].split('\n};')[0])
	rfc6761_tlds      = set(eval('[' + rfc6761_tlds_part + ']'))
	roots_part        = re.sub(cpp_comments, "", dnsstats.split(
	    'DefaultRootAddresses[] = {')[1].split('\n};')[0])
	roots             = set(eval('[' + roots_part        + ']'))
	
	iana_tlds_txt = urlopen(
	    "https://data.iana.org/TLD/tlds-alpha-by-domain.txt").read()
	iana_tlds = set(iana_tlds_txt.decode('utf-8').strip().split('\n')[1:])
	if tlds != iana_tlds:
		fail = True
		if iana_tlds - tlds:
			print( 'TLDs to add: "%s"'
			     % '", "'.join(sorted(iana_tlds - tlds)))
		if tlds - iana_tlds:
			print( 'TLDs to remove: "%s"'
			     % '", "'.join(sorted(tlds - iana_tlds)))

	special_use_domain_names_txt = urlopen(
	    "https://www.iana.org/assignments/special-use-domain-names/" + 
	    "special-use-domain.csv").read().decode('utf-8')
	special_use_domain_names = [ln[0] for ln in csv.reader(
	    special_use_domain_names_txt.strip().upper().split('\n'))][1:]
	iana_rfc6761_tlds = set(map(lambda x: x.split('.')[-2],
	    special_use_domain_names)) - iana_tlds

	if rfc6761_tlds != iana_rfc6761_tlds:
		fail = True
		if iana_rfc6761_tlds - rfc6761_tlds:
			print( 'RFC6761 TLDs to add: "%s"' 
			     % '", "'.join(
				     sorted(iana_rfc6761_tlds - rfc6761_tlds)))
		if rfc6761_tlds - iana_rfc6761_tlds:
			print( 'RFC6761 TLDs to remove: "%s"'
			     % '", "'.join(
				     sorted(rfc6761_tlds - iana_rfc6761_tlds)))

	root_hints = urlopen("https://www.internic.net/domain/named.root") \
		     .read().decode('utf-8')
	root_hints = set([ln.split()[-1] for ln in root_hints.split('\n')
	                                 if ln[0].isalpha()])
	if roots != root_hints:
		if root_hints - roots:
			fail = True
			print( 'Root servers to add: "%s"'
			     % '", "'.join(sorted(root_hints - roots)))
		if roots - root_hints:
			# Not a failure, because the old ones are kept
			# working for a while
			print( 'Root servers not in root.hints: "%s"' 
			     % '", "'.join(sorted(roots - root_hints)))
	if fail:
		sys.exit(1)
