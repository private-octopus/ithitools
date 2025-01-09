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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include "pcap_reader.h"
#include "DnsTypes.h"
#include "Version.h"
#include "ithiutil.h"
#include "DnsStats.h"

DnsStats::DnsStats()
    :
    is_capture_dns_only(true),
    is_capture_stopped(false),
    t_start_sec(0),
    t_start_usec(0),
    duration_usec(0),
    volume_53only(0),
    enable_frequent_address_filtering(false),
    capture_cache_ratio_nx_domain(false),
    target_number_dns_packets(0),
    frequent_address_max_count(128),
    max_tld_leakage_count(0x80),
    max_tld_leakage_table_count(0x8000),
    max_query_usage_count(8000000),
    max_tld_string_usage_count(0x8000),
    max_tld_string_leakage_count(0x200),
    max_stats_by_ip_count(0x8000),
    dnsstat_flags(0),
    record_count(0),
    query_count(0),
    response_count(0),
    error_flags(0),
    dnssec_name_index(0),
    dnssec_packet(NULL),
    dnssec_packet_length(0),
    is_do_flag_set(false),
    is_using_edns(false),
    edns_options(NULL),
    edns_options_length(0),
    is_qname_minimized(false),
    is_recursive_query(false),
    address_report(NULL),
    name_report(NULL),
    compress_name_and_address_reports(false)
{
}


DnsStats::~DnsStats()
{
}

static char const * DefaultRootAddresses[] = {
    "2001:503:ba3e::2:30",
    "198.41.0.4",
    "192.228.79.201", /* Since 2023-11-27, b.root-servers.net changed to 170.247.170.2 */
    "170.247.170.2",   /* See https://b.root-servers.org/news/2023/05/16/new-addresses.html */
    "2001:500:2::c",
    "192.33.4.12",
    "2001:500:2d::d",
    "199.7.91.13",
    "2001:500:a8::e",
    "192.203.230.10",
    "2001:500:2f::f",
    "192.5.5.241",
    "2001:500:12::d0d",
    "192.112.36.4",
    "2001:500:1::53",
    "198.97.190.53",
    "2001:7fe::53",
    "192.36.148.17",
    "2001:503:c27::2:30",
    "192.58.128.30",
    "2001:7fd::1",
    "193.0.14.129",
    "2001:500:9f::42",
    "199.7.83.42",
    "2001:dc3::35",
    "202.12.27.33",
    "170.247.170.2",
    "2801:1b8:10::b" /* https://b.root-servers.org/news/2023/05/16/new-addresses.html  */

};

static char const * RegistryNameById[] = {
    "ITHITOOLS_VERSION",
    "CLASS",
    "RR Type",
    "OpCode",
    "RCODE",
    "AFSDB RRSubtype",
    "DHCID RRIdType",
    "Label Type",
    "EDNS OPT CODE",
    "Header Flags",
    "EDNS Header_Flags",
    "EDNS Version number",
    "CSYNC Flags",
    "DNSSEC Algorithm Numbers",
    "DNSSEC KEY Prime Lengths",
    "Q-CLASS",
    "Q-RR Type",
    "DNSSEC Well Known Primes",
    "EDNS Packet Size",
    "Query Size",
    "Response Size",
    "TC Length",
    "Z-Q-TLD",
    "Z-R-TLD",
    "Z-Error Flags",
    "TLD Error Class",
    "Underlined part",
    "root-QR",
    "LeakByLength",
    "LeakedTLD",
    "RFC6761-TLD",
    "UsefulQueries",
    "DANE_CertUsage",
    "DANE_TlsaSelector",
    "DANE_TlsaMatchingType",
    "FrequentAddress",
    "TldUsage",
    "RFC6761Usage",
    "Frequent-TLD-usage",
    "TLD-Usage-Count",
    "Local_TLD_Usage_Count",
    "DNSSEC_Client_Usage",
    "DNSSEC_Zone_Usage",
    "EDNS_CLIENT_USAGE",
    "QNAME_MINIMIZATION_USAGE",
    "EDNS_OPTIONS_USAGE",
    "EDNS_OPTIONS_TOTAL_QUERIES",
    "VOLUME_PER_PROTO",
    "TCPSYN_PER_PROTO",
    "CAPTURE_DURATION",
    "VOLUME_53ONLY",
    "CAPTURE_DURATION53",
    "LeakedTLD_BINARY",
    "LeakedTLD_SYNTAX",
    "LeakedTLD_IPV4",
    "LeakedTLD_NUMERIC",
    "Leaked2ndLD",
    "ADDRESS_LIST",
    "FULL_NAME_LIST",
    "TLD_MIN_DELAY_IP",
    "TLD_AVG_DELAY_IP",
    "TLD_MIN_DELAY_LOAD",
    "ADDRESS_DELAY",
    "NAME_PARTS_COUNT",
    "CHROMIUM_PROBES",
    "SENDING_RECURSIVE_SET",
    "CHROMIUM_LEAK_REF",
    "DEBUG"
};

static uint32_t RegistryNameByIdNb = sizeof(RegistryNameById) / sizeof(char const*);

char const * RegisteredTldName[] = {
    "AAA", "AARP", "ABB", "ABBOTT", "ABBVIE", "ABC", "ABLE", "ABOGADO",
    "ABUDHABI", "AC", "ACADEMY", "ACCENTURE", "ACCOUNTANT", "ACCOUNTANTS", "ACO",
    "ACTOR", "AD", "ADS", "ADULT", "AE", "AEG", "AERO", "AETNA",
    "AF", "AFL", "AFRICA", "AG", "AGAKHAN", "AGENCY", "AI",
    "AIG", "AIRBUS", "AIRFORCE", "AIRTEL", "AKDN", "AL",
    "ALIBABA", "ALIPAY", "ALLFINANZ", "ALLSTATE", "ALLY", "ALSACE", "ALSTOM", "AM",
    "AMAZON",
    "AMERICANEXPRESS", "AMERICANFAMILY", "AMEX", "AMFAM", "AMICA", "AMSTERDAM",
    "ANALYTICS", "ANDROID", "ANQUAN", "ANZ", "AO", "AOL", "APARTMENTS", "APP",
    "APPLE", "AQ", "AQUARELLE", "AR", "ARAB", "ARAMCO", "ARCHI", "ARMY", "ARPA",
    "ART", "ARTE", "AS", "ASDA", "ASIA", "ASSOCIATES", "AT", "ATHLETA", "ATTORNEY",
    "AU", "AUCTION", "AUDI", "AUDIBLE", "AUDIO", "AUSPOST", "AUTHOR", "AUTO", "AUTOS",
    "AW", "AWS", "AX", "AXA", "AZ", "AZURE", "BA", "BABY", "BAIDU", "BANAMEX",
    "BAND", "BANK", "BAR", "BARCELONA", "BARCLAYCARD", "BARCLAYS",
    "BAREFOOT", "BARGAINS", "BASEBALL", "BASKETBALL", "BAUHAUS", "BAYERN", "BB", "BBC",
    "BBT", "BBVA", "BCG", "BCN", "BD", "BE", "BEATS", "BEAUTY", "BEER", "BENTLEY", "BERLIN",
    "BEST", "BESTBUY", "BET", "BF", "BG", "BH", "BHARTI", "BI", "BIBLE", "BID", "BIKE",
    "BING", "BINGO", "BIO", "BIZ", "BJ", "BLACK", "BLACKFRIDAY", "BLOCKBUSTER",
    "BLOG", "BLOOMBERG", "BLUE", "BM", "BMS", "BMW", "BN", "BNPPARIBAS", "BO", "BOATS",
    "BOEHRINGER", "BOFA", "BOM", "BOND", "BOO", "BOOK", "BOOKING", "BOSCH", "BOSTIK",
    "BOSTON", "BOT", "BOUTIQUE", "BOX", "BR", "BRADESCO", "BRIDGESTONE", "BROADWAY", "BROKER",
    "BROTHER", "BRUSSELS", "BS", "BT", "BUILD", "BUILDERS", "BUSINESS",
    "BUY", "BUZZ", "BV", "BW", "BY", "BZ", "BZH", "CA", "CAB", "CAFE", "CAL", "CALL",
    "CALVINKLEIN", "CAM", "CAMERA", "CAMP", "CANON", "CAPETOWN", "CAPITAL",
    "CAPITALONE", "CAR", "CARAVAN", "CARDS", "CARE", "CAREER", "CAREERS", "CARS",
    "CASA", "CASE", "CASH", "CASINO", "CAT", "CATERING", "CATHOLIC", "CBA", "CBN",
    "CBRE", "CC", "CD", "CENTER", "CEO", "CERN", "CF", "CFA", "CFD", "CG", "CH",
    "CHANEL", "CHANNEL", "CHARITY", "CHASE", "CHAT", "CHEAP", "CHINTAI", "CHRISTMAS", "CHROME",
    "CHURCH", "CI", "CIPRIANI", "CIRCLE", "CISCO", "CITADEL", "CITI", "CITIC", "CITY",
    "CK", "CL", "CLAIMS", "CLEANING", "CLICK", "CLINIC", "CLINIQUE", "CLOTHING",
    "CLOUD", "CLUB", "CLUBMED", "CM", "CN", "CO", "COACH", "CODES", "COFFEE", "COLLEGE",
    "COLOGNE", "COM", "COMMBANK", "COMMUNITY", "COMPANY", "COMPARE", "COMPUTER",
    "COMSEC", "CONDOS", "CONSTRUCTION", "CONSULTING", "CONTACT", "CONTRACTORS", "COOKING",
    "COOL", "COOP", "CORSICA", "COUNTRY", "COUPON", "COUPONS", "COURSES",
    "CPA",
    "CR", "CREDIT", "CREDITCARD", "CREDITUNION", "CRICKET", "CROWN", "CRS", "CRUISE",
    "CRUISES", "CU", "CUISINELLA", "CV", "CW", "CX", "CY", "CYMRU", "CYOU", "CZ",
    "DAD", "DANCE", "DATA", "DATE", "DATING", "DATSUN", "DAY", "DCLK", "DDS", "DE",
    "DEAL", "DEALER", "DEALS", "DEGREE", "DELIVERY", "DELL", "DELOITTE", "DELTA", "DEMOCRAT",
    "DENTAL", "DENTIST", "DESI", "DESIGN", "DEV", "DHL", "DIAMONDS", "DIET", "DIGITAL",
    "DIRECT", "DIRECTORY", "DISCOUNT", "DISCOVER", "DISH", "DIY", "DJ", "DK", "DM", "DNP",
    "DO", "DOCS", "DOCTOR", "DOG", "DOMAINS", "DOT", "DOWNLOAD", "DRIVE",
    "DTV", "DUBAI", "DUNLOP", "DUPONT", "DURBAN", "DVAG", "DVR", "DZ",
    "EARTH", "EAT", "EC", "ECO", "EDEKA", "EDU", "EDUCATION", "EE", "EG", "EMAIL", "EMERCK",
    "ENERGY", "ENGINEER", "ENGINEERING", "ENTERPRISES", "EPSON", "EQUIPMENT", "ER",
    "ERICSSON", "ERNI", "ES", "ESQ", "ESTATE", "ET", "EU",
    "EUROVISION", "EUS", "EVENTS", "EXCHANGE", "EXPERT", "EXPOSED", "EXPRESS",
    "EXTRASPACE", "FAGE", "FAIL", "FAIRWINDS", "FAITH", "FAMILY", "FAN", "FANS", "FARM",
    "FARMERS", "FASHION", "FAST", "FEDEX", "FEEDBACK", "FERRARI", "FERRERO", "FI",
    "FIDELITY", "FIDO", "FILM", "FINAL", "FINANCE", "FINANCIAL", "FIRE", "FIRESTONE",
    "FIRMDALE", "FISH", "FISHING", "FIT", "FITNESS", "FJ", "FK", "FLICKR", "FLIGHTS", "FLIR",
    "FLORIST", "FLOWERS", "FLY", "FM", "FO", "FOO", "FOOD", "FOOTBALL", "FORD",
    "FOREX", "FORSALE", "FORUM", "FOUNDATION", "FOX", "FR", "FREE", "FRESENIUS", "FRL",
    "FROGANS", "FRONTIER", "FTR", "FUJITSU", "FUN", "FUND",
    "FURNITURE", "FUTBOL", "FYI", "GA", "GAL", "GALLERY", "GALLO", "GALLUP", "GAME", "GAMES",
    "GAP", "GARDEN", "GAY", "GB", "GBIZ", "GD", "GDN", "GE", "GEA", "GENT", "GENTING", "GEORGE",
    "GF", "GG", "GGEE", "GH", "GI", "GIFT", "GIFTS", "GIVES", "GIVING", "GL",
    "GLASS", "GLE", "GLOBAL", "GLOBO", "GM", "GMAIL", "GMBH", "GMO", "GMX", "GN", "GODADDY",
    "GOLD", "GOLDPOINT", "GOLF", "GOO", "GOODYEAR", "GOOG", "GOOGLE", "GOP",
    "GOT", "GOV", "GP", "GQ", "GR", "GRAINGER", "GRAPHICS", "GRATIS", "GREEN", "GRIPE",
    "GROCERY", "GROUP", "GS", "GT", "GU", "GUCCI", "GUGE", "GUIDE", "GUITARS",
    "GURU", "GW", "GY", "HAIR", "HAMBURG", "HANGOUT", "HAUS", "HBO", "HDFC", "HDFCBANK",
    "HEALTH", "HEALTHCARE", "HELP", "HELSINKI", "HERE", "HERMES", "HIPHOP",
    "HISAMITSU", "HITACHI", "HIV", "HK", "HKT", "HM", "HN", "HOCKEY", "HOLDINGS", "HOLIDAY",
    "HOMEDEPOT", "HOMEGOODS", "HOMES", "HOMESENSE", "HONDA", "HORSE",
    "HOSPITAL", "HOST", "HOSTING", "HOT", "HOTELS", "HOTMAIL", "HOUSE", "HOW",
    "HR", "HSBC", "HT", "HU", "HUGHES", "HYATT", "HYUNDAI", "IBM", "ICBC", "ICE",
    "ICU", "ID", "IE", "IEEE", "IFM", "IKANO", "IL", "IM", "IMAMAT", "IMDB", "IMMO",
    "IMMOBILIEN", "IN", "INDUSTRIES", "INFINITI", "INFO", "INC", "ING", "INK", "INSTITUTE",
    "INSURANCE", "INSURE", "INT", "INTERNATIONAL", "INTUIT", "INVESTMENTS", "IO",
    "IPIRANGA", "IQ", "IR", "IRISH", "IS", "ISMAILI", "IST", "ISTANBUL", "IT",
    "ITAU", "ITV", "JAGUAR", "JAVA", "JCB", "JE", "JEEP", "JETZT",
    "JEWELRY", "JIO", "JLL", "JM", "JMP", "JNJ", "JO", "JOBS", "JOBURG", "JOT", "JOY",
    "JP", "JPMORGAN", "JPRS", "JUEGOS", "JUNIPER", "KAUFEN", "KDDI", "KE", "KERRYHOTELS",
    "KERRYLOGISTICS", "KERRYPROPERTIES", "KFH", "KG", "KH", "KI", "KIA",
    "KIDS", "KIM",
    "KINDLE", "KITCHEN", "KIWI", "KM", "KN", "KOELN", "KOMATSU", "KOSHER", "KP", "KPMG",
    "KPN", "KR", "KRD", "KRED", "KUOKGROUP", "KW", "KY", "KYOTO", "KZ", "LA", "LACAIXA",
    "LAMBORGHINI", "LAMER", "LANCASTER", "LAND",
    "LANDROVER", "LANXESS", "LASALLE", "LAT", "LATINO", "LATROBE", "LAW", "LAWYER", "LB",
    "LC", "LDS", "LEASE", "LECLERC", "LEFRAK", "LEGAL", "LEGO", "LEXUS", "LGBT", "LI",
    "LIDL", "LIFE", "LIFEINSURANCE", "LIFESTYLE", "LIGHTING", "LIKE", "LILLY",
    "LIMITED", "LIMO", "LINCOLN", "LINK", "LIPSY", "LIVE", "LIVING", "LK", "LLC", "LLP",
    "LOAN", "LOANS", "LOCKER", "LOCUS", "LOL", "LONDON", "LOTTE", "LOTTO", "LOVE",
    "LPL", "LPLFINANCIAL", "LR", "LS", "LT", "LTD", "LTDA", "LU", "LUNDBECK",
    "LUXE", "LUXURY", "LV", "LY", "MA", "MADRID", "MAIF", "MAISON", "MAKEUP", "MAN",
    "MANAGEMENT", "MANGO", "MAP", "MARKET", "MARKETING", "MARKETS", "MARRIOTT", "MARSHALLS",
    "MATTEL", "MBA", "MC", "MCKINSEY", "MD", "ME", "MED", "MEDIA", "MEET",
    "MELBOURNE", "MEME", "MEMORIAL", "MEN", "MENU", "MERCKMSD", "MG", "MH",
    "MIAMI", "MICROSOFT", "MIL", "MINI", "MINT", "MIT", "MITSUBISHI", "MK", "ML", "MLB",
    "MLS", "MM", "MMA", "MN", "MO", "MOBI", "MOBILE", "MODA", "MOE", "MOI", "MOM",
    "MONASH", "MONEY", "MONSTER", "MORMON", "MORTGAGE", "MOSCOW", "MOTO",
    "MOTORCYCLES", "MOV", "MOVIE", "MP", "MQ", "MR", "MS", "MSD", "MT", "MTN",
    "MTR", "MU", "MUSIC",
    "MUSEUM", "MV", "MW", "MX", "MY", "MZ", "NA", "NAB",
    "NAGOYA", "NAME", "NAVY", "NBA", "NC", "NE", "NEC", "NET",
    "NETBANK", "NETFLIX", "NETWORK", "NEUSTAR", "NEW", "NEWS", "NEXT",
    "NEXTDIRECT", "NEXUS", "NF", "NFL", "NG", "NGO", "NHK", "NI", "NICO", "NIKE", "NIKON",
    "NINJA", "NISSAN", "NISSAY", "NL", "NO", "NOKIA", "NORTON", "NOW",
    "NOWRUZ", "NOWTV", "NP", "NR", "NRA", "NRW", "NTT", "NU", "NYC", "NZ", "OBI",
    "OBSERVER", "OFFICE", "OKINAWA", "OLAYAN", "OLAYANGROUP", "OLLO",
    "OM", "OMEGA", "ONE", "ONG", "ONL", "ONLINE", "OOO", "OPEN", "ORACLE",
    "ORANGE", "ORG", "ORGANIC", "ORIGINS", "OSAKA", "OTSUKA", "OTT", "OVH", "PA", "PAGE",
    "PANASONIC", "PARIS", "PARS", "PARTNERS", "PARTS", "PARTY",
    "PAY", "PCCW", "PE", "PET", "PF", "PFIZER", "PG", "PH", "PHARMACY", "PHD", "PHILIPS",
    "PHONE", "PHOTO", "PHOTOGRAPHY", "PHOTOS", "PHYSIO", "PICS", "PICTET",
    "PICTURES", "PID", "PIN", "PING", "PINK", "PIONEER", "PIZZA", "PK", "PL", "PLACE",
    "PLAY", "PLAYSTATION", "PLUMBING", "PLUS", "PM", "PN", "PNC", "POHL", "POKER", "POLITIE",
    "PORN", "POST", "PR", "PRAMERICA", "PRAXI", "PRESS", "PRIME", "PRO", "PROD",
    "PRODUCTIONS", "PROF", "PROGRESSIVE", "PROMO", "PROPERTIES", "PROPERTY", "PROTECTION",
    "PRU", "PRUDENTIAL", "PS", "PT", "PUB", "PW", "PWC", "PY", "QA", "QPON", "QUEBEC",
    "QUEST", "RACING", "RADIO", "RE", "READ", "REALESTATE", "REALTOR",
    "REALTY", "RECIPES", "RED", "REDSTONE", "REDUMBRELLA", "REHAB", "REISE", "REISEN",
    "REIT", "RELIANCE", "REN", "RENT", "RENTALS", "REPAIR", "REPORT", "REPUBLICAN", "REST",
    "RESTAURANT", "REVIEW", "REVIEWS", "REXROTH", "RICH", "RICHARDLI", "RICOH",
    "RIL", "RIO", "RIP", "RO", "ROCKS", "RODEO", "ROGERS",
    "ROOM", "RS", "RSVP", "RU", "RUGBY", "RUHR", "RUN", "RW", "RWE", "RYUKYU", "SA",
    "SAARLAND", "SAFE", "SAFETY", "SAKURA", "SALE", "SALON", "SAMSCLUB", "SAMSUNG",
    "SANDVIK", "SANDVIKCOROMANT", "SANOFI", "SAP", "SARL", "SAS", "SAVE", "SAXO",
    "SB", "SBI", "SBS", "SC", "SCB", "SCHAEFFLER", "SCHMIDT", "SCHOLARSHIPS",
    "SCHOOL", "SCHULE", "SCHWARZ", "SCIENCE", "SCOT", "SD", "SE",
    "SEARCH", "SEAT", "SECURE", "SECURITY", "SEEK", "SELECT", "SENER", "SERVICES",
    "SEVEN", "SEW", "SEX", "SEXY", "SFR", "SG", "SH", "SHANGRILA", "SHARP", "SHELL",
    "SHIA", "SHIKSHA", "SHOES", "SHOP", "SHOPPING", "SHOUJI", "SHOW",
    "SI", "SILK", "SINA", "SINGLES", "SITE", "SJ", "SK", "SKI", "SKIN", "SKY", "SKYPE", "SL",
    "SLING", "SM", "SMART", "SMILE", "SN", "SNCF", "SO", "SOCCER", "SOCIAL", "SOFTBANK",
    "SOFTWARE", "SOHU", "SOLAR", "SOLUTIONS", "SONG", "SONY", "SOY", "SPA", "SPACE", "SPORT",
    "SPOT", "SR", "SRL", "SS", 
    "ST", "STADA", "STAPLES", "STAR", 
    "STATEBANK", "STATEFARM", "STC", "STCGROUP", "STOCKHOLM", "STORAGE", "STORE",
    "STREAM", "STUDIO", "STUDY", "STYLE", "SU", "SUCKS", "SUPPLIES", "SUPPLY", "SUPPORT",
    "SURF", "SURGERY", "SUZUKI", "SV", "SWATCH", "SWISS", "SX", "SY",
    "SYDNEY", "SYSTEMS", "SZ", "TAB", "TAIPEI", "TALK", "TAOBAO", "TARGET", "TATAMOTORS",
    "TATAR", "TATTOO", "TAX", "TAXI", "TC", "TCI", "TD", "TDK", "TEAM", "TECH", "TECHNOLOGY",
    "TEL", "TEMASEK", "TENNIS", "TEVA", "TF", "TG", "TH", "THD",
    "THEATER", "THEATRE", "TIAA", "TICKETS", "TIENDA", "TIPS", "TIRES", "TIROL",
    "TJ", "TJMAXX", "TJX", "TK", "TKMAXX", "TL", "TM", "TMALL", "TN", "TO", "TODAY", "TOKYO",
    "TOOLS", "TOP", "TORAY", "TOSHIBA", "TOTAL", "TOURS", "TOWN", "TOYOTA", "TOYS", "TR",
    "TRADE", "TRADING", "TRAINING", "TRAVEL", "TRAVELERS",
    "TRAVELERSINSURANCE", "TRUST", "TRV", "TT", "TUBE", "TUI", "TUNES", "TUSHU", "TV", "TVS",
    "TW", "TZ", "UA", "UBANK", "UBS", "UG", "UK", "UNICOM", "UNIVERSITY", "UNO",
    "UOL", "UPS", "US", "UY", "UZ", "VA", "VACATIONS", "VANA", "VANGUARD", "VC", "VE",
    "VEGAS", "VENTURES", "VERISIGN", "VERSICHERUNG", "VET", "VG", "VI", "VIAJES", "VIDEO",
    "VIG", "VIKING", "VILLAS", "VIN", "VIP", "VIRGIN", "VISA", "VISION",
    "VIVA", "VIVO", "VLAANDEREN", "VN", "VODKA", "VOLVO", "VOTE",
    "VOTING", "VOTO", "VOYAGE", "VU", "WALES", "WALMART", "WALTER", "WANG",
    "WANGGOU", "WATCH", "WATCHES", "WEATHER", "WEATHERCHANNEL", "WEBCAM", "WEBER",
    "WEBSITE", "WED", "WEDDING", "WEIBO", "WEIR", "WF", "WHOSWHO", "WIEN", "WIKI",
    "WILLIAMHILL", "WIN", "WINDOWS", "WINE", "WINNERS", "WME", "WOLTERSKLUWER", "WOODSIDE",
    "WORK", "WORKS", "WORLD", "WOW", "WS", "WTC", "WTF", "XBOX", "XEROX",
    "XIHUAN", "XIN", "XN--11B4C3D", "XN--1CK2E1B", "XN--1QQW23A", "XN--2SCRJ9C",
    "XN--30RR7Y", "XN--3BST00M", "XN--3DS443G", "XN--3E0B707E", "XN--3HCRJ9C",
    "XN--3PXU8K", "XN--42C2D9A", "XN--45BR5CYL", "XN--45BRJ9C",
    "XN--45Q11C", "XN--4DBRK0CE", "XN--4GBRIM", "XN--54B7FTA0CC", "XN--55QW42G", "XN--55QX5D",
    "XN--5SU34J936BGSG", "XN--5TZM5G", "XN--6FRZ82G", "XN--6QQ986B3XL", "XN--80ADXHKS",
    "XN--80AO21A", "XN--80AQECDR1A", "XN--80ASEHDB", "XN--80ASWG", "XN--8Y0A063A",
    "XN--90A3AC", "XN--90AE", "XN--90AIS", "XN--9DBQ2A", "XN--9ET52U", "XN--9KRT00A",
    "XN--B4W605FERD", "XN--BCK1B9A5DRE4C", "XN--C1AVG", "XN--C2BR7G", "XN--CCK2B3B",
    "XN--CCKWCXETD", 
    "XN--CG4BKI", "XN--CLCHC0EA0B2G2A9GCD", "XN--CZR694B", "XN--CZRS0T", "XN--CZRU2D",
    "XN--D1ACJ3B", "XN--D1ALF", "XN--E1A4C", "XN--ECKVDTC9D", "XN--EFVY88H",
    "XN--FCT429K", "XN--FHBEI", "XN--FIQ228C5HS", "XN--FIQ64B", "XN--FIQS8S", "XN--FIQZ9S",
    "XN--FJQ720A", "XN--FLW351E", "XN--FPCRJ9C3D", "XN--FZC2C9E2C", "XN--FZYS8D69UVGM",
    "XN--G2XX48C", "XN--GCKR3F0F", "XN--GECRJ9C", "XN--GK3AT1E", "XN--H2BREG3EVE",
    "XN--H2BRJ9C", "XN--H2BRJ9C8C", "XN--HXT814E", "XN--I1B6B1A6A2E", "XN--IMR513N",
    "XN--IO0A7I", "XN--J1AEF", "XN--J1AMH", "XN--J6W193G", 
    "XN--JLQ480N2RG", "XN--JVR189M",
    "XN--KCRX77D1X4A", "XN--KPRW13D", "XN--KPRY57D", "XN--KPUT3I",
    "XN--L1ACC", "XN--LGBBAT1AD8J", "XN--MGB9AWBF", "XN--MGBA3A3EJT", "XN--MGBA3A4F16A",
    "XN--MGBA7C0BBN0A", "XN--MGBAAM7A8H", "XN--MGBAB2BD",
    "XN--MGBAH1A3HJKRD",
    "XN--MGBAI9AZGQP6J", "XN--MGBAYH7GPA", "XN--MGBBH1A", "XN--MGBBH1A71E",
    "XN--MGBC0A9AZCG", "XN--MGBCA7DZDO",  "XN--MGBCPQ6GPA1A", "XN--MGBERP4A5D4AR", "XN--MGBGU82A",
    "XN--MGBI4ECEXP", "XN--MGBPL2FH", "XN--MGBT3DHD", "XN--MGBTX2B", "XN--MGBX4CD0AB",
    "XN--MIX891F", "XN--MK1BU44C", "XN--MXTQ1M", "XN--NGBC5AZD", "XN--NGBE9E0A", "XN--NGBRX",
    "XN--NODE", "XN--NQV7F", "XN--NQV7FS00EMA", "XN--NYQY26A", "XN--O3CW4H", "XN--OGBPF8FL", "XN--OTU796D",
    "XN--P1ACF", "XN--P1AI", "XN--PGBS0DH", "XN--PSSY2U", "XN--Q7CE6A", "XN--Q9JYB4C",
    "XN--QCKA1PMC", "XN--QXA6A", "XN--QXAM", "XN--RHQV96G", "XN--ROVU88B", "XN--RVC1E0AM3E",
    "XN--S9BRJ9C", "XN--SES554G", "XN--T60B56A", "XN--TCKWE", "XN--TIQ49XQYJ", "XN--UNUP4Y",
    "XN--VERMGENSBERATER-CTB", "XN--VERMGENSBERATUNG-PWB", "XN--VHQUV", "XN--VUQ861B",
    "XN--W4R85EL8FHU5DNRA", "XN--W4RS40L", "XN--WGBH1C", "XN--WGBL6A", "XN--XHQ521B",
    "XN--XKC2AL3HYE2A", "XN--XKC2DL3A5EE0H", "XN--Y9A3AQ", "XN--YFRO4I67O", "XN--YGBI2AMMX",
    "XN--ZFR164B", "XXX", "XYZ", "YACHTS", "YAHOO", "YAMAXUN", "YANDEX", "YE",
    "YODOBASHI", "YOGA", "YOKOHAMA", "YOU", "YOUTUBE", "YT", "YUN", "ZA", "ZAPPOS", "ZARA",
    "ZERO", "ZIP", "ZM", "ZONE", "ZUERICH", "ZW",
};

