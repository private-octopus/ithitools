#!/usr/bin/python
# coding=utf-8
#
# This script takes the prefix list from the main file
import codecs
import encodings.idna
import re
import sys

def _normalizeHostname(domain):
  """
  Normalizes the given domain, component by component.  ASCII components are
  lowercased, while non-ASCII components are processed using the ToASCII
  algorithm.
  """
  def convertLabel(label):
    if _isASCII(label):
      return label.lower()
    xnstr = encodings.idna.ToASCII(label);
    return xnstr.decode("ascii");
  return ".".join(map(convertLabel, domain.split(".")))

def _isASCII(s):
  "True if s consists entirely of ASCII characters, false otherwise."
  for c in s:
    if ord(c) > 127:
      return False
  return True

def _reverseDomain(domainName):
    "provide a reversed domain name, easier to sort and compare"
    tokens = domainName.split(".")
    reversed = ""
    l = len(tokens)
    if l > 0:
        tok0 = tokens[0]
        if tok0[0] == '!':
            tokens[0] = tok0[1:] + '!'
        else:
            if tok0[0] == '*':
                tokens[0] = '~'
    while l > 0 :
        l -= 1
        reversed += tokens[l]
        if l > 0:
            reversed += "."
    return reversed


def _loadPrefixTable(path):
    "Create a set of prefix entries by parsing the prefix file."
    file = codecs.open(path, "r", "UTF-8")
    ldomains = set()
    while True: 
        line = file.readline()
        # line always contains a line terminator unless the file is empty
        if len(line) == 0:
            break;
        # comment, empty, or superfluous line for explicitness purposes
        if line.startswith("//"):
            continue;
        line = line.rstrip()
        if  "." not in line and len(line) == 0:
            continue;
        ldomains.add(_normalizeHostname(line))
    file.close();
    return ldomains;

# Main loop

domains = _loadPrefixTable("C:\\Users\\Christian\\Documents\\GitHub\\list\\public_suffix_list.dat");
is_first = 1
print("char const * dnsPrefixList[] = {");
sep = ''
for d in domains:
    if is_first == 1 :
        sep = '  '
        is_first = 0
    else:
        sep = ', '
    print(sep + '"' + d.upper() + '"')
print("};");
print("");
print("size_t nbDnsPrefixList = sizeof(dnsPrefixList)/sizeof(char const *);");
