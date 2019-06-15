#!/usr/bin/python
# coding=utf-8
#
# This module manages anonymization of names.

import hashlib
import tld
import ipaddress

public_ipv4 = [
    "0.0.0.0",
    "8.8.8.8", "8.8.4.4", 
    "1.1.1.1", "1.0.0.1",
    "9.9.9.9", "149.112.112.112"
    ]

public_ipv6 = [
    "2001:4860:4860::8888", "2001:4860:4860::8844",
    "2606:4700:4700::1111", "2606:4700:4700::1001",
    "2620:fe::fe", "2620:fe::9",
    "208.67.222.222", "208.67.220.220",
    "2620:119:35::35", "2620:119:53::53"
    ]

def getIpv4Number(token):
    try:
        x = int(token)
        if (x < 0 or x > 255):
            return -1
        else:
            return x
    except:
        return -1

def getIpv6Number(token):
    if (len(token) > 4):
        return -1
    else:
        try:
            x = int(token,16)
            return x
        except:
            return -1

def areIpv4Tokens(tokens, rank):
    is_ipv4 = 0
    if (rank + 3 < len(tokens)):
        i = 0
        while (i < 4):
            if (getIpv4Number(tokens[rank + i]) < 0):
                break
            i += 1
        if (i == 4):
            is_ipv4 = 1
    return is_ipv4

def isIpv4(addr):
    tokens = addr.split(".")
    return areIpv4Tokens(tokens, 0)

def areIpv6Tokens(tokens):
    is_ipv6 = 0
    l = len(tokens)
    if (l > 2 and l <= 8):
        i = 0
        empty_found = 0
        ipv4_found = 0
        while (i < l):
            if (len(tokens[i]) == 0):
                if (i != 0 and i != l-1):
                    if (empty_found == 0):
                        empty_found = 1
                    else:
                        break
            elif (i == l-1 and isIpv4(tokens[i])):
                ipv4_found = 1
            elif (getIpv6Number(tokens[i]) < 0):
                break
            i+=1
        if (i == l and (empty_found > 0 or l+ipv4_found == 8)):
            is_ipv6 = 1
    return is_ipv6

def ipv6Canonic(tokens):
    l = len(tokens)
    i = 0
    addr = ""
    while (i < l):
        if (i != 0):
            addr += ":"
        addr += tokens[i]
        i += 1
    ip6 = str(ipaddress.IPv6Address(addr))
    if (ip6 != addr):
        t = ip6.split(":")
        l6 = len(t)
        i = 0
        while (i < l6 and i < l):
            tokens[i] = t[i]
            i += 1
        while (i < l6):
            tokens.append(t[i])
            i += 1
        while (i < l):
            del(tokens[i])
            l -= 1

class reserved_parts:
    def __init__(self):
        self.li = []
        self.is_sorted = 0

    def add(self, element):
        self.li.append(element)
        self.is_sorted = 0

    def add_list(self, l2):
        for element in l2:
            self.add(element.lower())

    def check(self, target):
        if (len(self.li) == 0):
            return 0
        if (self.is_sorted == 0):
            self.li.sort()
            self.is_sorted = 1
        element = target.lower()
        is_found = 0
        i_low = 0
        i_high = len(self.li) - 1

        if (element < self.li[i_low] or element > self.li[i_high]):
            return 0
        elif (element == self.li[i_low] or element == self.li[i_high]):
            return 1
        else:
            while (i_low + 1 < i_high):
                i_mid = int((i_low + i_high)/2)
                if (element == self.li[i_mid]):
                    return 1
                elif (element < self.li[i_mid]):
                    i_high = i_mid
                else:
                    i_low = i_mid
            return 0