const uint32_t RegisteredTldNameNb = sizeof(RegisteredTldName) / sizeof(char const*);

static char const * FrequentTldLeak[] = {
    "AAAAAA",
    "AIS",
    "AJ",
    "ALARMSERVER",
    "AN",
    "ASUS",
    "BELKIN",
    "BLINKAP",
    "C3T",
    "COM_",
    "COMHTTP",
    "CORP",
    "COTIA",
    "CPE",
    "DA_FTP_SERVER",
    "DANET",
    "DAVOLINK",
    "DEF",
    "DHCP",
    "DLINK",
    "DLINKROUTER",
    "DNS",
    "DOM",
    "DOMAIN",
    "DS",
    "DSLROUTER",
    "F200",
    "FACEBOOK",
    "FCNAME",
    "FFRGW",
    "GATEWAY",
    "GIF",
    "GOF",
    "GOTHAN",
    "GREATEK",
    "GRP",
    "HARAXIOFFICE",
    "HOME",
    "HOMESTATION",
    "HOTSPOT300",
    "HTM",
    "HTML",
    "HTTP",
    "INTENO",
    "INTERN",
    "INTERNAL",
    "INTRA",
    "INTRANET",
    "INTRAXA",
    "IPTIME",
    "JPG",
    "JS",
    "KORNET",
    "KROSSPRECISION",
    "LAN",
    "LCL",
    "LD",
    "LOC",
    "LOCALDOMAIN",
    "LVMH",
    "MAIL",
    "MAXPRINT",
    "MINIHUB",
    "MP3",
    "MSHOME",
    "MULTILASERAP",
    "MYMAX",
    "NETIS",
    "NONE",
    "NULL",
    "NUPROSM",
    "OIWTECH",
    "OLX",
    "PHP",
    "PIXEL",
    "PDF",
    "PNG",
    "PRI",
    "PRIV",
    "PRIVATE",
    "PVT",
    "RBL",
    "REALTEK",
    "REJECT_RHSBL_CLIENT",
    "ROOT",
    "ROUTER",
    "SERVER",
    "SETUP",
    "SNECMA",
    "SOCGEN",
    "SOPRA",
    "SPEEDPORT_W_724V_09091602_00_006",
    "SYS",
    "TANKS",
    "TLD",
    "TOTOLINK",
    "TP",
    "TVV",
    "UAPROM",
    "UFU",
    "UNICORN",
    "UNIFI",
    "UNIFIQUE",
    "VDS",
    "WAG320N",
    "WEIN",
    "WIRELESSAP",
    "WNET",
    "WORKGROUP",
    "WPAD",
    "WWW",
    "X",
    "XML",
    "XN--3D5443G",
    "YABS",
    "YU",
    "ZYXEL-USG",
    "_MSDCS",
    "_TCP",
    "_UDP"
};

static uint32_t FrequentTldLeakNb = sizeof(FrequentTldLeak) / sizeof(char const *);

int DnsStats::SubmitQuery(uint8_t * packet, uint32_t length, uint32_t start, bool is_response, int * qclass, int * qtype)
{
    int rrclass = 0;
    int rrtype = 0;
    uint32_t name_start = start;

    /* TODO: if we decide to tabulate parameters in queries, will need
     * to check "is_response" value and controlling flag */
    UNREFERENCED_PARAMETER(is_response);

    start = SubmitName(packet, length, start, false);

    if (start + 4 <= length)
    {
        rrtype = (packet[start] << 8) | packet[start + 1];
        rrclass = (packet[start + 2] << 8) | packet[start + 3];
        start += 4;
        *qclass = rrclass;
        *qtype = rrtype;

        SubmitQueryContent(rrtype, rrclass, packet, length, name_start);
    }
    else
    {
        error_flags |= DNS_REGISTRY_ERROR_FORMAT;
        start = length;
    }

    return start;
}

void DnsStats::SubmitQueryContent(int rrtype, int rrclass, 
    uint8_t* packet, uint32_t packet_length, uint32_t name_offset)
{
    if (dnsstat_flags & dnsStateFlagCountQueryParms)
    {
        SubmitRegistryNumber(REGISTRY_DNS_Q_CLASSES, rrclass);
        SubmitRegistryNumber(REGISTRY_DNS_Q_RRType, rrtype);
    }

    if (dnsstat_flags & dnsStateFlagCountUnderlinedNames)
    {
        if (rrtype == DnsRtype_TXT)
        {
            SubmitRegistryString(REGISTRY_DNS_txt_underline, 3, (uint8_t*) "TXT");
            CheckForUnderline(packet, packet_length, name_offset);
        }
    }
}

int DnsStats::SubmitRecord(uint8_t* packet, uint32_t length, uint32_t start,
    uint32_t* e_rcode, uint32_t* e_length, bool is_response)
{
    int rrtype = 0;
    int rrclass = 0;
    unsigned int ttl = 0;
    int ldata = 0;
    int name_start = start;

    record_count++;

    /* Labels are only tabulated in responses, to avoid polluting data with erroneous packets */
    start = SubmitName(packet, length, start, is_response);

    if ((start + 10) > length)
    {
        error_flags |= DNS_REGISTRY_ERROR_FORMAT;
        start = length;
    }
    else
    {
        rrtype = (packet[start] << 8) | packet[start + 1];
        rrclass = (packet[start + 2] << 8) | packet[start + 3];
        ttl = (packet[start + 4] << 24) | (packet[start + 5] << 16)
            | (packet[start + 6] << 8) | packet[start + 7];
        ldata = (packet[start + 8] << 8) | packet[start + 9];

        if (start + ldata + 10 > length)
        {
            error_flags |= DNS_REGISTRY_ERROR_FORMAT;
            start = length;
        }
        else
        {
            SubmitRecordContent(rrtype, rrclass, ttl, ldata, packet + start + 10,
                packet, length, name_start, e_rcode, e_length, is_response);
            start += ldata + 10;
        }
    }

    return start;
}

void DnsStats::SubmitRecordContent(int rrtype, int rrclass, int ttl, int ldata,
    uint8_t * data, uint8_t* packet, uint32_t packet_length, uint32_t name_offset,
    uint32_t* e_rcode, uint32_t* e_length, bool is_response)
{
    if (ldata > 0 || rrtype == DnsRtype_OPT)
    {
        /* only record rrtypes and rrclass if valid response */
        if (rrtype != DnsRtype_OPT)
        {
            if (is_response)
            {
                SubmitRegistryNumber(REGISTRY_DNS_CLASSES, rrclass);
            }
            else if (dnsstat_flags & dnsStateFlagCountQueryParms)
            {
                SubmitRegistryNumber(REGISTRY_DNS_Q_CLASSES, rrclass);
            }
        }
        else
        {
            /* document the extended length */
            if (e_length != NULL)
            {
                *e_length = rrclass;
            }
        }

        if (is_response)
        {
            SubmitRegistryNumber(REGISTRY_DNS_RRType, rrtype);
        }
        else if (dnsstat_flags & dnsStateFlagCountQueryParms)
        {
            SubmitRegistryNumber(REGISTRY_DNS_Q_RRType, rrtype);
        }

        /* For records of type RRSIG, NSEC, NSEC3, DNSKEY, DS,
         * mark the domain as supporting DNSSEC */
        if (
            rrtype == DnsRtype_DNSKEY ||
            rrtype == DnsRtype_RRSIG ||
            rrtype == DnsRtype_NSEC ||
            rrtype == DnsRtype_NSEC3 ||
            rrtype == DnsRtype_DS){
            if (dnssec_packet == NULL) {
                dnssec_name_index = name_offset;
                dnssec_packet = packet;
                dnssec_packet_length = packet_length;
            }
        }

        /* Further parsing for OPT, DNSKEY, RRSIG, DS,
         * and maybe also AFSDB, NSEC3, DHCID, RSYNC types */
        switch (rrtype)
        {
        case (int)DnsRtype_OPT:
            SubmitOPTRecord(ttl, data, ldata, e_rcode);
            break;
        case (int)DnsRtype_DNSKEY:
            SubmitKeyRecord(data, ldata);
            break;
        case (int)DnsRtype_RRSIG:
            SubmitRRSIGRecord(data, ldata);
            break;
        case (int)DnsRtype_DS:
            SubmitDSRecord(data, ldata);
            break;
        case (int)DnsRtype_TLSA:
            SubmitTLSARecord(data, ldata);
            break;
        default:
            break;
        }
    }
}

void DnsStats::SubmitOpcodeAndFlags(uint32_t opcode, uint32_t flags)
{
    SubmitRegistryNumber(REGISTRY_DNS_OpCodes, opcode);

    for (uint32_t i = 0; i < 7; i++)
    {
        if ((flags & (1 << i)) != 0)
        {
            SubmitRegistryNumber(REGISTRY_DNS_Header_Flags, i);
        }
    }

    is_recursive_query = ((flags & (1 << 4)) != 0);
}

int DnsStats::SubmitName(uint8_t * packet, uint32_t length, uint32_t start, bool should_tabulate)
{
    uint32_t l = 0;

    while (start < length)
    {
        l = packet[start];

        if (l == 0)
        {
            /* end of parsing*/
            if (should_tabulate)
            {
                SubmitRegistryNumber(REGISTRY_DNS_LabelType, 0);
            }
            start++;
            break;
        }
        else if ((l & 0xC0) == 0xC0)
        {
            /* Name compression */
            if (should_tabulate)
            {
                SubmitRegistryNumber(REGISTRY_DNS_LabelType, 0xC0);
            }

            if ((start + 2) > length)
            {
                error_flags |= DNS_REGISTRY_ERROR_FORMAT;
                start = length;
                break;
            }
            else
            {
                start += 2;
                break;
            }
        }
        else if (l > 0x3F)
        {
            /* found an extension. Don't know how to parse it! */
            error_flags |= DNS_REGISTRY_ERROR_LABEL;
            if (should_tabulate)
            {
                SubmitRegistryNumber(REGISTRY_DNS_LabelType, l);
            }
            start = length;
            break;
        }
        else
        {
            /* regular name part. To do: tracking of underscore labels. */
            if (should_tabulate)
            {
                SubmitRegistryNumber(REGISTRY_DNS_LabelType, 0);
            }
            if (start + l + 1 > length)
            {
                error_flags |= DNS_REGISTRY_ERROR_FORMAT;
                start = length;
                break;
            }
            else
            {
                start += l + 1;
            }
        }
    }

    return start;
}

void DnsStats::SubmitOPTRecord(uint32_t flags, uint8_t * content, uint32_t length, uint32_t * e_rcode)
{
    uint32_t current_index = 0;

    RegisterEdnsUsage(flags, e_rcode);

    if (current_index < length) {
        edns_options = &content[current_index];
        edns_options_length = length - current_index;
    }
    else {
        edns_options = NULL;
        edns_options_length = 0;
    }

    /* Find the options in the payload */
    while (current_index + 4 <= length)
    {
        uint32_t o_code = (content[current_index] << 8) | content[current_index + 1];
        uint32_t o_length = (content[current_index+2] << 8) | content[current_index + 3];
        current_index += 4 + o_length;
        SubmitRegistryNumber(REGISTRY_EDNS_OPT_CODE, o_code);
    }
}

void DnsStats::RegisterEdnsUsage(uint32_t flags, uint32_t* e_rcode)
{
    /* Little safety because flags are set differently for some packets */
    if (dnssec_packet != NULL) {
        flags |= (1 << 15);
    }
    /* Process the flags and rcodes */
    if (e_rcode != NULL)
    {
        *e_rcode = (flags >> 24) & 0xFF;
    }

    /* Register that EDNS was used */
    is_using_edns = true;

    /* Register whether the DO bit was set */
    is_do_flag_set = (flags & (1 << 15)) != 0;

    /* Add flags to registration */
    for (int i = 0; i < 16; i++)
    {
        if ((flags & (1 << i)) != 0)
        {
            SubmitRegistryNumber(REGISTRY_EDNS_Header_Flags, 15 - i);
        }
    }

    SubmitRegistryNumber(REGISTRY_EDNS_Version_number, (flags >> 16) & 0xFF);
}


void DnsStats::SubmitKeyRecord(uint8_t * content, uint32_t length)
{
    if (length > 8)
    {
        uint32_t algorithm = content[3];
        SubmitRegistryNumber(REGISTRY_DNSSEC_Algorithm_Numbers, algorithm);

        if (algorithm == 2)
        {
            uint32_t prime_length = (content[4] << 8) | content[5];
            if (prime_length < 16)
            {
                SubmitRegistryNumber(REGISTRY_DNSSEC_KEY_Prime_Lengths, prime_length);

                if (prime_length == 1 || prime_length == 2)
                {
                    uint32_t well_known_prime = (content[6] << 8) | content[7];
                    SubmitRegistryNumber(REGISTRY_DNSSEC_KEY_Well_Known_Primes, well_known_prime);
                }
            }
        }
    }
}

void DnsStats::SubmitRRSIGRecord(uint8_t * content, uint32_t length)
{
    if (length > 18)
    {
        uint32_t algorithm = content[2];
        SubmitRegistryNumber(REGISTRY_DNSSEC_Algorithm_Numbers, algorithm);
    }
}

void DnsStats::SubmitDSRecord(uint8_t * content, uint32_t length)
{
    if (length > 4)
    {
        uint32_t algorithm = content[2];
        SubmitRegistryNumber(REGISTRY_DNSSEC_Algorithm_Numbers, algorithm);
    }
}

void DnsStats::SubmitTLSARecord(uint8_t * content, uint32_t length)
{
    if (length >= 3)
    {
        SubmitRegistryNumber(REGISTRY_DANE_CertUsage, content[0]);
        SubmitRegistryNumber(REGISTRY_DANE_TlsaSelector, content[1]);
        SubmitRegistryNumber(REGISTRY_DANE_TlsaMatchingType, content[2]);
    }
}

void DnsStats::SubmitRegistryNumberAndCount(uint32_t registry_id, uint32_t number, uint64_t count)
{
    DnsHashEntry key;
    bool stored = false;

    key.count = count;
    key.registry_id = registry_id;
    key.key_length = sizeof(uint32_t);
    key.key_type = 0; /* number */
    key.key_number = number;

    (void)hashTable.InsertOrAdd(&key, true, &stored);
}

void DnsStats::SubmitRegistryNumber(uint32_t registry_id, uint32_t number)
{
    SubmitRegistryNumberAndCount(registry_id, number, 1);
}

void DnsStats::SubmitRegistryStringAndCount(uint32_t registry_id, uint32_t length, uint8_t * value, uint64_t count)
{
    DnsHashEntry key;
    bool stored = false;

    if (length < 64)
    {
        key.count = count;
        key.registry_id = registry_id;
        key.key_length = length;
        key.key_type = 1; /* string */
        memcpy(key.key_value, value, length);
        key.key_value[length] = 0;

        (void)hashTable.InsertOrAdd(&key, true, &stored);
    }
}

void DnsStats::SubmitRegistryString(uint32_t registry_id, uint32_t length, uint8_t * value)
{
    SubmitRegistryStringAndCount(registry_id, length, value, 1);
}

int DnsStats::CheckForUnderline(uint8_t * packet, uint32_t length, uint32_t start)
{
    uint32_t l = 0;
    uint32_t name_start = start;

    while (start < length)
    {
        l = packet[start];

        if (l == 0)
        {
            /* end of parsing*/
            start++;
            break;
        }
        else if ((l & 0xC0) == 0xC0)
        {
            if ((start + 2) > length)
            {
                /* error */
                start = length;
                break;
            }
            else
            {
                uint32_t new_start = ((l & 63) << 8) + packet[start + 1];

                if (new_start < name_start)
                {
                    (void)CheckForUnderline(packet, length, new_start);
                }

                start += 2;
                break;
            }
        }
        else if (l > 0x3F)
        {
            /* found an extension. Don't know how to parse it! */
            start = length;
            break;
        }
        else
        {
            /* Tracking of underscore labels. */
            if (start + l + 1 > length)
            {
                /* format error */
                start = length;
                break;
            }
            else
            {
                if (l > 0 && packet[start + 1] == '_')
                {
                    uint8_t underlined_name[64];
                    uint32_t flags;

                    (void) NormalizeNamePart(l, &packet[start + 1], underlined_name, sizeof(underlined_name), &flags);

                    if ((flags & 3) == 0)
                    {
                        SubmitRegistryString(REGISTRY_DNS_txt_underline, l, underlined_name);
                    }
                }
                start += l + 1;
            }
        }
    }

    return start;
}

bool DnsStats::GetTLD(uint8_t * packet, uint32_t length, uint32_t start, uint32_t * offset,
    uint32_t * previous_offset,  int * nb_name_parts)
{
    bool ret = true;
    uint32_t l = 0;
    uint32_t previous_o = 0;
    uint32_t previous = 0;
    uint32_t name_start = start;
    int nb_parts = 0;

    while (start < length)
    {
        l = packet[start];

        if (l == 0)
        {
            /* end of parsing*/

            if (nb_parts != 0)
            {
                *offset = previous;
            }
            else
            {
                *offset = start;
            }
            break;
        }
        else if ((l & 0xC0) == 0xC0)
        {
            /* Name compression */
            if ((start + 2) > length)
            {
                ret = false;
                break;
            }
            else
            {
                uint32_t new_start = ((l & 63) << 8) + packet[start + 1];
                
                if (new_start < name_start)
                {
                    int nb_parts_recursive = 0;
                    uint32_t previous_recursive = 0;

                    ret = GetTLD(packet, length, new_start, offset, &previous_recursive, &nb_parts_recursive);
                    if (previous_recursive != 0) {
                        previous_o = previous_recursive;
                    }
                    else {
                        previous_o = previous;
                    }
                    nb_parts += nb_parts_recursive;
                }
                else
                {
                    ret = false;
                }
                break;
            }
        }
        else if (l > 0x3F)
        {
            /* Unexpected name part */
            ret = false;
            break;
        }
        else
        {
            if (start + l + 1 > length)
            {
                /* malformed name part */
                ret = false;
                break;
            }
            else
            {
                nb_parts++;
                previous_o = previous;
                previous = start;
                start += l + 1;
            }
        }
    }

    if (nb_name_parts != NULL) {
        *nb_name_parts = nb_parts;
    }

    if (previous_offset != NULL) {
        *previous_offset = previous_o;
    }

    return ret;
}

int64_t DnsStats::DeltaUsec(long tv_sec, long tv_usec, long tv_sec_start, long tv_usec_start)
{
    int32_t delta_usec = tv_usec - tv_usec_start;
    int32_t delta_sec = tv_sec - tv_sec_start;
    int64_t delta_t = (int64_t)delta_sec * 1000000;
    delta_t += delta_usec;
    return delta_t;
}

char const* DnsStats::LeakTypeName(DnsStatsLeakType leakType)
{
    char const* x = "unknown";

    switch (leakType) {
    case dnsLeakNoLeak:
        x = "tld";
        break;
    case dnsLeakRoot:
        x = "root";
        break;
    case dnsLeakBinary:
        x = "binary";
        break;
    case dnsLeakBadSyntax:
        x = "bad_syntax";
        break;
    case dnsLeakNumeric:
        x = "numeric";
        break;
    case dnsLeakRfc6771:
        x = "rfc6771";
        break;
    case dnsLeakFrequent:
        x = "frequent";
        break;
    case dnsLeakChromiumProbe:
        x = "dga";
        break;
    case dnsLeakJumbo:
        x = "jumbo";
        break;
    case dnsLeakOther:
        x = "other";
        break;
    case dnsLeakChaos:
        x = "chaos";
        break;
    default:
        break;
    }

    return x;
}

bool DnsStats::IsRegisteredTLD(uint8_t* tld, size_t tld_length)
{

    bool isRegistered = false;
    TldAsKey key(tld, tld_length);

    if (registeredTld.GetCount() == 0)
    {
        this->LoadRegisteredTLD_from_memory();
    }

    if (registeredTld.Retrieve(&key) != NULL)
    {
        isRegistered = true;
    }

    return isRegistered;
}

int DnsStats::GetDnsName(uint8_t * packet, uint32_t length, uint32_t start,
    uint8_t * name, size_t name_max, size_t * name_length)
{
    uint32_t l = 0;
    uint32_t name_start = start;
    uint32_t start_next = 0;
    size_t name_index = 0;

    while (start <length && name_index < name_max) {
        l = packet[start];

        if (l == 0)
        {
            /* end of parsing*/
            start++;

            if (start_next == 0) {
                start_next = start;
            }
            break;
        }
        else if ((l & 0xC0) == 0xC0)
        {
            if ((start + 2) > length)
            {
                /* error */
                start_next = length;
                break;
            }
            else
            {
                uint32_t new_start = ((l & 63) << 8) + packet[start + 1];

                if (new_start < name_start)
                {
                    if (start_next == 0) {
                        start_next = start + 2;
                    }
                    start = new_start;
                } else {
                    /* Basic restriction to avoid name decoding loops */
                    name_index = 0;
                    start_next = length;
                    break;
                }
            }
        }
        else if (l > 0x3F)
        {
            /* found an extension. Don't know how to parse it! */
            name_index = 0;
            start_next = length;
            break;
        }
        else
        {
            /* add a label to the name. */
            if (start + l + 1 > length ||
                name_index + l + 2 > name_max)
            {
                /* format error */
                name_index = 0;
                start_next = length;
                break;
            }
            else
            {
                uint32_t flags;
                if (name_index > 0 && name_index+1 < name_max) {
                    name[name_index++] = '.';
                }

                name_index += NormalizeNamePart(l, &packet[start + 1], name + name_index, name_max - name_index, &flags);

                start += l + 1; 
            }
        }
    }

    name[name_index] = 0;

    *name_length = name_index;

    return start_next;
}

uint32_t DnsStats::SkipDnsName(uint8_t* packet, uint32_t length, uint32_t start)
{
    uint32_t l = 0;

    while (start < length) {
        l = packet[start];

        if (l == 0)
        {
            /* end of parsing*/
            start++;
            break;
        }
        else if ((l & 0xC0) == 0xC0)
        {
            if ((start + 2) > length)
            {
                /* error */
                start = length;
                break;
            }
            else
            {
                start += 2;
                break;
            }
        }
        else if (l > 0x3F)
        {
            /* found an extension. Don't know how to parse it! */
            start = length;
            break;
        }
        else
        {
            /* add a label to the name. */
            if (start + l + 1 > length)
            {
                /* format error */
                start = length;
                break;
            }
            else
            {
                start += l + 1;
            }
        }
    }

    return start;
}

uint32_t DnsStats::CountDnsNameParts(uint8_t* packet, uint32_t length, uint32_t start)
{
    uint32_t l = 0;
    uint32_t name_start = start;
    uint32_t nb_parts = 0;

    while (start < length) {
        l = packet[start];

        if (l == 0)
        {
            /* end of parsing*/
            break;
        }
        else if ((l & 0xC0) == 0xC0)
        {
            if ((start + 2) > length)
            {
                /* error */
                break;
            }
            else
            {
                uint32_t new_start = ((l & 63) << 8) + packet[start + 1];

                if (new_start < name_start)
                {
                    start = new_start;
                }
                else {
                    /* Basic restriction to avoid name decoding loops */
                    break;
                }
            }
        }
        else if (l > 0x3F)
        {
            /* found an extension. Don't know how to parse it! */
            break;
        }
        else
        {
            /* add a label to the name. */
            if (start + l + 1 > length)
            {
                /* format error */
                break;
            }
            else
            {
                nb_parts++;
                start += l + 1;
            }
        }
    }


    return nb_parts;
}

int DnsStats::CompareDnsName(const uint8_t * packet, uint32_t length, uint32_t start1, uint32_t start2)
{
    return Compare2DnsNames(packet, length, start1, packet, length, start2);
}

int DnsStats::Compare2DnsNames(const uint8_t* packet1, uint32_t length1, uint32_t start1, const uint8_t* packet2, uint32_t length2, uint32_t start2)

{
    bool ret = false;

    while (start1 < length1 && start2 < length2) {
        if (start1 == start2 && packet1 == packet2) {
            ret = true;
            break;
        } 
        else if ((packet1[start1] & 0xC0) == 0xC0)
        {
            start1 = ((packet1[start1] & 63) << 8) + packet1[start1 + 1];
        }
        else if ((packet2[start2] & 0xC0) == 0xC0)
        {
            start2 = ((packet2[start2] & 63) << 8) + packet2[start2 + 1];
        }
        else if (packet1[start1] > 0x3f || packet2[start2] > 0x3f)
        {
            break;
        }
        else if (packet1[start1] != packet2[start2])
        {
            break;
        }
        else
        {
            uint32_t l = packet1[start1];
            start1++;
            start2++;

            if (l == 0)
            {
                ret = true;
                break;
            }
            else if (start1 + l > length1 || start2 + l >= length2)
            {
                break;
            }
            else
            {
                bool cmp = true;
                for (uint32_t i = 0; cmp && i < l; i++) {
                    uint8_t c1 = packet1[start1 + i];
                    uint8_t c2 = packet1[start2 + i];

                    cmp = (c1 == c2 || (c1 >= 'a' && c1 <= 'z' && (c1 + 'A' - 'a') == c2) || (c1 >= 'A' && c1 <= 'Z' && (c1 + 'a' - 'A') == c2));
                }

                if (!cmp) {
                    break;
                }
                start1 += l;
                start2 += l;
            }
        }
    }

    return ret;
}