class anonymizer:
    def __init__(self):
        def_key = "key"
        self.encoded_key = def_key.encode('utf-8')
        self.reserved = reserved_parts()
        self.reserved.add_list(tld.reserved)
        self.reserved.add_list(tld.tlds)
        self.reserved.add_list(tld.frequent)
        self.reserved_ip = reserved_parts()
        self.reserved_ip.add_list(public_ipv4)
        self.reserved_ip.add_list(public_ipv6)

    def set_key(self, key):
        self.encoded_key = key.encode('utf-8')

    def anonymizeNamePart(self, part):
        m = hashlib.sha256()
        m.update(self.encoded_key)
        low = part.lower()
        m.update(low.encode('utf-8'))
        m.update(self.encoded_key)
        d = m.digest()
        r = ""
        i = 0
        while (i < len(part)):
            z = part[i]
            x = d[i%32]*256 + d[(i+8)%32]
            if (z.isalpha()):
                y = x % 26
                r += chr(ord('A') + y)
            elif (z.isnumeric()):
                y = x % 10
                r += chr(ord('0') + y)
            else:
                r += z
            i += 1
        return r

    def anonymizeIpv4(self, tokens, rank):
        preserved = 1;
        x = getIpv4Number(tokens[rank])
        if (x == 0 or x == 127 or x >= 224):
            preserved = 4
        elif (x >= 128):
            y = getIpv4Number(tokens[rank+1])
            if ((x == 172 and y == 16) or (x == 192 and y == 168)
                or (x == 169 and y == 254)):
                preserved = 2 
        if (preserved < 4):
            m = hashlib.sha256()
            m.update(self.encoded_key)
            i = 0
            while (i < 4):
                m.update(tokens[rank+i].encode('utf8'))
                i += 1
            m.update(self.encoded_key)
            d = m.digest()
            i = preserved
            while (i < 4): 
                tokens[rank+i] = str(d[i])
                i += 1

    def anonymizeIpv6(self, tokens):
        l = len(tokens)
        m = hashlib.sha256()
        m.update(self.encoded_key)
        i = 0
        while (i < l):
            m.update(tokens[i].encode('utf8'))
            i += 1
        m.update(self.encoded_key)
        d = m.digest()
        i = 1
        while (i < l):
            if (i == l-1 and isIpv4(tokens[i])):
                s = "."
                ipv4_tokens = tokens[i].split(s)
                self.anonymizeIpv4(ipv4_tokens, 0)
                tokens[i] = s.join(ipv4_tokens)
            elif (len(tokens[i]) > 0):
                x = getIpv6Number(tokens[i])
                if (x != 0 and x != 1 and x != 0xFFFF):
                    r = ""
                    j = 0
                    while (j < len(tokens[i])):
                        k = 4*i + j
                        x = d[k%32]&0x0F
                        r += format(x,"x")
                        j += 1
                    tokens[i] = r
            i += 1

    def ipv4Check(self, tokens, rank):
        i = 0
        addr = ""
        while (i < 4):
            if (i > 0):
                addr += "."
            addr += tokens[rank+i]
            i += 1
        return self.reserved_ip.check(addr)

    def anonymizeName(self, name, level):
        s = "."
        tokens = name.split(s)
        l = len(tokens)
        i = 0
        while (i + level < l):
            if (areIpv4Tokens(tokens, i) > 0):
                if (self.ipv4Check(tokens, i) == 0):
                    self.anonymizeIpv4(tokens, i)
                i += 3
            elif (self.reserved.check(tokens[i]) == 0):
                tokens[i] = self.anonymizeNamePart(tokens[i])
            i += 1
        r = s.join(tokens)
        return r

    def anonymizeAddress(self, addr_raw):
        try:
            addr = str(ipaddress.IPAddress(addr_raw))
        except:
            addr = addr_raw
        if (self.reserved_ip.check(addr) != 0):
            return addr
        else:
            s = "."
            tokens = addr.split(s)
            if (len(tokens) == 4 and areIpv4Tokens(tokens, 0) > 0):
                self.anonymizeIpv4(tokens, 0)
                return s.join(tokens)
            else:
                s = ":"
                tokens = addr.split(s)
                if (areIpv6Tokens(tokens) > 0):
                    ipv6Canonic(tokens)
                    self.anonymizeIpv6(tokens)
                return s.join(tokens)