bool DnsStats::IsIpv4Name(const uint8_t * name, size_t name_length)
{
    int nb_num_dot = 0;
    size_t i;
    bool is_ipv4 = true;

    i = name_length;
    while (i > 0 && is_ipv4 && nb_num_dot < 4) {
        int mult = 1;
        int r = 0;
        int c = 0;

        while (is_ipv4 && i > 0) {
            i--;
            c = name[i];
            if (c >= '0' && c <= '9') {
                if (mult < 1000) {
                    r += mult * (c - '0');
                    mult *= 10;
                }
                else {
                    is_ipv4 = false;
                }
            }
            else {
                break;
            }
        }

        if (is_ipv4) {
            if ((i == 0 || c == '.') && r < 256) {
                nb_num_dot++;
            }
            else {
                is_ipv4 = false;
            }
        }
    }

    is_ipv4 &= (nb_num_dot == 4);

    return is_ipv4;
}

bool DnsStats::IsIpv4Tld(uint8_t * packet, uint32_t length, uint32_t start)
{
    uint8_t name[1024];
    size_t name_length = 0;

    (void)GetDnsName(packet, length, start, name, sizeof(name), &name_length);

    return IsIpv4Name(name, name_length);
}


/*
* A query is considered minimized if the queried record type is S or A, and
* if the name in the first response or NS record is identical to the name in the query
*/

bool DnsStats::IsQNameMinimized(
    uint32_t nb_queries, int q_rclass, int q_rtype,
    uint8_t * packet1, uint32_t length1, uint32_t qr_name_offset,
    uint8_t* packet2, uint32_t length2, uint32_t rr_name_offset)
{
    bool ret = false;

    if (nb_queries == 1 && q_rclass == DnsRClass_IN &&
        (q_rtype == DnsRtype_A || q_rtype == DnsRtype_NS)) {
        ret = Compare2DnsNames(packet1, length1, qr_name_offset, packet2, length2, rr_name_offset);
    }

    return ret;
}


size_t DnsStats::NormalizeNamePart(uint32_t length, uint8_t * value,
    uint8_t * normalized, size_t normalized_max, uint32_t * flags)
{
    bool has_letter = false;
    bool has_number = false;
    bool has_special = false;
    bool has_dash = false;
    bool has_non_ascii = false;
    size_t normal_length = 0;

    for (uint32_t i = 0; i < length && normal_length + 1 < normalized_max; i++)
    {
        uint8_t c = value[i];
        bool need_escape = false;

        if (c >= 'A' && c <= 'Z')
        {
            has_letter = true;
        }
        else if (c >= 'a' && c <= 'z')
        {
            has_letter = true;
        }
        else if (c >= '0' && c <= '9')
        {
            has_number = true;
        }
        else if (c == '-' || c == '_')
        {
            has_dash = true;
        }
        else if (c == 127 || c < ' ')
        {
            need_escape = true;
            has_special = true;
        }
        else if (c > 127) {
            need_escape = true;
            has_non_ascii = true;
        }
        else if (c == ' ')
        {
            need_escape = (i == 0 || i == (length - 1));
            has_special = true;
        }
        else
        {
            has_special = true;
            need_escape = (c == '.');
        }
        if (need_escape) {
            if (normal_length + 5 < normalized_max) {
                int dec[3];
                dec[0] = c / 100;
                dec[1] = (c % 100) / 10;
                dec[2] = c % 10;

                normalized[normal_length++] = '\\';
                for (int x = 0; x < 3; x++) {
                    normalized[normal_length++] = '0' + dec[x];
                }
            }
            else {
                normalized[normal_length++] = '!';
            }
        }
        else {
            normalized[normal_length++] = c;
        }
    }
    normalized[normal_length] = 0;

    if (flags != NULL)
    {
        *flags = 0;

        if (has_non_ascii)
        {
            *flags += 1;
        }
        else if (has_special)
        {
            *flags += 2;
        }

        if (has_letter)
        {
            *flags += 64;
        }
        if (has_number)
        {
            *flags += 128;
        }
        if (has_dash)
        {
            *flags += 256;
        }
    }

    return(normal_length);
}

void DnsStats::GetSourceAddress(int ip_type, uint8_t * ip_header, uint8_t ** addr, size_t * addr_length)
{
    if (ip_type == 4)
    {
        *addr = ip_header + 12;
        *addr_length = 4;
    }
    else
    {
        *addr = ip_header + 8;
        *addr_length = 16;
    }
}

void DnsStats::GetDestAddress(int ip_type, uint8_t * ip_header, uint8_t ** addr, size_t * addr_length)
{
    if (ip_type == 4)
    {
        *addr = ip_header + 16;
        *addr_length = 4;
    }
    else
    {
        *addr = ip_header + 24;
        *addr_length = 16;
    }
}

bool TldAsKey::CompareTldEntries(TldAsKey * x, TldAsKey * y)
{
    bool ret = x->count >  y->count;

    if (x->count == y->count)
    {
        for (size_t i = 0; i < x->tld_len; i++)
        {
            if (x->tld[i] != y->tld[i])
            {
                ret = x->tld[i] < y->tld[i];
                break;
            }
        }
    }

    return ret;
}


bool DnsStats::IsNumericDomain(uint8_t * tld, uint32_t length)
{
    bool ret = true;

    for (uint32_t i = 0; i < length; i++)
    {
        if (tld[i] < '0' || tld[i] > '9')
        {
            ret = false;
            break;
        }
    }

    return ret;
}

uint64_t DnsStats::GetLeaksRef()
{
    DnsHashEntry key;
    DnsHashEntry * r_key;
    uint64_t leaks_ref = 0;
    const uint32_t rcode[2] = { DNS_RCODE_NXDOMAIN, DNS_RCODE_NOERROR };

    for (int i = 0; i < 2; i++) {
        key.count = 0;
        key.hash = 0;
        key.registry_id = REGISTRY_DNS_root_QR;
        key.key_length = sizeof(uint32_t);
        key.key_type = 0; /* number */
        key.key_number = rcode[i];

        r_key = (DnsHashEntry*)hashTable.Retrieve(&key);
        if (r_key != NULL) {
            leaks_ref += r_key->count;
        }
    }

    return leaks_ref;
}

void DnsStats::ExportDomains(LruHash<TldAsKey> * table, uint32_t registry_id, uint32_t max_leak_count)
{
    TldAsKey *tld_entry;
    std::vector<TldAsKey *> lines(table->GetCount());
    int vector_index = 0;
    uint32_t export_count = 0;

    for (uint32_t i = 0; i < table->GetSize(); i++)
    {
        tld_entry = table->GetEntry(i);

        while (tld_entry != NULL)
        {
            lines[vector_index] = tld_entry;
            vector_index++;
            tld_entry = tld_entry->HashNext;
        }
    }

    std::sort(lines.begin(), lines.end(), TldAsKey::CompareTldEntries);

    /* Retain the N most interesting values */
    for (size_t i = 0; i < lines.size(); i++)
    {
        if (export_count < max_leak_count && lines[i]->tld_len < 64)
        {
            if (registry_id == REGISTRY_DNS_LeakedTLD && 
                IsProbablyChromiumProbe(lines[i]->tld, lines[i]->tld_len, lines[i]->max_name_parts) &&
                lines[i]->count < 3) {
                SubmitRegistryNumberAndCount(REGISTRY_CHROMIUM_PROBES,
                    (uint32_t)lines[i]->tld_len, lines[i]->count);
            }
            else {
                SubmitRegistryStringAndCount(registry_id,
                    (uint32_t)lines[i]->tld_len, lines[i]->tld, lines[i]->count);
                export_count++;
            }
        }
        else if (registry_id == REGISTRY_DNS_LeakedTLD)
        {
            /* Add count of leaks by length -- should replace by pattern match later */
            if (IsProbablyChromiumProbe(lines[i]->tld, lines[i]->tld_len, lines[i]->max_name_parts)) {
                SubmitRegistryNumberAndCount(REGISTRY_CHROMIUM_PROBES,
                    (uint32_t)lines[i]->tld_len, lines[i]->count);
            }
            else {
                SubmitRegistryNumberAndCount(REGISTRY_DNS_LeakByLength,
                    (uint32_t)lines[i]->tld_len, lines[i]->count);
            }
        }
        else if (registry_id == REGISTRY_DNS_LEAK_2NDLEVEL)
        {
            /* Add count to the "others" line */
            SubmitRegistryStringAndCount(REGISTRY_DNS_LEAK_2NDLEVEL, 
                12, (uint8_t *)"-- OTHERS --", lines[i]->count);
        }
        else if (export_count >= max_leak_count)
        {
            break;
        }
    }
}

void DnsStats::ExportLeakedDomains()
{
    ExportDomains(&tldLeakage, REGISTRY_DNS_LeakedTLD, max_tld_leakage_count);
    tldLeakage.Clear();
}

void DnsStats::ExportStringUsage()
{
    ExportDomains(&tldStringUsage, REGISTRY_DNS_Frequent_TLD_Usage, max_tld_string_leakage_count);
    tldStringUsage.Clear();
}

void DnsStats::ExportSecondLeaked()
{
    ExportDomains(&secondLdLeakage, REGISTRY_DNS_LEAK_2NDLEVEL, max_tld_leakage_count);
    secondLdLeakage.Clear();
}

void DnsStats::LoadRegisteredTLD_from_memory()
{
    for (size_t i = 0; i < RegisteredTldNameNb; i++)
    {
        bool stored = false;
        TldAsKey * tak = new
            TldAsKey((uint8_t *) RegisteredTldName[i], strlen(RegisteredTldName[i]));

        registeredTld.InsertOrAdd(tak, false, &stored);

        if (!stored)
        {
            delete tak;
        }
    }
}

bool DnsStats::CheckAddress(uint8_t* addr, size_t len)
{
    bool ret = true;

    if (!allowedAddresses.IsInList(addr, len))
    {
        if (bannedAddresses.IsInList(addr, len))
        {
            ret = false;
        }
        else if (enable_frequent_address_filtering)
        {
            uint32_t count = frequentAddresses.Check(addr, len);

            if (count > frequent_address_max_count)
            {
                /* Add the address to the dropped list */
                char addr_text[64];

                if (AddressFilter::AddressText(addr, len, addr_text, sizeof(addr_text)))
                {
                    SubmitRegistryString(REGISTRY_TOOL_FrequentAddress,
                        (uint32_t) strlen(addr_text), (uint8_t *) addr_text);
                }

                ret = false;
            }
        }
    }
    return ret;
}

static char const * rfc6761_tld[] = {
    "ALT",
    "EXAMPLE",
    "INVALID",
    "LOCAL",
    "LOCALHOST",
    "ONION",
    "TEST"
};

const uint32_t nb_rfc6771_tld = sizeof(rfc6761_tld) / sizeof(char const *);

bool DnsStats::IsValidTldSyntax(uint8_t * tld, size_t length)
{
    bool ret = length > 0;

    for (size_t i = 0; ret && i < length; i++) {
        int x = tld[i];
        ret = (x >= 'A' && x <= 'Z') || (x >= '0' && x <= '9') || x == '-' || (x >= 'a' && x <= 'z');
    }
    return ret;
}

static int CompareToUpperCaseString(const uint8_t * tld, size_t length, const char * target)
{
    int ret = 0;
    size_t j = 0;
    uint8_t * x = (uint8_t *)target;

    for (; j < length; j++)
    {
        if (x[j] == 0)
        {
            ret = 1; /* Target string is longer, thus larger */
            break;
        }
        else if (x[j] != tld[j]) {
            ret = (tld[j] < x[j]) ? -1 : 1;
            break;
        }
    }

    if (ret == 0 && j == length && x[j] != 0)
    {
        ret = -1; /* Target string is shorter, thus lower */
    }

    return ret;
}

bool DnsStats::IsInSortedList(const char ** list, size_t nb_list, uint8_t * tld, size_t length)
{
    bool is_found = false;
    size_t i_low = 0;
    size_t i_high = nb_list - 1;
    int c;
    uint8_t target[64];

    if (length < sizeof(target)) {
        for (size_t i = 0; i < length; i++) {
            c = tld[i];
            if (c >= 'a' && c <= 'z') {
                c += 'A' - 'a';
            }
            target[i] = c;
        }

        c = CompareToUpperCaseString(target, length, list[i_low]);
        if (c == 0) {
            is_found = true;
        }
        else if (c < 0) {
            is_found = false;
        }
        else {
            c = CompareToUpperCaseString(target, length, list[i_high]);
            if (c == 0) {
                is_found = true;
            }
            else if (c > 0) {
                is_found = false;
            }
            else {
                while (i_low + 1 < i_high) {
                    size_t i_mid = (i_low + i_high) / 2;
                    c = CompareToUpperCaseString(target, length, list[i_mid]);
                    if (c == 0) {
                        is_found = true;
                        break;
                    }
                    else if (c < 0) {
                        i_high = i_mid;
                    }
                    else {
                        i_low = i_mid;
                    }
                }
            }
        }
    }
    return is_found;
}

bool DnsStats::IsRfc6761Tld(uint8_t * tld, size_t length)
{
    return IsInSortedList(rfc6761_tld, nb_rfc6771_tld, tld, length);
}


bool DnsStats::IsFrequentLeakTld(uint8_t * tld, size_t length)
{
    return IsInSortedList(FrequentTldLeak, FrequentTldLeakNb, tld, length);
}

/* Try to assess whether a leaked domain looks like the product of DGA
 * In theory, we should be able to check that the distribution of letters 
 * and numbers "looks random", but in practice that's very hard, since
 * actual domain names are often created from acronyms and abbreviations */

bool DnsStats::IsProbablyChromiumProbe(uint8_t * tld, size_t length, uint32_t nb_name_parts)
{
    bool is_dga = (nb_name_parts == 1 && length >= 7 && length <= 15);

    if (is_dga) {
        for (size_t i = 0; i < length; i++) {
            int c = tld[i];
            if ((c < 'a' || c > 'z') && (c < 'A' || c > 'Z')) {
                is_dga = false;
                break;
            }
        }
    }
    return is_dga;
}

void DnsStats::SetToUpperCase(uint8_t * domain, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        int c = domain[i];

        if (c >= 'a' && c <= 'z')
        {
            c += 'A' - 'a';
            domain[i] = (uint8_t)c;
        }
    }
}

void DnsStats::TldCheck(uint8_t * domain, size_t length, bool * is_binary, bool * is_wrong_syntax, bool * is_numeric)
{
    *is_binary = false;
    *is_wrong_syntax = false;
    *is_numeric = (length > 0);

    for (size_t i = 0; i < length; i++)
    {
        int c = domain[i];

        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_')) {
            *is_numeric = false;
        }
        else if (c >= '0' && c <= '9') {
            continue;
        }
        else if (c == '-') {
            *is_numeric &= (i == 0);
        }
        else if (c >= ' ' && c < 127) {
            *is_wrong_syntax = true;
        }
        else {
            *is_binary = true;
        }
    }
}

char const * DnsStats::GetTableName(uint32_t tableId)
{
    char const * ret = NULL;

    if (tableId < RegistryNameByIdNb)
    {
        ret = RegistryNameById[tableId];
    }

    return ret;
}

/*
* Examine the packet level information
*
* - DNS OpCodes
* - DNS RCodes
* - DNS Header Flags
*
* Analyze queries and responses.
* Special cases for TXT, KEY, CSYNC
*

1  1  1  1  1  1
0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                      ID                       |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                    QDCOUNT                    |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                    ANCOUNT                    |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                    NSCOUNT                    |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                    ARCOUNT                    |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*/

bool DnsStats::LoadPcapFiles(size_t nb_files, char const ** fileNames)
{
    bool ret = true;

    for (size_t i = 0; ret && i < nb_files; i++)
    {
        ret = LoadPcapFile(fileNames[i]);
    }

    return ret;
}

bool DnsStats::LoadCborFiles(size_t nb_files, char const** fileNames)
{
    bool ret = true;

    for (size_t i = 0; ret && i < nb_files; i++)
    {
        ret = LoadCborFile(fileNames[i]);
    }

    return ret;
}


bool DnsStats::LoadCborFile(char const* fileName)
{
    cdns cdns_ctx;
    int err;
    bool ret = cdns_ctx.open(fileName);

    while (ret) {
        if (!cdns_ctx.open_block(&err)) {
            ret = (err == CBOR_END_OF_ARRAY);
            break;
        }

        for (size_t i = 0; i < cdns_ctx.block.queries.size(); i++) {
            SubmitCborPacket(&cdns_ctx, i);
        }
    }

    return ret;
}

bool DnsStats::LoadPcapFile(char const * fileName)
{
    bool ret = true;
    pcap_reader reader;
    size_t nb_udp_dns = 0;
    uint64_t data_udp53 = 0;
    uint64_t data_tcp53 = 0;
    uint64_t data_tcp853 = 0;
    uint64_t data_tcp443 = 0;

    if (!reader.Open(fileName, NULL))
    {
        ret = false;
    }
    else
    {
        while (reader.ReadNext())
        {
            if (reader.tp_version == 17 &&
                (reader.tp_port1 == 53 || reader.tp_port2 == 53))
            {
                data_udp53 += (uint64_t)reader.tp_length - 8;

                if (!reader.is_fragment)
                {
                    my_bpftimeval ts;

                    ts.tv_sec = reader.frame_header.ts_sec;
                    ts.tv_usec = reader.frame_header.ts_usec;
                    SubmitPacket(reader.buffer + reader.tp_offset + 8,
                        reader.tp_length - 8, reader.ip_version, reader.buffer + reader.ip_offset, ts);
                    nb_udp_dns++;

                    if (target_number_dns_packets > 0 &&
                        nb_udp_dns >= target_number_dns_packets) {
                        /* Break when enough data captured */
                        break;
                    }
                }
            }
            else if (reader.tp_version == 6) {
                /* Do simple statistics on TCP traffic */
                size_t header_length32 = reader.buffer[reader.tp_offset + 12] >> 4;
                size_t tcp_payload = reader.tp_length - 4 * header_length32;
                bool is_port_853 = false;
                bool is_port_443 = false;

                if (reader.tp_port1 == 53 || reader.tp_port2 == 53) {
                    data_tcp53 += tcp_payload;
                }
                else if (reader.tp_port1 == 853 || reader.tp_port2 == 853) {
                    data_tcp853 += tcp_payload;
                    is_port_853 = true;
                    is_capture_dns_only = false;
                }
                else if (reader.tp_port1 == 443 || reader.tp_port2 == 443) {
                    data_tcp443 += tcp_payload;
                    is_port_443 = true;
                    is_capture_dns_only = false;
                }
                else {
                    is_capture_dns_only = false;
                }
                if (is_port_443 || is_port_853) {
                    uint8_t flags = reader.buffer[reader.tp_offset + 13] & 0x3F;
                    if (flags == 0x02) {
                        uint8_t * addr;
                        size_t addr_length;
                        GetSourceAddress(reader.ip_version, reader.buffer + reader.ip_offset,
                            &addr, &addr_length);
                        RegisterTcpSynByIp(addr, addr_length, is_port_853, is_port_443);
                    }
                }
            }
            else {
                is_capture_dns_only = false;
            }
        }
    }

    if (!is_capture_dns_only && t_start_sec != 0 && t_start_usec != 0) {
        /* Add the traffic load statistics to the summary, but only if the
         * capture was not filtered to only catch DNS data */
        SubmitRegistryNumberAndCount(REGISTRY_VOLUME_PER_PROTO, 0, data_udp53);
        SubmitRegistryNumberAndCount(REGISTRY_VOLUME_PER_PROTO, 53, data_tcp53);
        SubmitRegistryNumberAndCount(REGISTRY_VOLUME_PER_PROTO, 853, data_tcp853);
        SubmitRegistryNumberAndCount(REGISTRY_VOLUME_PER_PROTO, 443, data_tcp443);


        /* Export the duration at the end of the file */
        SubmitRegistryNumberAndCount(REGISTRY_CAPTURE_DURATION, 0, duration_usec);
    }
    /* Account for TCP in the total port 53 traffic */
    SubmitRegistryNumberAndCount(REGISTRY_VOLUME_53ONLY, 0, data_tcp53);

    return ret;
}

void DnsStats::SubmitPacket(uint8_t * packet, uint32_t length, int ip_type, uint8_t* ip_header,
    my_bpftimeval ts)
{
    uint8_t * source_addr;
    size_t source_addr_length;
    uint8_t * dest_addr;
    size_t dest_addr_length;

    GetSourceAddress(ip_type, ip_header, &source_addr, &source_addr_length);
    GetDestAddress(ip_type, ip_header, &dest_addr, &dest_addr_length);

    SubmitPacket(packet, length, source_addr, source_addr_length,
        dest_addr, dest_addr_length, ts);
}

void DnsStats::UpdateDuration(my_bpftimeval ts)
{
    if (t_start_sec == 0 && t_start_usec == 0) {
        t_start_sec = ts.tv_sec;
        t_start_usec = ts.tv_usec;
    }
    else {
        int64_t delta_t = DeltaUsec(ts.tv_sec, ts.tv_usec, t_start_sec, t_start_usec);

        if (delta_t < 0) {
            t_start_sec = ts.tv_sec;
            t_start_usec = ts.tv_usec;
            duration_usec -= delta_t;
        }
        else if (delta_t > duration_usec) {
            duration_usec = delta_t;
        }
    }
}

void DnsStats::SubmitPacket(uint8_t * packet, uint32_t length,
    uint8_t * source_addr, size_t source_addr_length,
    uint8_t * dest_addr, size_t dest_addr_length,
    my_bpftimeval ts)
{
    bool is_response = false;

    bool has_header = true;
    uint32_t flags = 0;
    uint32_t opcode = 0;
    uint32_t rcode = 0;
    uint32_t qdcount = 0;
    uint32_t ancount = 0;
    uint32_t nscount = 0;
    uint32_t arcount = 0;
    uint32_t parse_index = 0;
    bool unfiltered = false;

    UpdateDuration(ts);

    volume_53only += length;

    error_flags = 0;
    is_do_flag_set = false;
    is_using_edns = false;
    edns_options = NULL;
    edns_options_length = 0;
    is_qname_minimized = false;
    dnssec_name_index = 0;
    dnssec_packet = NULL;
    dnssec_packet_length = 0;

    if (rootAddresses.GetCount() == 0)
    {
        rootAddresses.SetList(DefaultRootAddresses, sizeof(DefaultRootAddresses) / sizeof(char const *));
    }

    if (length < 12)
    {
        error_flags |= DNS_REGISTRY_ERROR_FORMAT;
        has_header = false;
    }
    else
    {
        is_response = ((packet[2] & 128) != 0);

        if (is_response)
        {
            unfiltered = CheckAddress(dest_addr, dest_addr_length);
        }
        else
        {
            unfiltered = CheckAddress(source_addr, source_addr_length);
        }
    }

    if (unfiltered)
    {
        if (is_response)
        {
            response_count++;
        }
        else
        {
            query_count++;
        }
        flags = ((packet[2] & 7) << 4) | ((packet[3] & 15) >> 4);
        opcode = (packet[2] >> 3) & 15;
        rcode = (packet[3] & 15);
        qdcount = (packet[4] << 8) | packet[5];
        ancount = (packet[6] << 8) | packet[7];
        nscount = (packet[8] << 8) | packet[9];
        arcount = (packet[10] << 8) | packet[11];

        SubmitOpcodeAndFlags(opcode, flags);

        if (is_response && opcode == DNS_OPCODE_QUERY) {
            /* Find qr_class, so leak analysis can distinguish between Internet and Chaos,
             * when the response code does not indicate an error */
            int qr_class = 0;
            int qr_type = 0;
            if (rcode == DNS_RCODE_NOERROR) {
                uint32_t after_name = SkipDnsName(packet, length, 12);
                if (after_name + 4 <= length) {
                    qr_type = ((packet[after_name]) << 8) + packet[after_name + 1];
                    qr_class = ((packet[after_name + 2]) << 8) + packet[after_name + 3];
                }
            }

            NameLeaksAnalysis(source_addr, source_addr_length, dest_addr, dest_addr_length,
                rcode, qr_class, qr_type, packet, length, 12, ts, ancount > 0 || nscount > 0,
                flags);
        }

        parse_index = 12;

        SubmitPcapRecords(packet, length, parse_index, is_response, has_header, rcode, flags,
            qdcount, ancount, nscount, arcount);

        if (has_header && opcode == DNS_OPCODE_QUERY &&
            rcode == DNS_RCODE_NOERROR && error_flags == 0)
        {
            if (is_response) {
                RegisterStatsByIp(dest_addr, dest_addr_length);

                if (is_do_flag_set) {
                    if (dnssec_packet == NULL) {
                        RegisterDnssecUsageByName(packet, length, 12, false);
                    }
                    else {
                        RegisterDnssecUsageByName(dnssec_packet, dnssec_packet_length, dnssec_name_index, true);
                    }
                }
            }
            else {
                SubmitQueryExtensions(packet, length, 12, source_addr, source_addr_length);
            }
        }
    }

    SubmitRegistryNumber(REGISTRY_DNS_error_flag, error_flags);
}

void DnsStats::SubmitQueryExtensions(
    uint8_t* packet, uint32_t length, uint32_t name_offset,
    uint8_t* client_addr, size_t client_addr_length)
{
    uint32_t tld_offset = 0;
    int nb_name_parts = 0;
    uint32_t previous_offset = 0;
    bool gotTld = GetTLD(packet, length, name_offset, &tld_offset, &previous_offset, &nb_name_parts);

    if (gotTld)
    {
        bool is_binary = false;
        bool is_bad_syntax = false;
        bool is_numeric = false;

        /* Verify that the TLD is valid, so as to exclude random traffic that would drown the stats */
        TldCheck(packet + tld_offset + 1, packet[tld_offset], &is_binary, &is_bad_syntax, &is_numeric);

        if (!is_binary && !is_bad_syntax) {
            SetToUpperCase(packet + tld_offset + 1, packet[tld_offset]);
            TldAsKey key(packet + tld_offset + 1, packet[tld_offset]);

            if (registeredTld.GetCount() == 0)
            {
                LoadRegisteredTLD_from_memory();
            }

            if (registeredTld.Retrieve(&key) != NULL)
            {
                /* This is a registered TLD
                 * Use the list of source addresses as a filter to
                 * check the incoming EDNS options */
                RegisterOptionsByIp(client_addr, client_addr_length);
            }
        }
    }
}

void DnsStats::SubmitCborPacket(cdns* cdns_ctx, size_t packet_id)
{
    bool unfiltered = false;
    cdns_query* query = &cdns_ctx->block.queries[packet_id];
    cdns_query_signature* q_sig = NULL; 
    size_t c_address_id = (size_t)query->client_address_index - cdns_ctx->index_offset;
    bool is_udp;

    if (query->query_signature_index >= cdns_ctx->index_offset) {
        q_sig = &cdns_ctx->block.tables.q_sigs[(size_t)query->query_signature_index - cdns_ctx->index_offset];
    }

    if (t_start_sec == 0 && t_start_usec == 0) {
        t_start_sec = (uint32_t)cdns_ctx->block.preamble.earliest_time_sec;
        t_start_usec = (uint32_t)cdns_ctx->block.preamble.earliest_time_usec;
    }

    volume_53only += query->query_size;
    volume_53only += query->response_size;

    if (rootAddresses.GetCount() == 0)
    {
        rootAddresses.SetList(DefaultRootAddresses, sizeof(DefaultRootAddresses) / sizeof(char const*));
    }

    unfiltered = CheckAddress(cdns_ctx->block.tables.addresses[c_address_id].v,
        cdns_ctx->block.tables.addresses[c_address_id].l);

    if (cdns_ctx->is_old_version()) {
        is_udp = (q_sig == NULL) || (q_sig->qr_transport_flags & 1) == 0;
    }
    else {
        is_udp = (q_sig == NULL) || ((q_sig->qr_transport_flags >> 1)&0xF) == 0;
    }

    if (unfiltered && q_sig != NULL &&
        ( is_udp || (dnsstat_flags& dnsStateFlagIncludeTcpRecords) != 0))
    {
        /* Some QSIG flags bits vary between RFC and draft, but bit 0 and bit 5 have the same meaning. */
        if ((q_sig->qr_sig_flags&0x01) != 0)
        {
            query_count++;

            SubmitCborPacketQuery(cdns_ctx, query, q_sig);
        }

        if ((q_sig->qr_sig_flags & 0x32) != 0)
        {
            response_count++;

            SubmitCborPacketResponse(cdns_ctx, query, q_sig);
        }
    }
}
  
void DnsStats::SubmitCborPacketQuery(cdns* cdns_ctx, cdns_query* query, cdns_query_signature* q_sig)
{
    uint64_t query_time_usec = cdns_ctx->block.block_start_us + query->time_offset_usec;
    my_bpftimeval ts;

    error_flags = 0;
    is_do_flag_set = false;
    is_using_edns = false;
    edns_options = NULL;
    edns_options_length = 0;
    is_qname_minimized = false;
    is_recursive_query = false;
    dnssec_name_index = 0;
    dnssec_packet = NULL;
    dnssec_packet_length = 0;
    error_flags = 0;

    ts.tv_sec = (long)(query_time_usec / 1000000);
    ts.tv_usec = (long)(query_time_usec % 1000000);

    UpdateDuration(ts);

    SubmitOpcodeAndFlags(q_sig->query_opcode, cdns::get_dns_flags(q_sig->qr_dns_flags,false));

    SubmitCborRecords(cdns_ctx, query, q_sig, &query->q_extended, false);

    /* Check that all work is done here.. */
    if (q_sig->query_opcode == DNS_OPCODE_QUERY &&
        q_sig->query_rcode == DNS_RCODE_NOERROR &&
        query->query_name_index >= 0 &&
        error_flags == 0)
    {
        /* This works because parsing of OPT records sets the proper values for OPT fields */
        size_t nid = (size_t)query->query_name_index - cdns_ctx->index_offset;
        size_t addrid = (size_t)query->client_address_index - cdns_ctx->index_offset;
        uint32_t name_len = (uint32_t)cdns_ctx->block.tables.name_rdata[nid].l;
        uint8_t* name = cdns_ctx->block.tables.name_rdata[nid].v;
        uint8_t null_name[] = { 0,0 };
        if (name_len == 0) {
            name = null_name;
            name_len = 2;
        }

        SubmitQueryExtensions(name, name_len, 0,
            cdns_ctx->block.tables.addresses[addrid].v,
            cdns_ctx->block.tables.addresses[addrid].l);
    }

    SubmitRegistryNumber(REGISTRY_DNS_error_flag, error_flags);
}

void DnsStats::SubmitCborPacketResponse(cdns* cdns_ctx, cdns_query* query, cdns_query_signature* r_sig)
{
    uint32_t rcode = r_sig->response_rcode;
    size_t s_addrid = (size_t)r_sig->server_address_index - cdns_ctx->index_offset;
    size_t c_addrid = (size_t)query->client_address_index - cdns_ctx->index_offset;
    uint32_t server_addr_length = (uint32_t)cdns_ctx->block.tables.addresses[s_addrid].l;
    uint8_t* server_addr = cdns_ctx->block.tables.addresses[s_addrid].v;
    uint32_t client_addr_length = (uint32_t)cdns_ctx->block.tables.addresses[c_addrid].l;
    uint8_t* client_addr = cdns_ctx->block.tables.addresses[c_addrid].v;
    uint8_t null_name[] = { 0, 0 };
    my_bpftimeval ts;
    uint64_t query_time_usec = cdns_ctx->block.block_start_us + query->time_offset_usec;
    uint64_t r_delay = query_time_usec + query->delay_useconds;

    ts.tv_sec = (long)(r_delay / 1000000);
    ts.tv_usec = (long)(r_delay % 1000000);

    UpdateDuration(ts);

    error_flags = 0;
    is_do_flag_set = false;
    is_using_edns = false;
    edns_options = NULL;
    edns_options_length = 0;
    is_qname_minimized = false;
    dnssec_name_index = 0;
    dnssec_packet = NULL;
    dnssec_packet_length = 0;
    error_flags = 0;

    SubmitOpcodeAndFlags(r_sig->query_opcode, cdns::get_dns_flags(r_sig->qr_dns_flags, true));

    if (query->query_name_index >= cdns_ctx->index_offset) {
        size_t nid = (size_t)query->query_name_index - cdns_ctx->index_offset;
        uint8_t* q_name = cdns_ctx->block.tables.name_rdata[nid].v;
        uint32_t q_name_length = (uint32_t)cdns_ctx->block.tables.name_rdata[nid].l;

        if (q_name_length == 0) {
            q_name = null_name;
            q_name_length = 0;
        }

        if (r_sig->query_opcode == DNS_OPCODE_QUERY)
        {
            /* Find qr_class, so leak analysis can distinguish between Internet and Chaos */
            int qr_class = 0;
            int qr_type = 0;
            if (r_sig != NULL) {
                if (r_sig->query_classtype_index >= cdns_ctx->index_offset) {
                    size_t cid = (size_t)r_sig->query_classtype_index - cdns_ctx->index_offset;
                    qr_class = cdns_ctx->block.tables.class_ids[cid].rr_class;
                    qr_type = cdns_ctx->block.tables.class_ids[cid].rr_type;
                }
            }

            NameLeaksAnalysis(server_addr, server_addr_length, client_addr, client_addr_length,
                r_sig->response_rcode, qr_class, qr_type, q_name, q_name_length, 0, ts, true,
                cdns::get_dns_flags(r_sig->qr_dns_flags, true));
        }

        SubmitCborRecords(cdns_ctx, query, r_sig, &query->r_extended, true);

        if (r_sig->query_opcode == DNS_OPCODE_QUERY &&
            rcode == DNS_RCODE_NOERROR && error_flags == 0)
        {
            RegisterStatsByIp(client_addr, client_addr_length);

            if (is_do_flag_set) {
                if (dnssec_packet == NULL) {
                    RegisterDnssecUsageByName(q_name, q_name_length, 0, false);
                }
                else {
                    /* TODO: verify that this the proper name */
                    RegisterDnssecUsageByName(dnssec_packet, dnssec_packet_length, dnssec_name_index, true);
                }
            }
        }
    }
    else {
        /* Response code 1, malformed packet, happens here. */
        if (rootAddresses.IsInList(server_addr, server_addr_length))
        {
            /* Perform statistics on root traffic */
            SubmitRegistryNumber(REGISTRY_DNS_root_QR, rcode);
        }
        SubmitRegistryNumber(REGISTRY_DNS_RCODES, rcode);
    }

    SubmitRegistryNumber(REGISTRY_DNS_error_flag, error_flags);
}

void DnsStats::SubmitCborRecords(cdns* cdns_ctx, cdns_query* query, cdns_query_signature* q_sig,
    cdns_qr_extended * ext, bool is_response)
{
    uint32_t rcode = (is_response) ? q_sig->response_rcode : q_sig->query_rcode;
    uint32_t e_rcode = 0;
    uint32_t e_length = 512;
    int first_rname_index = -1;


    if (ext->is_filled) {
        int x_i[4] = { ext->question_index, ext->answer_index, ext->authority_index, ext->additional_index };

        if (x_i[0] >= 0) {
            /* assume just one query per q_sig, but sometimes there is none. */
            size_t cid = (size_t)q_sig->query_classtype_index - cdns_ctx->index_offset;
            size_t nid = (size_t)query->query_name_index - cdns_ctx->index_offset;
            SubmitQueryContent(cdns_ctx->block.tables.class_ids[cid].rr_type,
                cdns_ctx->block.tables.class_ids[cid].rr_class,
                cdns_ctx->block.tables.name_rdata[nid].v,
                (uint32_t)cdns_ctx->block.tables.name_rdata[nid].l, 0);
        }

        for (int i = 1; i < 4; i++) {
            if (x_i[i] >= 0) {
                cdns_rr_list* list = &cdns_ctx->block.tables.rr_list[(size_t)x_i[i]- cdns_ctx->index_offset];

                for (size_t j = 0; j < list->rr_index.size(); j++) {
                    cdns_rr_field * rr = &cdns_ctx->block.tables.rrs[(size_t)list->rr_index[j]- cdns_ctx->index_offset];
                    size_t cid = (size_t)rr->classtype_index - cdns_ctx->index_offset;
                    size_t rrid = (size_t) rr->rdata_index - cdns_ctx->index_offset;
                    size_t nid = (size_t)rr->name_index - cdns_ctx->index_offset;
                    int rr_class = cdns_ctx->block.tables.class_ids[cid].rr_class;
                    int ttl = rr->ttl;

                    if (cdns_ctx->block.tables.class_ids[cid].rr_type == DnsRtype_OPT) {
                        rr_class = q_sig->udp_buf_size;
                        ttl = cdns::get_edns_flags(q_sig->qr_dns_flags);

                        if (dnssec_packet != NULL) {
                            ttl |= (1 << 15);
                        }
                    }

                    SubmitRecordContent(
                        cdns_ctx->block.tables.class_ids[cid].rr_type, rr_class, ttl,
                        (uint32_t)cdns_ctx->block.tables.name_rdata[rrid].l,
                        cdns_ctx->block.tables.name_rdata[rrid].v,
                        cdns_ctx->block.tables.name_rdata[nid].v,
                        (uint32_t)cdns_ctx->block.tables.name_rdata[nid].l,
                        0, (i == 3) ? &e_rcode : NULL, (i == 3) ? &e_length : NULL, is_response);
                    if (first_rname_index < 0 && i < 3) {
                        first_rname_index = rr->name_index;
                    }
                }
            }
        }
    }

    if (!is_response) {
        int edns_flags = cdns::get_edns_flags(q_sig->qr_dns_flags);
        if (q_sig->opt_rdata_index >= 0) {
            size_t opt_rrid = (size_t)q_sig->opt_rdata_index - cdns_ctx->index_offset;

            SubmitRecordContent(DnsRtype_OPT, q_sig->udp_buf_size, edns_flags,
                (uint32_t)cdns_ctx->block.tables.name_rdata[opt_rrid].l,
                cdns_ctx->block.tables.name_rdata[opt_rrid].v,
                NULL, 0, 0, &e_rcode, &e_length, is_response);
        }
    }

    
    rcode |= (e_rcode << 4);
    SubmitRegistryNumber(REGISTRY_DNS_RCODES, rcode);

    if ((dnsstat_flags & dnsStateFlagCountPacketSizes) != 0)
    {
        if (is_response)
        {
            SubmitRegistryNumber(REGISTRY_DNS_Response_Size, query->response_size);
            if ((cdns::get_dns_flags(q_sig->qr_dns_flags,true) & (1 << 5)) != 0)
            {
                SubmitRegistryNumber(REGISTRY_DNS_TC_length, e_length);
            }
        }
        else
        {
            SubmitRegistryNumber(REGISTRY_DNS_Query_Size, query->query_size);
            SubmitRegistryNumber(REGISTRY_EDNS_Packet_Size, e_length);
        }
    }

    if (is_response) {
        size_t nid = (size_t)query->query_name_index - cdns_ctx->index_offset;
        if (first_rname_index >= 0) {
            if (cdns_ctx->block.tables.name_rdata[nid].l == 0) {
                is_qname_minimized = true;
            }
            else {
                size_t cid = (size_t)q_sig->query_classtype_index - cdns_ctx->index_offset;
                size_t rname_id = (size_t)first_rname_index - cdns_ctx->index_offset;

                is_qname_minimized = IsQNameMinimized(
                    1,
                    cdns_ctx->block.tables.class_ids[cid].rr_class,
                    cdns_ctx->block.tables.class_ids[cid].rr_type,
                    cdns_ctx->block.tables.name_rdata[nid].v,
                    (uint32_t)cdns_ctx->block.tables.name_rdata[nid].l, 0,
                    cdns_ctx->block.tables.name_rdata[rname_id].v,
                    (uint32_t)cdns_ctx->block.tables.name_rdata[rname_id].l, 0);
            }
        }
        else {
            /* In the absence of further knowledge, assume that this is true. */
            is_qname_minimized = true;
        }
    }
}

void DnsStats::SubmitPcapRecords(uint8_t * packet, uint32_t length, uint32_t parse_index,
    bool is_response, bool has_header, uint32_t rcode, uint32_t flags,
    uint32_t qdcount, uint32_t ancount, uint32_t nscount, uint32_t arcount)
{
    int query_rclass = 0;
    int query_rtype = 0;
    uint32_t e_rcode = 0;
    uint32_t e_length = 512;
    uint32_t first_query_index = 0;
    uint32_t first_answer_index = 0;
    uint32_t first_ns_index = 0;

    first_query_index = parse_index;

    for (uint32_t i = 0; i < qdcount; i++)
    {
        if (parse_index >= length)
        {
            error_flags |= DNS_REGISTRY_ERROR_FORMAT;
        }
        else
        {
            parse_index = SubmitQuery(packet, length, parse_index, is_response, &query_rclass, &query_rtype);
        }
    }

    first_answer_index = parse_index;

    for (uint32_t i = 0; i < ancount; i++)
    {
        if (parse_index >= length)
        {
            error_flags |= DNS_REGISTRY_ERROR_FORMAT;
        }
        else
        {
            parse_index = SubmitRecord(packet, length, parse_index, NULL, NULL, is_response);
        }
    }

    first_ns_index = parse_index;

    for (uint32_t i = 0; i < nscount; i++)
    {
        if (parse_index >= length)
        {
            error_flags |= DNS_REGISTRY_ERROR_FORMAT;
        }
        else
        {
            parse_index = SubmitRecord(packet, length, parse_index, NULL, NULL, is_response);
        }
    }

    for (uint32_t i = 0; i < arcount; i++)
    {
        if (parse_index >= length)
        {
            error_flags |= DNS_REGISTRY_ERROR_FORMAT;
        }
        else
        {
            parse_index = SubmitRecord(packet, length, parse_index, &e_rcode, &e_length, is_response);
        }
    }

    if (has_header)
    {
        rcode |= (e_rcode << 4);
        SubmitRegistryNumber(REGISTRY_DNS_RCODES, rcode);
    }

    if (has_header && (dnsstat_flags & dnsStateFlagCountPacketSizes) != 0)
    {
        if (is_response)
        {
            SubmitRegistryNumber(REGISTRY_DNS_Response_Size, length);
            if ((flags & (1 << 5)) != 0)
            {
                SubmitRegistryNumber(REGISTRY_DNS_TC_length, e_length);
            }
        }
        else
        {
            SubmitRegistryNumber(REGISTRY_DNS_Query_Size, length);
            SubmitRegistryNumber(REGISTRY_EDNS_Packet_Size, e_length);
        }
    }

    if (has_header && is_response) {
        if (ancount > 0 || nscount > 0) {
            uint32_t second_name_index = (ancount == 0) ? first_ns_index : first_answer_index;
            is_qname_minimized = IsQNameMinimized(
                qdcount, query_rclass, query_rtype,
                packet, length, first_query_index,
                packet, length, second_name_index);
        }
        else {
            /* In the absence of further knowledge, this may be true... */
            is_qname_minimized = true;
        }
    }
}

void DnsStats::NameLeaksAnalysis(
    uint8_t * server_addr, 
    size_t server_addr_length,
    uint8_t* client_addr,
    size_t client_addr_length,
    int rcode,
    int qr_class,
    int qr_type,
    uint8_t * packet,
    uint32_t packet_length,
    uint32_t name_offset,
    my_bpftimeval ts,
    bool is_not_empty_response,
    int flags
    )
{

    uint32_t tld_offset = 0;
    int nb_name_parts = 0;
    uint32_t previous_offset = 0;
    bool gotTld = GetTLD(packet, packet_length, name_offset, &tld_offset, &previous_offset, &nb_name_parts);
    uint8_t* tld = packet + tld_offset + 1;
    uint32_t tld_length = *(packet + tld_offset);
    bool is_binary = false;
    bool is_bad_syntax = false;
    bool is_numeric = false;

    if (gotTld) {
        DnsStats::SetToUpperCase(tld, tld_length);
        /* Verify that the TLD is valid, so as to exclude random traffic that would drown the stats */
        TldCheck(tld, tld_length, &is_binary, &is_bad_syntax, &is_numeric);
    }

    if (rootAddresses.IsInList(server_addr, server_addr_length))
    {
        uint32_t nb_name_parts = CountDnsNameParts(packet, packet_length, name_offset);
        /* Perform statistics on root traffic */
        SubmitRegistryNumber(REGISTRY_DNS_root_QR, rcode);

        if (gotTld)
        {
            /* Debug option, classify the TLD */
            DnsStatsLeakType x_type = dnsLeakNoLeak;
            int is_nx = -1;
            
            if (rcode == DNS_RCODE_NXDOMAIN && tld_length == 4 && memcmp(tld, "ARPA", 4) == 0)
            {
                /* very special case: some root servers are also authoritative
                * servers for .arpa. If they receive a query for no-such.arpa,
                * they will return an NX_DOMAIN error, but these errors should not
                * result in increasing the tally of leaked domains.
                */
                rcode = DNS_RCODE_NOERROR;
            }

            if (rcode == DNS_RCODE_NXDOMAIN)
            {
                is_nx = 1;
                /* Analysis of domain leakage */
                if (is_binary) {
                    SubmitRegistryNumber(REGISTRY_DNS_LEAK_BINARY, 0);
                    x_type = dnsLeakBinary;
                }
                else if (is_bad_syntax) {
                    SubmitRegistryNumber(REGISTRY_DNS_LEAK_SYNTAX, 0);
#ifdef PRIVACY_CONSCIOUS
                    x_type = dnsLeakBadSyntax;
#endif
                }
                else if (is_numeric) {
#ifdef PRIVACY_CONSCIOUS
                    x_type = dnsLeakNumeric;
#endif
                    if (IsIpv4Tld(packet, packet_length, name_offset)) {
                        SubmitRegistryNumber(REGISTRY_DNS_LEAK_IPV4, 0);
                    }
                    else {
                        SubmitRegistryNumber(REGISTRY_DNS_LEAK_NUMERIC, 0);
                    }
                }
                else if (IsRfc6761Tld(tld, tld_length))
                {
                    SubmitRegistryString(REGISTRY_DNS_RFC6761TLD, tld_length, tld);
#ifdef PRIVACY_CONSCIOUS
                    x_type = dnsLeakRfc6771;
#endif
                }
                else
                {
                    /* Insert in leakage table */
                    TldAsKey key(tld, tld_length, nb_name_parts);
                    bool stored = false;
                    (void)tldLeakage.InsertOrAdd(&key, true, &stored);
                    if (IsFrequentLeakTld(tld, tld_length)) {
                        x_type = dnsLeakFrequent;
                    }
                    else if (IsProbablyChromiumProbe(tld, tld_length, nb_name_parts)) {
                        x_type = dnsLeakChromiumProbe;
                    }
                    else if (tld_length >= 16) {
                        x_type = dnsLeakJumbo;
                    }
                    else {
                        x_type = dnsLeakOther;
                    }

                    /* If full enough, remove the LRU, and account for it in the patterns catalog */
                    if (tldLeakage.GetCount() > max_tld_leakage_table_count)
                    {
                        TldAsKey* removed = tldLeakage.RemoveLRU();
                        if (removed != NULL)
                        {
                            /* Add count of leaks by length -- should replace by pattern match later */
                            /* TODO: accumulate as Chromium Probe if profile matches. */
                            if (IsProbablyChromiumProbe(removed->tld, removed->tld_len, removed->max_name_parts)) {
                                SubmitRegistryNumberAndCount(REGISTRY_CHROMIUM_PROBES,
                                    (uint32_t)removed->tld_len, removed->count);
                            }
                            else {
                                SubmitRegistryNumberAndCount(REGISTRY_DNS_LeakByLength,
                                    (uint32_t)removed->tld_len, removed->count);
                            }

                            delete removed;
                        }
                    }

                    /* Insert the 2nd level name part */
                    uint8_t* key2_name;
                    uint8_t key2_length;
                    uint8_t should_keep = false;

                    if (nb_name_parts <= 1) {
                        key2_name = (uint8_t*)"-- NONE --";
                        key2_length = 10;
                        should_keep = true;
                    }
                    else {
                        key2_name = packet + previous_offset + 1;
                        key2_length = packet[previous_offset];
                        if (IsNumericDomain(key2_name, key2_length)) {
                            key2_name = (uint8_t*)"-- NUMBER --";
                            key2_length = 12;
                            should_keep = true;
                        }
                    }
                    TldAsKey key2(key2_name, key2_length);
                    (void)secondLdLeakage.InsertOrAdd(&key2, true, &stored);

                    /* TODO: If full enough, remove the LRU, and account for it in the -- OTHERS -- entry */
                    if (!should_keep &&
                        secondLdLeakage.GetCount() > max_tld_leakage_table_count)
                    {
                        TldAsKey* removed = secondLdLeakage.RemoveLRU();
                        if (removed != NULL)
                        {
                            TldAsKey key3((uint8_t*)"-- OTHERS --", 12);
                            key3.count = removed->count;
                            (void)secondLdLeakage.InsertOrAdd(&key3, true, &stored);
                            delete removed;
                        }
                    }
                }
            }
            else if (rcode == DNS_RCODE_NOERROR && is_not_empty_response)
            {
                is_nx = 0;
                if (tld_length == 0) {
                    x_type = dnsLeakRoot;
                }
                else if (qr_class == DNS_CLASS_CHAOS) {
                    x_type = dnsLeakChaos;
                }

                /* Analysis of traffic per TLD */
                if (dnsstat_flags & dnsStateFlagCountTld)
                {
                    SubmitRegistryString(REGISTRY_TLD_response, tld_length, tld);
                }
            }

            if (is_nx >= 0) {
                if (is_nx == 0 || capture_cache_ratio_nx_domain || address_report != NULL) {
                    /* Analysis of useless traffic to the root */
                    TldAddressAsKey key(client_addr, client_addr_length, tld, tld_length, ts, is_nx, x_type, flags);
                    TldAddressAsKey* present = queryUsage.Retrieve(&key);

                    if (present != NULL) {
                        /* keep statistics about this address */
                        present->count++;
                        /* Accumulate the DNS flags observed for this address and TLD */
                        present->flags |= flags;
                        /* Compute the delay between this and the previous view, and update */
                        int64_t delay = DeltaUsec(ts.tv_sec, ts.tv_usec, present->ts.tv_sec, present->ts.tv_usec);

                        if (present->tld_min_delay < 0 || present->tld_min_delay > delay) {
                            present->tld_min_delay = delay;
                        }
                        present->ts.tv_sec = ts.tv_sec;
                        present->ts.tv_usec = ts.tv_usec;
                    }
                    else if (queryUsage.GetCount() < max_query_usage_count) {
                        /* If table is full, stick with just the transactions that are present */
                        bool stored = false;
                        (void)queryUsage.InsertOrAdd(&key, true, &stored);
                    }
                }

                SubmitRegistryNumber(REGISTRY_DNS_NAME_PARTS_COUNT, CountDnsNameParts(packet, packet_length, name_offset));

#ifdef PRIVACY_CONSCIOUS
                /* Debug option, list all the names found in queries to the root */
                if (name_report != NULL) {
                    uint8_t name[1024];
                    size_t name_len = 0;

                    (void)GetDnsName(packet, packet_length, name_offset, name, sizeof(name), &name_len);

                    if (name_len > 0) {
                        /* Insert in leakage table */
                        DnsNameEntry key;
                        bool stored = false;

                        DnsStats::SetToUpperCase(name, name_len);
                        key.name_len = name_len;
                        key.name = name;
                        key.is_nx = is_nx;
                        key.leakType = x_type;
                        key.count = 1;
                        if (client_addr_length == 4 || client_addr_length == 16) {
                            memcpy(key.addr, client_addr, client_addr_length);
                            key.addr_len = client_addr_length;
                        }
                        else {
                            key.addr_len = 0;
                        }
                        key.rr_type = qr_type;
                        key.flags = flags;

                        (void)nameList.InsertOrAdd(&key, true, &stored);
                        key.name = NULL;
                        key.name_len = 0;
                    }
                }
#endif
            }
        }
    }
    else if (gotTld)
    {
        /* Perform statistics on user traffic */
        TldAsKey key(tld, tld_length);

        /* Check whether this TLD is in the registered list */
        if (registeredTld.GetCount() == 0)
        {
            LoadRegisteredTLD_from_memory();
        }

        if (tld_length == 0 || registeredTld.Retrieve(&key) != NULL)
        {
            /* This is a registered TLD, or the root */
            SubmitRegistryNumber(REGISTRY_DNS_TLD_Usage_Count, 1);
            if ((dnsstat_flags & dnsStateFlagListTldUsed) != 0)
            {
                SubmitRegistryString(REGISTRY_DNS_Tld_Usage, tld_length, tld);
            }
        }
        else
        {
            /* Keep a count */
            SubmitRegistryNumber(REGISTRY_DNS_TLD_Usage_Count, 0);

            /* Analysis of domain leakage */
            if (IsRfc6761Tld(tld, tld_length))
            {
                SubmitRegistryString(REGISTRY_DNS_RFC6761_Usage, tld_length, tld);
            }
            else
            {
                bool stored = false;

                if (tldStringUsage.GetCount() >= max_tld_string_usage_count)
                {
                    TldAsKey* removed = tldStringUsage.RemoveLRU();
                    if (removed != NULL)
                    {
                        delete removed;
                    }
                }

                if (is_binary) {
                    TldAsKey key2((uint8_t*)"-- BINARY --", 12);

                    tldStringUsage.InsertOrAdd(&key2, true, &stored);
                }
                else if (is_bad_syntax) {
                    TldAsKey key2((uint8_t*)"-- SYNTAX --", 12);

                    tldStringUsage.InsertOrAdd(&key2, true, &stored);
                }
                else if (is_numeric) {
                    if (IsIpv4Tld(packet, packet_length, name_offset)) {
                        TldAsKey key2((uint8_t*)"-- IPV4 --", 10);

                        tldStringUsage.InsertOrAdd(&key2, true, &stored);
                    }
                    else {
                        TldAsKey key2((uint8_t*)"-- NUMBER --", 12);

                        tldStringUsage.InsertOrAdd(&key2, true, &stored);
                    }
                }
                else {
                    tldStringUsage.InsertOrAdd(&key, true, &stored);
                }
            }
        }
    }
}

void DnsStats::ExportQueryUsage()
{
    /* Tabulate the entire set of query usage responses, and compute the minimum cache per address */

    TldAddressAsKey *tld_address_entry;
    std::vector<TldAddressAsKey *> lines(queryUsage.GetCount());
    int vector_index = 0;
    uint64_t total_no_error_queries = 0;
    uint64_t total_no_error_entries = 0;
    uint64_t total_error_queries = 0;
    uint64_t total_error_entries = 0;
    const uint32_t cache_bucket[9] = { 1, 10, 30, 60, 120, 180, 240, 300, 600 };
    uint64_t ip_per_bucket[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint64_t ip_per_bucket_d[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint64_t total_per_bucket[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    FILE* F = NULL;
    
    if (address_report) {
#ifdef PRIVACY_CONSCIOUS
        if (compress_name_and_address_reports) {
            int err = 0;
            F = ithi_gzip_compress_open(address_report, &err);
            if (F == NULL || err != 0) {
                fprintf(stderr, "Cannot open file <%s> for compression, err = %d\n", address_report, err);
            }
        }
        else {
            F = ithi_file_open(address_report, "w");
            if (F == NULL) {
                fprintf(stderr, "Cannot open <%s> for writing\n", address_report);
            }
        }
#else
        F = ithi_file_open(address_report, "w");
        if (F == NULL) {
            fprintf(stderr, "Cannot open <%s> for writing\n", address_report);
        }
#endif

        if (F != NULL) {
            fprintf(F, "Address, TLD, nx_domain, name_type, min_delay, count, flags\n");
        }
    }

    for (uint32_t i = 0; i < queryUsage.GetSize(); i++)
    {
        tld_address_entry = queryUsage.GetEntry(i);

        while (tld_address_entry != NULL)
        {
            lines[vector_index] = tld_address_entry;
            vector_index++;
            tld_address_entry = tld_address_entry->HashNext;
        }
    }

    std::sort(lines.begin(), lines.end(), TldAddressAsKey::CompareByAddressAndTld);

    /* Tabulate by address */
    int64_t min_tld_delay = -1;
    uint64_t count_per_ip = 0;
    uint64_t tld_average_delay;
    uint64_t tld_sum_delay = 0;
    uint64_t tld_nb_delay = 0;

    for (size_t i = 0; i < lines.size(); i++) {
#ifdef PRIVACY_CONSCIOUS
        /* Optional detailed data */
        if (F != NULL) {
            char safe_tld[512];
           (void)ithi_copy_to_safe_text(safe_tld, sizeof(safe_tld), lines[i]->tld, lines[i]->tld_len);


            if (lines[i]->addr_len == 4) {
                fprintf(F, "%d.%d.%d.%d,\"%s\",%d,%s,%lld,%llu,%d\n",
                    lines[i]->addr[0], lines[i]->addr[1], lines[i]->addr[2], lines[i]->addr[3],
                    safe_tld, lines[i]->is_nx, LeakTypeName(lines[i]->leakType), 
                    (long long)lines[i]->tld_min_delay, (unsigned long long)lines[i]->count,
                    lines[i]->flags);
            }
            else if (lines[i]->addr_len == 16) {
                fprintf(F,
                    "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x,\"%s\",%d,%s,%lld,%llu,%d\n",
                    lines[i]->addr[0], lines[i]->addr[1], lines[i]->addr[2], lines[i]->addr[3],
                    lines[i]->addr[4], lines[i]->addr[5], lines[i]->addr[6], lines[i]->addr[7],
                    lines[i]->addr[8], lines[i]->addr[9], lines[i]->addr[10], lines[i]->addr[11],
                    lines[i]->addr[12], lines[i]->addr[13], lines[i]->addr[14], lines[i]->addr[15],
                    safe_tld, lines[i]->is_nx, LeakTypeName(lines[i]->leakType), (long long)lines[i]->tld_min_delay, 
                    (unsigned long long)lines[i]->count, lines[i]->flags);
            }
        }
#endif
        if (lines[i]->is_nx == 0) {
            total_no_error_queries += lines[i]->count;
            total_no_error_entries++;

            if (min_tld_delay < 0 ||
                (lines[i]->tld_min_delay > 0 && lines[i]->tld_min_delay < min_tld_delay)) {
                min_tld_delay  = lines[i]->tld_min_delay;
            }
            count_per_ip += lines[i]->count;
            if (lines[i]->count > 1) {
                int64_t duration = DeltaUsec(lines[i]->ts.tv_sec, lines[i]->ts.tv_usec, lines[i]->ts_init.tv_sec, lines[i]->ts_init.tv_usec);
                tld_sum_delay += duration;
                tld_nb_delay += lines[i]->count - 1;
            }
        }
        else {
            total_error_queries += lines[i]->count;
            total_error_entries++;
        }

        if (i + 1 >= lines.size() ||
            lines[i]->addr_len != lines[i + 1]->addr_len ||
            memcmp(lines[i]->addr, lines[i + 1]->addr, lines[i]->addr_len) != 0) {
            if (count_per_ip > 0) {
                /* Finished analyzing this IP address */
                int i_bucket = 8;
                int i_bucket_d = 8;

                if (tld_nb_delay > 0) {
                    tld_average_delay = tld_sum_delay / tld_nb_delay;
                }
                else {
                    tld_average_delay = 600000000;
                }

                for (i_bucket_d = 0; i_bucket_d < 8; i_bucket_d++) {
                    if ((uint64_t)cache_bucket[i_bucket_d] * 1000000 > tld_average_delay) {
                        break;
                    }
                }

                if (min_tld_delay > 0) {
                    for (i_bucket = 0; i_bucket < 8; i_bucket++) {
                        if ((uint64_t)cache_bucket[i_bucket] * 1000000 > (uint64_t)min_tld_delay) {
                            break;
                        }
                    }
                }
                else if (min_tld_delay < 0) {
#ifdef PRIVACY_CONSCIOUS
                    min_tld_delay = 600000000;
#endif
                }
                else
                {
                    i_bucket = 0;
                }
                ip_per_bucket[i_bucket] += 1;
                ip_per_bucket_d[i_bucket_d] += 1;
                total_per_bucket[i_bucket] += count_per_ip;
            }

            /* Reset the counters */
            min_tld_delay = -1;
            count_per_ip = 0;
            tld_sum_delay = 0;
            tld_nb_delay = 0;
        }
    }

    if (F != NULL) {
#ifdef PRIVACY_CONSCIOUS
        if (compress_name_and_address_reports) {
            ithi_pipe_close(F);
        }
        else {
            (void)fclose(F);
        }
#else
        (void)fclose(F);
#endif
    }

    SubmitRegistryNumberAndCount(REGISTRY_DNS_UsefulQueries, 1, total_no_error_entries);
    SubmitRegistryNumberAndCount(REGISTRY_DNS_UsefulQueries, 0, total_no_error_queries - total_no_error_entries);
    if (capture_cache_ratio_nx_domain) {
        SubmitRegistryNumberAndCount(REGISTRY_DNS_UsefulQueries, 2, total_error_entries);
        SubmitRegistryNumberAndCount(REGISTRY_DNS_UsefulQueries, 3, total_error_queries - total_error_entries);
    }

    /* Add the counters per bucket */
    for (int i_bucket = 0; i_bucket < 9; i_bucket++) {
        SubmitRegistryNumberAndCount(REGISTRY_DNS_TLD_MIN_DELAY_IP, cache_bucket[i_bucket], ip_per_bucket[i_bucket]);
        SubmitRegistryNumberAndCount(REGISTRY_DNS_TLD_AVG_DELAY_IP, cache_bucket[i_bucket], ip_per_bucket_d[i_bucket]);
        SubmitRegistryNumberAndCount(REGISTRY_DNS_TLD_MIN_DELAY_LOAD, cache_bucket[i_bucket], total_per_bucket[i_bucket]);
    }

    queryUsage.Clear();
}

#ifdef PRIVACY_CONSCIOUS
void DnsStats::ExportNameReport()
{
    DnsNameEntry* name_entry;
    FILE* F = NULL;
    
    if (compress_name_and_address_reports) {
        int err = 0;
        F = ithi_gzip_compress_open(name_report, &err);
        if (F == NULL || err != 0) {
            fprintf(stderr, "Cannot open file <%s> for compression, err = %d\n", name_report, err);
        }
    }
    else {
        F = ithi_file_open(name_report, "w");
        if (F == NULL) {
            fprintf(stderr, "Cannot open file <%s> for writing\n", name_report);
        }
    }

    if (F != NULL) {
        bool ret = true;

        if (fprintf(F, "Name, nx_domain, name_type, count, IP, RR, flags\n") <= 0) {
            ret = false;
            fprintf(stderr, "Cannot write header line to <%s>\n", name_report);
        }

        for (uint32_t i = 0; ret && i < nameList.GetSize(); i++)
        {
            name_entry = nameList.GetEntry(i);

            while (ret && name_entry != NULL)
            {
                char safe_name[1024];

                if (ithi_copy_to_safe_text(safe_name, sizeof(safe_name), name_entry->name, name_entry->name_len) <= 0) {
                    fprintf(stderr, "Cannot sanitize name entry #%d (%s) for <%s>\n", i, name_entry->name, name_report);
                    ret = false;
                } else if (fprintf(F, "%s,%d,%s,%llu,", safe_name, name_entry->is_nx, LeakTypeName(name_entry->leakType), (unsigned long long)name_entry->count) <= 0){
                    ret = false;
                    fprintf(stderr, "Cannot export entry #%d (%s) to <%s>\n", i, name_entry->name, name_report);
                } else if (name_entry->addr_len == 4) {
                    if (fprintf(F, "%d.%d.%d.%d", name_entry->addr[0], name_entry->addr[1], name_entry->addr[2], name_entry->addr[3]) <= 0) {
                        ret = false;
                        fprintf(stderr, "Cannot export IP address for entry #%d/\n", i);
                    }
                }
                else if (name_entry->addr_len == 16) {
                    if (fprintf(F,
                        "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                        name_entry->addr[0], name_entry->addr[1], name_entry->addr[2], name_entry->addr[3],
                        name_entry->addr[4], name_entry->addr[5], name_entry->addr[6], name_entry->addr[7],
                        name_entry->addr[8], name_entry->addr[9], name_entry->addr[10], name_entry->addr[11],
                        name_entry->addr[12], name_entry->addr[13], name_entry->addr[14], name_entry->addr[15]) <= 0) {
                        ret = false;
                        fprintf(stderr, "Cannot export IP address for entry #%d/\n", i);
                    }
                }
                else if (fprintf(F, "::0") <= 0) {
                    fprintf(stderr, "Cannot export NULL IP address for entry #%d/\n", i);
                }
                
                if (ret && 
                    fprintf(F,",%d,%d\n", name_entry->rr_type, name_entry->flags) <= 0){
                    ret = false;
                    fprintf(stderr, "Cannot export end of line for entry #%d/\n", i);
                }
                name_entry = name_entry->HashNext;
            }
        }

        (void)fclose(F);
    }
}
#endif

bool DnsStats::ExportToCaptureSummary(CaptureSummary * cs)
{
    DnsHashEntry *entry;
    CaptureLine line;

    /* Add the leaks references to the exported list */
    SubmitRegistryNumberAndCount(REGISTRY_CHROMIUM_LEAK_REF,
        0, DnsStats::GetLeaksRef());

    /* Export the duration if not already done */
    if (t_start_sec != 0 && t_start_usec != 0) {
        SubmitRegistryNumberAndCount(REGISTRY_VOLUME_53ONLY, 0, volume_53only);
        SubmitRegistryNumberAndCount(REGISTRY_CAPTURE_DURATION53, 0, duration_usec);
    }

    /* Get the ordered list of leaked domain into the main hash */
    ExportLeakedDomains();
    /* Get the ordered list of 2nd level domains */
    ExportSecondLeaked();
    /* Get the ordered list of domain string usage into the main hash */
    ExportStringUsage();
    /* Get the statistics on DO bit, EDNS and QNAME minimization */
    ExportStatsByIp();
    /* get the counts of DNSSEC usage per address and per zone */
    ExportDnssecUsage();
    /* export the cache statistics */
    ExportQueryUsage();
#ifdef PRIVACY_CONSCIOUS
    if (name_report != NULL) {
        ExportNameReport();
    }
#endif

    /* Export the data */
    cs->Reserve((size_t)hashTable.GetCount()+1);

    /* Export the stored values */
    for (uint32_t i = 0; i < hashTable.GetSize(); i++)
    {
        entry = hashTable.GetEntry(i);

        while (entry != NULL)
        {
            if (entry->registry_id < RegistryNameByIdNb)
            {
                memcpy(line.registry_name, RegistryNameById[entry->registry_id],
                    strlen(RegistryNameById[entry->registry_id]) + 1);
            }
            else
            {
                /* turns out that itoa() is not portable, so here we go. */
                char number[16];
                uint32_t target = entry->registry_id;
                size_t k = 0;
                for (; (target > 0 || k == 0) && k < 16; k++)
                {
                    number[k] = (target % 10) + '0';
                    target /= 10;
                }

                for (size_t l = 0; l < k; l++)
                {
                    line.registry_name[l] = number[k - 1 - l];
                }

                line.registry_name[k] = 0;
            }
            line.key_type = entry->key_type;

            if (entry->key_type == 0)
            {
                line.key_number = entry->key_number;
            }
            else
            {
                char text[128];
                size_t text_length = 0;

                text_length = ithi_copy_to_safe_text(text, sizeof(text), entry->key_value, entry->key_length);

                if (text_length < sizeof(line.key_value)) {
                    memcpy(line.key_value, text, text_length);
                    line.key_value[text_length] = 0;
                }
                else {
                    if (text_length < sizeof(text)) {
                        text[text_length] = 0;
                    }
                    else {
                        text[sizeof(text) - 1] = 0;
                    }
                    fprintf(stderr, "Cannot copy key value: %s\n", text);
                }
            }
            line.count = entry->count;

            cs->AddLine(&line, true);

            entry = entry->HashNext;
        }
    }

    cs->Sort();


    return true;
}

TldAsKey::TldAsKey(uint8_t * tld, size_t tld_len)
    :
    HashNext(NULL),
    MoreRecentKey(NULL),
    LessRecentKey(NULL),
    max_name_parts(1),
    count(1),
    hash(0)
{
    CanonicCopy(this->tld, sizeof(this->tld) - 1, &this->tld_len, tld, tld_len);
}

TldAsKey::TldAsKey(uint8_t* tld, size_t tld_len, uint32_t nb_name_parts)
    :
    HashNext(NULL),
    MoreRecentKey(NULL),
    LessRecentKey(NULL),
    max_name_parts(nb_name_parts),
    count(1),
    hash(0)
{
    CanonicCopy(this->tld, sizeof(this->tld) - 1, &this->tld_len, tld, tld_len);
}

TldAsKey::~TldAsKey()
{
}

bool TldAsKey::IsSameKey(TldAsKey * key)
{
    bool ret = (this->tld_len == key->tld_len &&
        memcmp(this->tld, key->tld, this->tld_len) == 0);

    return ret;
}

uint32_t TldAsKey::Hash()
{
    if (hash == 0)
    {
        hash = 0xBABAC001;

        for (size_t i = 0; i < tld_len; i++)
        {
            hash = hash * 101 + tld[i];
        }
    }

    return hash;
}

TldAsKey * TldAsKey::CreateCopy()
{
    TldAsKey * ret = new TldAsKey(this->tld, this->tld_len, this->max_name_parts);

    if (ret != NULL)
    {
        ret->count = count;
    }

    return ret;
}

void TldAsKey::Add(TldAsKey * key)
{
    if (this->max_name_parts < key->max_name_parts) {
        this->max_name_parts = key->max_name_parts;
    }
    this->count += key->count;
}

void TldAsKey::CanonicCopy(uint8_t * tldDest, size_t tldDestMax, size_t * tldDestLength, 
    uint8_t * tldSrce, size_t tldSrceLength)
{
    size_t i = 0;

    for (; i < tldSrceLength && i < tldDestMax; i++)
    {
        int c = tldSrce[i];

        if (c >= 'a' && c <= 'z')
        {
            c += 'A' - 'a';
        }

        tldDest[i] = c;
    }

    *tldDestLength = i;

    tldDest[i] = 0;
}


TldAddressAsKey::TldAddressAsKey(uint8_t * addr, size_t addr_len, uint8_t * tld, size_t tld_len, my_bpftimeval ts, int is_nx, DnsStatsLeakType leakType, int flags)
    :
    HashNext(NULL),
    count(1),
    hash(0),
    tld_min_delay(-1),
    is_nx(is_nx),
    leakType(leakType),
    flags(flags)
{
    if (addr_len > 16)
    {
        addr_len = 16;
    }

    memcpy(this->addr, addr, addr_len);
    this->addr_len = addr_len;

    this->ts.tv_sec = ts.tv_sec;
    this->ts.tv_usec = ts.tv_usec;

    this->ts_init.tv_sec = ts.tv_sec;
    this->ts_init.tv_usec = ts.tv_usec;

    TldAsKey::CanonicCopy(this->tld, sizeof(this->tld) - 1, &this->tld_len, tld, tld_len);
}

TldAddressAsKey::~TldAddressAsKey()
{
}

bool TldAddressAsKey::IsSameKey(TldAddressAsKey * key)
{
    bool ret = (this->tld_len == key->tld_len &&
        (this->tld_len == 0 || memcmp(this->tld, key->tld, this->tld_len) == 0) &&
        this->addr_len == key->addr_len &&
        memcmp(this->addr, key->addr, this->addr_len) == 0);

    return ret;
}

uint32_t TldAddressAsKey::Hash()
{
    if (hash == 0)
    {
        hash = 0xCACAB0B0;

        for (size_t i = 0; i < tld_len; i++)
        {
            hash = hash * 101 + tld[i];
        }

        for (size_t i = 0; i < addr_len; i++)
        {
            hash = hash * 101 + addr[i];
        }
    }

    return hash;
}

TldAddressAsKey * TldAddressAsKey::CreateCopy()
{
    TldAddressAsKey* ret = new TldAddressAsKey(addr, addr_len, tld, tld_len, ts, is_nx, leakType, flags);

    if (ret != NULL)
    {
        ret->count = count;
        ret->tld_min_delay = tld_min_delay;
        ret->ts_init.tv_sec = ts_init.tv_sec;
        ret->ts_init.tv_usec = ts_init.tv_usec;
    }

    return ret;
}

void TldAddressAsKey::Add(TldAddressAsKey * key)
{
    this->count += key->count;
    this->flags |= key->flags;
}

bool TldAddressAsKey::CompareByAddressAndTld(TldAddressAsKey * x, TldAddressAsKey * y)
{
    bool ret = x->addr_len > y->addr_len;

    if (x->addr_len == y->addr_len)
    {
        int r = memcmp(x->addr, y->addr, x->addr_len);

        if (r > 0) {
            ret = true;
        }
        else if (r == 0) {
            size_t tld_len = x->tld_len;
            if (y->tld_len < x->tld_len) {
                tld_len = y->tld_len;
            }
            r = memcmp(x->tld, y->tld, tld_len);
            if (r > 0) {
                ret = true;
            }
            else if (r == 0) {
                ret = x->tld_len > y->tld_len;
            }
        }
    }

    return ret;
}

DnsHashEntry::DnsHashEntry()
    :
    HashNext(NULL),
    hash(0),
    registry_id(0),
    count(0),
    key_type(0),
    key_length(0),
    key_number(0)
{
    key_value[0] = 0;
}

DnsHashEntry::~DnsHashEntry()
{
}

bool DnsHashEntry::IsSameKey(DnsHashEntry * key)
{
    bool ret = registry_id == key->registry_id &&
        key_type == key->key_type &&
        key_length == key->key_length &&
        memcmp(key_value, key->key_value, key_length) == 0;

    return ret;
}

uint32_t DnsHashEntry::Hash()
{
    if (hash == 0)
    {
        uint64_t hash64 = 0;

        hash64 = registry_id;
        hash64 ^= (hash64 << 23) ^ (hash64 >> 17);
        hash64 ^= key_type;
        hash64 ^= (hash64 << 23) ^ (hash64 >> 17);
        hash64 ^= key_length;
        hash64 ^= (hash64 << 23) ^ (hash64 >> 17);
        for (uint32_t i = 0; i < key_length; i++)
        {
            hash64 ^= key_value[i];
            hash64 ^= (hash64 << 23) ^ (hash64 >> 17);
        }

        hash = (uint32_t)(hash64 ^ (hash64 >> 32));
    }
    return hash;
}

DnsHashEntry * DnsHashEntry::CreateCopy()
{
    DnsHashEntry * key = new DnsHashEntry();

    if (key != NULL)
    {
        key->registry_id = registry_id;
        key->key_type = key_type;
        key->key_length = key_length;
        memcpy(key->key_value, key_value, key_length);
        key->count = count;
    }

    return key;
}

void DnsHashEntry::Add(DnsHashEntry * key)
{
    count += key->count;
}

DnsPrefixEntry::DnsPrefixEntry()
    :
    HashNext(NULL),
    hash(0),
    dnsPrefix(NULL),
    dnsPrefixClass(DnsPrefixStd)
{
}

DnsPrefixEntry::~DnsPrefixEntry()
{
}

bool DnsPrefixEntry::IsSameKey(DnsPrefixEntry * key)
{
    return strcmp(dnsPrefix, key->dnsPrefix) == 0;
}

uint32_t DnsPrefixEntry::Hash()
{
    if (hash == 0)
    {
        size_t l = strlen(dnsPrefix);

        hash = 0xCACAB0B0;

        for (size_t i = 0; i < l; i++)
        {
            hash = hash * 101 + dnsPrefix[i];
        }
    }

    return hash;
}

DnsPrefixEntry * DnsPrefixEntry::CreateCopy()
{
    return NULL;
}

void DnsPrefixEntry::Add(DnsPrefixEntry * key)
{
    UNREFERENCED_PARAMETER(key);
}

#include "DnsPrefixList.inc"

void DnsStats::LoadPrefixTable_from_memory()
{
    for (size_t i = 0; i < nbDnsPrefixList; i++) {
        char const * x = dnsPrefixList[i];
        DnsPrefixEntry * dpe = new DnsPrefixEntry();
        bool stored = false;

        if (x[0] == '!') {
            dpe->dnsPrefix = (char *)(x + 1);
            dpe->dnsPrefixClass = DnsPrefixException;
        } else if (x[0] == '*') {
            dpe->dnsPrefix = (char *)(x + 2);
            dpe->dnsPrefixClass = DnsPrefixOneLevel;
        } else {
            dpe->dnsPrefix = (char *)(x);
            dpe->dnsPrefixClass = DnsPrefixStd;
        }

        dnsPrefixTable.InsertOrAdd(dpe, false, &stored);

        if (!stored)
        {
            delete dpe;
        }
    }
}

const char * DnsStats::GetZonePrefix(const char * dnsName)
{
    const char * ret = NULL;
    DnsPrefixEntry dpe;
    DnsPrefixEntry * retrieved;
    int prefixOffset = 0;
    int previousOffset = -1;
    int previousPreviousOffset = -1;

    if (dnsPrefixTable.GetSize() == 0) {
        LoadPrefixTable_from_memory();
    }

    while (ret == NULL && dnsName != NULL && dnsName[prefixOffset] != '.') {
        dpe.dnsPrefix = (char *)(dnsName + prefixOffset);
        dpe.hash = 0;

        if ((retrieved = dnsPrefixTable.Retrieve(&dpe)) != NULL)
        {
            switch (retrieved->dnsPrefixClass) {
            case DnsPrefixOneLevel:
                if (previousPreviousOffset >= 0) {
                    ret = dnsName + previousPreviousOffset;
                }
                break;
            case DnsPrefixException:
                ret = dnsName + prefixOffset;
                break;
            case DnsPrefixStd:
            default:
                if (previousOffset >= 0) {
                    ret = dnsName + previousOffset;
                }
                break;
            }
            break;
        } else {
            int offset = prefixOffset;

            while (dnsName[offset] != 0 && dnsName[offset] != '.') {
                offset++;
            }

            if (dnsName[offset] == 0) {
                if (previousOffset >= 0) {
                    ret = dnsName + previousOffset;
                }
                break;
            } else {
                previousPreviousOffset = previousOffset;
                previousOffset = prefixOffset;
                prefixOffset = offset + 1; /* Add 1 to skip the dot */
            }
        }
    }

    return ret;
}

void DnsStats::RegisterDnssecUsageByName(uint8_t * packet, uint32_t length, uint32_t name_start,
    bool is_dnssec)
{
    /* Get the query name */
    uint8_t name[1024];
    size_t name_len = 0; 
    const char * zone_prefix = NULL;

    (void)GetDnsName(packet, length, name_start, name, sizeof(name), &name_len);

    /* Get the query prefix */
    if (name_len > 0) {
        SetToUpperCase(name, name_len);
        zone_prefix = GetZonePrefix((const char *)name);
    }

    /* Register */
    if (zone_prefix != NULL) {
        RegisterDnssecUsageByPrefix(&dnssecPrefixTable,
            (uint8_t *)zone_prefix, strlen(zone_prefix), is_dnssec);
    }
}

void DnsStats::ExportDnssecUsage()
{
    ExportDnssecUsageByTable(&dnssecPrefixTable, REGISTRY_DNSSEC_Zone_Usage);
    dnssecPrefixTable.Clear();
}

void DnsStats::RegisterDnssecUsageByPrefix(
    BinHash<DnssecPrefixEntry> * dnssecTable,
    uint8_t * prefix, size_t prefix_length, bool is_dnssec)
{
    DnssecPrefixEntry dpe;
    bool stored = false;

    dpe.prefix = prefix;
    dpe.prefix_len = prefix_length;
    dpe.is_dnssec = is_dnssec;

    dnssecTable->InsertOrAdd(&dpe, true, &stored);
}

void DnsStats::ExportDnssecUsageByTable(BinHash<DnssecPrefixEntry>* dnssecTable, uint32_t registry_id)
{
    DnssecPrefixEntry *dpe;
    uint32_t usage_count = 0;
    uint32_t nonusage_count = 0;

    for (uint32_t i = 0; i < dnssecTable->GetSize(); i++)
    {
        dpe = dnssecTable->GetEntry(i);

        while (dpe != NULL)
        {
            if (dpe->is_dnssec) {
                usage_count++;
            } else {
                nonusage_count++;
            }
            dpe = dpe->HashNext;
        }
    }
    
    SubmitRegistryNumberAndCount(registry_id, 0, nonusage_count);
    SubmitRegistryNumberAndCount(registry_id, 1, usage_count);
}


void DnsStats::RegisterStatsByIp(uint8_t * dest_addr, size_t dest_addr_length)
{
    StatsByIP x(dest_addr, dest_addr_length, is_do_flag_set, is_using_edns,
        !is_qname_minimized, is_recursive_query);
    StatsByIP * y = statsByIp.Retrieve(&x);

    x.response_seen = true;

    if (y == NULL) {
        if (statsByIp.GetCount() < max_stats_by_ip_count) {
            bool stored = false;

            (void)statsByIp.InsertOrAdd(&x, true, &stored);
        }
    }
    else {
        y->Add(&x);
    }
}

/* This call assumes that is_using_edns and edns_options are set to
 * correct value when parsing the records */
void DnsStats::RegisterOptionsByIp(const uint8_t * source_addr, size_t source_addr_length)
{
    StatsByIP x(source_addr, source_addr_length, false, false, false, false);
    StatsByIP * y = statsByIp.Retrieve(&x);

    if (y == NULL) {
        if (statsByIp.GetCount() < max_stats_by_ip_count) {
            bool stored = false;

            if ((y = statsByIp.InsertOrAdd(&x, true, &stored)) != NULL) {
                y->query_seen = true;
            }
        }
    } else {
        if (!y->query_seen) {
            y->count++;
            y->query_seen = true;
        }
    }

    if (y != NULL && is_using_edns && edns_options != NULL) {
        /* Should export the EDNS Options to new table. */
        uint32_t current_index = 0;
        while (current_index + 4 <= edns_options_length)
        {
            uint16_t o_code = (edns_options[current_index] << 8) | edns_options[current_index + 1];
            uint16_t o_length = (edns_options[current_index + 2] << 8) | edns_options[current_index + 3];
            bool should_export = y->RegisterNewOption(o_code);

            if (should_export) {
                SubmitRegistryNumber(REGISTRY_EDNS_OPT_USAGE, o_code);
            }
            current_index += 4 + o_length;
        }
    }
}

void DnsStats::RegisterTcpSynByIp(uint8_t * source_addr,
    size_t source_addr_length, bool tcp_port_583, bool tcp_port_443)
{
    if ((tcp_port_583 && tcp_port_443) || (!tcp_port_583 && !tcp_port_443)) {
        return;
    }

    StatsByIP x(source_addr, source_addr_length, false, false, false, false);
    StatsByIP * y = statsByIp.Retrieve(&x);
    if (tcp_port_443) {
        x.nb_tcp_443++;
    }

    if (tcp_port_583) {
        x.nb_tcp_583++;
    }

    if (y == NULL) {
        if (statsByIp.GetCount() < max_stats_by_ip_count) {
            bool stored = false;

            (void)statsByIp.InsertOrAdd(&x, true, &stored);
        }
    }
    else {
        y->Add(&x);
    }

}

void DnsStats::ExportStatsByIp()
{
    StatsByIP *sbi;
    uint32_t using_do_count = 0;
    uint32_t not_using_do_count = 0;
    uint32_t using_edns_count = 0;
    uint32_t not_using_edns_count = 0;
    uint32_t qname_minimizing_count = 0;
    uint32_t not_minimizing_count = 0;
    uint32_t total_queries = 0;
    uint32_t issued_syn_583 = 0;
    uint32_t issued_syn_443 = 0;
    uint32_t sending_recursive_count = 0;
    uint32_t not_sending_recursive_count = 0;

    for (uint32_t i = 0; i < statsByIp.GetSize(); i++)
    {
        sbi = statsByIp.GetEntry(i);

        while (sbi != NULL)
        {
            if (sbi->query_seen) {
                total_queries++;
            }

            if (sbi->response_seen) {
                if (sbi->IsDoUsed()) {
                    using_do_count++;
                }
                else {
                    not_using_do_count++;
                }

                if (sbi->IsEdnsSupported()) {
                    using_edns_count++;
                }
                else {
                    not_using_edns_count++;
                }

                if (sbi->IsQnameMinimized()) {
                    qname_minimizing_count++;
                }
                else {
                    not_minimizing_count++;
                }

                if (sbi->nb_recursive_queries > 0) {
                    sending_recursive_count++;
                }
                else {
                    not_sending_recursive_count++;
                }
            }

            if (sbi->nb_tcp_443 > 0) {
                issued_syn_443++;
            }

            if (sbi->nb_tcp_583 > 0) {
                issued_syn_583++;
            }

            sbi = sbi->HashNext;
        }
    }

    SubmitRegistryNumberAndCount(REGISTRY_DNSSEC_Client_Usage, 0, not_using_do_count);
    SubmitRegistryNumberAndCount(REGISTRY_DNSSEC_Client_Usage, 1, using_do_count);

    SubmitRegistryNumberAndCount(REGISTRY_EDNS_Client_Usage, 0, not_using_edns_count);
    SubmitRegistryNumberAndCount(REGISTRY_EDNS_Client_Usage, 1, using_edns_count);

    SubmitRegistryNumberAndCount(REGISTRY_QNAME_MINIMIZATION_Usage, 0, not_minimizing_count);
    SubmitRegistryNumberAndCount(REGISTRY_QNAME_MINIMIZATION_Usage, 1, qname_minimizing_count);

    SubmitRegistryNumberAndCount(REGISTRY_RESOLVER_SENDING_RECURSIVE, 0, not_sending_recursive_count);
    SubmitRegistryNumberAndCount(REGISTRY_RESOLVER_SENDING_RECURSIVE, 1, sending_recursive_count);

    SubmitRegistryNumberAndCount(REGISTRY_EDNS_OPT_USAGE_REF, 0, total_queries);

    if (!is_capture_dns_only) {
        /* Only add these counts if using PCAP directly, not DNSCAP */
        SubmitRegistryNumberAndCount(REGISTRY_TCPSYN_PER_PROTO, 0, statsByIp.GetSize());
        SubmitRegistryNumberAndCount(REGISTRY_TCPSYN_PER_PROTO, 583, issued_syn_583);
        SubmitRegistryNumberAndCount(REGISTRY_TCPSYN_PER_PROTO, 443, issued_syn_443);
    }

    statsByIp.Clear();
}

DnssecPrefixEntry::DnssecPrefixEntry() :
    HashNext(NULL),
    hash(0),
    prefix(NULL),
    prefix_len(0),
    is_dnssec(false),
    prefix_data(NULL)
{
}

DnssecPrefixEntry::~DnssecPrefixEntry()
{
    if (prefix_data != NULL) {
        delete[] prefix_data;
        prefix_data = NULL;
    }
}

bool DnssecPrefixEntry::IsSameKey(DnssecPrefixEntry * key)
{
    return (prefix_len == key->prefix_len &&
        ((prefix_len == 0 && prefix == NULL && key->prefix == NULL) ||
        (prefix_len > 0 && prefix != NULL && key->prefix != NULL &&
            memcmp(prefix, key->prefix, prefix_len) == 0)));
}

uint32_t DnssecPrefixEntry::Hash()
{
    if (hash == 0)
    {
        hash = 0xCACAB0B0;

        for (size_t i = 0; i < prefix_len; i++)
        {
            hash = hash * 101 + prefix[i];
        }
    }

    return hash;
}

DnssecPrefixEntry * DnssecPrefixEntry::CreateCopy()
{
    DnssecPrefixEntry * key = new DnssecPrefixEntry();

    if (key != NULL)
    {
        key->is_dnssec = is_dnssec;
        key->prefix_len = prefix_len;
        if (prefix_len > 0) {
            if (key->prefix_data != NULL) {
                delete[] key->prefix_data;
            }
            key->prefix_data = new uint8_t[prefix_len];
            key->prefix = key->prefix_data;

            if (key->prefix == NULL) {
                delete key;
                key = NULL;
            }
            else {
                memcpy(key->prefix, prefix, prefix_len);
            }
        }
    }

    return key;
}

void DnssecPrefixEntry::Add(DnssecPrefixEntry * key)
{
    is_dnssec |= key->is_dnssec;
}

DomainEntry::DomainEntry() 
    :
    HashNext(NULL),
    hash(0),
    domain_length(0),
    domain (NULL),
    count(0)
{
}

DomainEntry::~DomainEntry()
{
    if (domain != NULL) {
        delete[] domain;
        domain = NULL;
    }
}

bool DomainEntry::IsSameKey(DomainEntry * key)
{
    return (domain_length == key->domain_length &&
        ((domain_length == 0 && domain == NULL && key->domain == NULL) ||
        (domain_length > 0 && domain != NULL && key->domain != NULL &&
            memcmp(domain, key->domain, domain_length) == 0)));
}

uint32_t DomainEntry::Hash()
{
    if (hash == 0)
    {
        hash = 0xCACAB0B0;

        for (size_t i = 0; i < domain_length; i++)
        {
            hash = hash * 101 + domain[i];
        }
    }

    return hash;
}

DomainEntry * DomainEntry::CreateCopy()
{
    DomainEntry * key = new DomainEntry();

    if (key != NULL)
    {
        key->domain_length = domain_length;
        if (domain_length > 0) {
            if (key->domain != NULL) {
                delete[] key->domain;
            }

            key->domain = new char[(size_t)domain_length+1];

            if (key->domain == NULL) {
                delete key;
                key = NULL;
            }
            else {
                memcpy(key->domain, domain, domain_length);
                key->domain[domain_length] = 0;
            }
        }
    }

    return key;
}

void DomainEntry::Add(DomainEntry * key)
{
    count += key->count;
}

DnsNameEntry::DnsNameEntry():
    HashNext(NULL),
    hash(0),
    name_len(0),
    name(NULL),
    count(0),
    is_nx(0),
    leakType(dnsLeakNoLeak),
    addr_len(0),
    rr_type(0),
    flags(0)
{
}

DnsNameEntry::~DnsNameEntry()
{
    if (name != NULL) {
        delete[] name;
        name = NULL;
    }
}

bool DnsNameEntry::IsSameKey(DnsNameEntry* key)
{
    return (name_len == key->name_len &&
        ((name_len == 0 && name == NULL && key->name == NULL) ||
        (name_len > 0 && name != NULL && key->name != NULL &&
            memcmp(name, key->name, name_len) == 0)));
}

uint32_t DnsNameEntry::Hash()
{
    if (hash == 0)
    {
        hash = 0xCACAB0B0;

        for (size_t i = 0; i < name_len; i++)
        {
            hash = hash * 101 + name[i];
        }
    }

    return hash;
}

DnsNameEntry* DnsNameEntry::CreateCopy()
{
    DnsNameEntry* key = new DnsNameEntry();

    if (key != NULL)
    {
        key->name_len = name_len;
        if (name_len > 0) {
            if (key->name != NULL) {
                delete[] key->name;
            }

            key->name = new uint8_t[(size_t)name_len + 1];

            if (key->name == NULL) {
                delete key;
                key = NULL;
            }
            else {
                memcpy(key->name, name, name_len);
                key->name[name_len] = 0;
                key->count = count;
                key->is_nx = is_nx;
                key->leakType = leakType;
                memcpy(key->addr, addr, addr_len);
                key->addr_len = addr_len;
                key->rr_type = rr_type;
                key->flags = flags;
            }
        }
    }

    return key;
}

void DnsNameEntry::Add(DnsNameEntry* key)
{
    count += key->count;
}
