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
    enable_ip_address_report(false),
    enable_erroneous_name_list(false),
    target_number_dns_packets(0),
    frequent_address_max_count(128),
    max_tld_leakage_count(0x80),
    max_tld_leakage_table_count(0x8000),
    max_query_usage_count(0x8000),
    max_tld_string_usage_count(0x8000),
    max_tld_string_leakage_count(0x200),
    max_stats_by_ip_count(0x8000),
    dnsstat_flags(0),
    record_count(0),
    query_count(0),
    response_count(0)
{
}


DnsStats::~DnsStats()
{
}

static char const * DefaultRootAddresses[] = {
    "2001:503:ba3e::2:30",
    "198.41.0.4",
    "2001:500:200::b",
    "192.228.79.201", /* Since 2017-10-24, b.root-servers.net changed from 192.228.79.201 to 199.9.14.201 */
    "199.9.14.201",   /* See https://b.root-servers.org/news/2017/08/09/new-ipv4.html */
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
    "202.12.27.33"
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
    "FULL_NAME_LIST"
};

static uint32_t RegistryNameByIdNb = sizeof(RegistryNameById) / sizeof(char const*);

static char const * RegisteredTldName[] = {
    "AAA", "AARP", "ABARTH", "ABB", "ABBOTT", "ABBVIE", "ABC", "ABLE", "ABOGADO",
    "ABUDHABI", "AC", "ACADEMY", "ACCENTURE", "ACCOUNTANT", "ACCOUNTANTS", "ACO",
    "ACTOR", "AD", "ADAC", "ADS", "ADULT", "AE", "AEG", "AERO", "AETNA",
    "AF", "AFAMILYCOMPANY", "AFL", "AFRICA", "AG", "AGAKHAN", "AGENCY", "AI",
    "AIG", "AIGO", "AIRBUS", "AIRFORCE", "AIRTEL", "AKDN", "AL", "ALFAROMEO",
    "ALIBABA", "ALIPAY", "ALLFINANZ", "ALLSTATE", "ALLY", "ALSACE", "ALSTOM", "AM",
    "AMERICANEXPRESS", "AMERICANFAMILY", "AMEX", "AMFAM", "AMICA", "AMSTERDAM",
    "ANALYTICS", "ANDROID", "ANQUAN", "ANZ", "AO", "AOL", "APARTMENTS", "APP",
    "APPLE", "AQ", "AQUARELLE", "AR", "ARAB", "ARAMCO", "ARCHI", "ARMY", "ARPA",
    "ART", "ARTE", "AS", "ASDA", "ASIA", "ASSOCIATES", "AT", "ATHLETA", "ATTORNEY",
    "AU", "AUCTION", "AUDI", "AUDIBLE", "AUDIO", "AUSPOST", "AUTHOR", "AUTO", "AUTOS",
    "AVIANCA", "AW", "AWS", "AX", "AXA", "AZ", "AZURE", "BA", "BABY", "BAIDU", "BANAMEX",
    "BANANAREPUBLIC", "BAND", "BANK", "BAR", "BARCELONA", "BARCLAYCARD", "BARCLAYS",
    "BAREFOOT", "BARGAINS", "BASEBALL", "BASKETBALL", "BAUHAUS", "BAYERN", "BB", "BBC",
    "BBT", "BBVA", "BCG", "BCN", "BD", "BE", "BEATS", "BEAUTY", "BEER", "BENTLEY", "BERLIN",
    "BEST", "BESTBUY", "BET", "BF", "BG", "BH", "BHARTI", "BI", "BIBLE", "BID", "BIKE",
    "BING", "BINGO", "BIO", "BIZ", "BJ", "BLACK", "BLACKFRIDAY", "BLOCKBUSTER",
    "BLOG", "BLOOMBERG", "BLUE", "BM", "BMS", "BMW", "BN", "BNL", "BNPPARIBAS", "BO", "BOATS",
    "BOEHRINGER", "BOFA", "BOM", "BOND", "BOO", "BOOK", "BOOKING", "BOSCH", "BOSTIK",
    "BOSTON", "BOT", "BOUTIQUE", "BOX", "BR", "BRADESCO", "BRIDGESTONE", "BROADWAY", "BROKER",
    "BROTHER", "BRUSSELS", "BS", "BT", "BUDAPEST", "BUGATTI", "BUILD", "BUILDERS", "BUSINESS",
    "BUY", "BUZZ", "BV", "BW", "BY", "BZ", "BZH", "CA", "CAB", "CAFE", "CAL", "CALL",
    "CALVINKLEIN", "CAM", "CAMERA", "CAMP", "CANCERRESEARCH", "CANON", "CAPETOWN", "CAPITAL",
    "CAPITALONE", "CAR", "CARAVAN", "CARDS", "CARE", "CAREER", "CAREERS", "CARS", "CARTIER",
    "CASA", "CASE", "CASEIH", "CASH", "CASINO", "CAT", "CATERING", "CATHOLIC", "CBA", "CBN",
    "CBRE", "CBS", "CC", "CD", "CEB", "CENTER", "CEO", "CERN", "CF", "CFA", "CFD", "CG", "CH",
    "CHANEL", "CHANNEL", "CHARITY", "CHASE", "CHAT", "CHEAP", "CHINTAI", "CHRISTMAS", "CHROME", "CHRYSLER",
    "CHURCH", "CI", "CIPRIANI", "CIRCLE", "CISCO", "CITADEL", "CITI", "CITIC", "CITY",
    "CITYEATS", "CK", "CL", "CLAIMS", "CLEANING", "CLICK", "CLINIC", "CLINIQUE", "CLOTHING",
    "CLOUD", "CLUB", "CLUBMED", "CM", "CN", "CO", "COACH", "CODES", "COFFEE", "COLLEGE",
    "COLOGNE", "COM", "COMCAST", "COMMBANK", "COMMUNITY", "COMPANY", "COMPARE", "COMPUTER",
    "COMSEC", "CONDOS", "CONSTRUCTION", "CONSULTING", "CONTACT", "CONTRACTORS", "COOKING",
    "COOKINGCHANNEL", "COOL", "COOP", "CORSICA", "COUNTRY", "COUPON", "COUPONS", "COURSES",
    "CR", "CREDIT", "CREDITCARD", "CREDITUNION", "CRICKET", "CROWN", "CRS", "CRUISE",
    "CRUISES", "CSC", "CU", "CUISINELLA", "CV", "CW", "CX", "CY", "CYMRU", "CYOU", "CZ",
    "DABUR", "DAD", "DANCE", "DATA", "DATE", "DATING", "DATSUN", "DAY", "DCLK", "DDS", "DE",
    "DEAL", "DEALER", "DEALS", "DEGREE", "DELIVERY", "DELL", "DELOITTE", "DELTA", "DEMOCRAT",
    "DENTAL", "DENTIST", "DESI", "DESIGN", "DEV", "DHL", "DIAMONDS", "DIET", "DIGITAL",
    "DIRECT", "DIRECTORY", "DISCOUNT", "DISCOVER", "DISH", "DIY", "DJ", "DK", "DM", "DNP",
    "DO", "DOCS", "DOCTOR", "DODGE", "DOG", "DOMAINS", "DOT", "DOWNLOAD", "DRIVE",
    "DTV", "DUBAI", "DUCK", "DUNLOP", "DUNS", "DUPONT", "DURBAN", "DVAG", "DVR", "DZ",
    "EARTH", "EAT", "EC", "ECO", "EDEKA", "EDU", "EDUCATION", "EE", "EG", "EMAIL", "EMERCK",
    "ENERGY", "ENGINEER", "ENGINEERING", "ENTERPRISES", "EPSON", "EQUIPMENT", "ER",
    "ERICSSON", "ERNI", "ES", "ESQ", "ESTATE", "ESURANCE", "ET", "ETISALAT", "EU",
    "EUROVISION", "EUS", "EVENTS", "EVERBANK", "EXCHANGE", "EXPERT", "EXPOSED", "EXPRESS",
    "EXTRASPACE", "FAGE", "FAIL", "FAIRWINDS", "FAITH", "FAMILY", "FAN", "FANS", "FARM",
    "FARMERS", "FASHION", "FAST", "FEDEX", "FEEDBACK", "FERRARI", "FERRERO", "FI", "FIAT",
    "FIDELITY", "FIDO", "FILM", "FINAL", "FINANCE", "FINANCIAL", "FIRE", "FIRESTONE",
    "FIRMDALE", "FISH", "FISHING", "FIT", "FITNESS", "FJ", "FK", "FLICKR", "FLIGHTS", "FLIR",
    "FLORIST", "FLOWERS", "FLY", "FM", "FO", "FOO", "FOOD", "FOODNETWORK", "FOOTBALL", "FORD",
    "FOREX", "FORSALE", "FORUM", "FOUNDATION", "FOX", "FR", "FREE", "FRESENIUS", "FRL",
    "FROGANS", "FRONTDOOR", "FRONTIER", "FTR", "FUJITSU", "FUJIXEROX", "FUN", "FUND",
    "FURNITURE", "FUTBOL", "FYI", "GA", "GAL", "GALLERY", "GALLO", "GALLUP", "GAME", "GAMES",
    "GAP", "GARDEN", "GB", "GBIZ", "GD", "GDN", "GE", "GEA", "GENT", "GENTING", "GEORGE",
    "GF", "GG", "GGEE", "GH", "GI", "GIFT", "GIFTS", "GIVES", "GIVING", "GL", "GLADE",
    "GLASS", "GLE", "GLOBAL", "GLOBO", "GM", "GMAIL", "GMBH", "GMO", "GMX", "GN", "GODADDY",
    "GOLD", "GOLDPOINT", "GOLF", "GOO", "GOODYEAR", "GOOG", "GOOGLE", "GOP",
    "GOT", "GOV", "GP", "GQ", "GR", "GRAINGER", "GRAPHICS", "GRATIS", "GREEN", "GRIPE",
    "GROCERY", "GROUP", "GS", "GT", "GU", "GUARDIAN", "GUCCI", "GUGE", "GUIDE", "GUITARS",
    "GURU", "GW", "GY", "HAIR", "HAMBURG", "HANGOUT", "HAUS", "HBO", "HDFC", "HDFCBANK",
    "HEALTH", "HEALTHCARE", "HELP", "HELSINKI", "HERE", "HERMES", "HGTV", "HIPHOP",
    "HISAMITSU", "HITACHI", "HIV", "HK", "HKT", "HM", "HN", "HOCKEY", "HOLDINGS", "HOLIDAY",
    "HOMEDEPOT", "HOMEGOODS", "HOMES", "HOMESENSE", "HONDA", "HONEYWELL", "HORSE",
    "HOSPITAL", "HOST", "HOSTING", "HOT", "HOTELES", "HOTELS", "HOTMAIL", "HOUSE", "HOW",
    "HR", "HSBC", "HT", "HU", "HUGHES", "HYATT", "HYUNDAI", "IBM", "ICBC", "ICE",
    "ICU", "ID", "IE", "IEEE", "IFM", "IKANO", "IL", "IM", "IMAMAT", "IMDB", "IMMO",
    "IMMOBILIEN", "IN", "INDUSTRIES", "INFINITI", "INFO", "INC", "ING", "INK", "INSTITUTE",
    "INSURANCE", "INSURE", "INT", "INTEL", "INTERNATIONAL", "INTUIT", "INVESTMENTS", "IO",
    "IPIRANGA", "IQ", "IR", "IRISH", "IS", "ISELECT", "ISMAILI", "IST", "ISTANBUL", "IT",
    "ITAU", "ITV", "IVECO", "JAGUAR", "JAVA", "JCB", "JCP", "JE", "JEEP", "JETZT",
    "JEWELRY", "JIO", "JLL", "JM", "JMP", "JNJ", "JO", "JOBS", "JOBURG", "JOT", "JOY",
    "JP", "JPMORGAN", "JPRS", "JUEGOS", "JUNIPER", "KAUFEN", "KDDI", "KE", "KERRYHOTELS",
    "KERRYLOGISTICS", "KERRYPROPERTIES", "KFH", "KG", "KH", "KI", "KIA", "KIM", "KINDER",
    "KINDLE", "KITCHEN", "KIWI", "KM", "KN", "KOELN", "KOMATSU", "KOSHER", "KP", "KPMG",
    "KPN", "KR", "KRD", "KRED", "KUOKGROUP", "KW", "KY", "KYOTO", "KZ", "LA", "LACAIXA",
    "LADBROKES", "LAMBORGHINI", "LAMER", "LANCASTER", "LANCIA", "LANCOME", "LAND",
    "LANDROVER", "LANXESS", "LASALLE", "LAT", "LATINO", "LATROBE", "LAW", "LAWYER", "LB",
    "LC", "LDS", "LEASE", "LECLERC", "LEFRAK", "LEGAL", "LEGO", "LEXUS", "LGBT", "LI",
    "LIAISON", "LIDL", "LIFE", "LIFEINSURANCE", "LIFESTYLE", "LIGHTING", "LIKE", "LILLY",
    "LIMITED", "LIMO", "LINCOLN", "LINDE", "LINK", "LIPSY", "LIVE", "LIVING", "LIXIL", "LK", "LLC",
    "LOAN", "LOANS", "LOCKER", "LOCUS", "LOFT", "LOL", "LONDON", "LOTTE", "LOTTO", "LOVE",
    "LPL", "LPLFINANCIAL", "LR", "LS", "LT", "LTD", "LTDA", "LU", "LUNDBECK", "LUPIN",
    "LUXE", "LUXURY", "LV", "LY", "MA", "MACYS", "MADRID", "MAIF", "MAISON", "MAKEUP", "MAN",
    "MANAGEMENT", "MANGO", "MAP", "MARKET", "MARKETING", "MARKETS", "MARRIOTT", "MARSHALLS",
    "MASERATI", "MATTEL", "MBA", "MC", "MCKINSEY", "MD", "ME", "MED", "MEDIA", "MEET",
    "MELBOURNE", "MEME", "MEMORIAL", "MEN", "MENU", "MERCKMSD", "METLIFE", "MG", "MH",
    "MIAMI", "MICROSOFT", "MIL", "MINI", "MINT", "MIT", "MITSUBISHI", "MK", "ML", "MLB",
    "MLS", "MM", "MMA", "MN", "MO", "MOBI", "MOBILE", "MOBILY", "MODA", "MOE", "MOI", "MOM",
    "MONASH", "MONEY", "MONSTER", "MOPAR", "MORMON", "MORTGAGE", "MOSCOW", "MOTO",
    "MOTORCYCLES", "MOV", "MOVIE", "MOVISTAR", "MP", "MQ", "MR", "MS", "MSD", "MT", "MTN",
    "MTR", "MU", "MUSEUM", "MUTUAL", "MV", "MW", "MX", "MY", "MZ", "NA", "NAB", "NADEX",
    "NAGOYA", "NAME", "NATIONWIDE", "NATURA", "NAVY", "NBA", "NC", "NE", "NEC", "NET",
    "NETBANK", "NETFLIX", "NETWORK", "NEUSTAR", "NEW", "NEWHOLLAND", "NEWS", "NEXT",
    "NEXTDIRECT", "NEXUS", "NF", "NFL", "NG", "NGO", "NHK", "NI", "NICO", "NIKE", "NIKON",
    "NINJA", "NISSAN", "NISSAY", "NL", "NO", "NOKIA", "NORTHWESTERNMUTUAL", "NORTON", "NOW",
    "NOWRUZ", "NOWTV", "NP", "NR", "NRA", "NRW", "NTT", "NU", "NYC", "NZ", "OBI",
    "OBSERVER", "OFF", "OFFICE", "OKINAWA", "OLAYAN", "OLAYANGROUP", "OLDNAVY", "OLLO",
    "OM", "OMEGA", "ONE", "ONG", "ONL", "ONLINE", "ONYOURSIDE", "OOO", "OPEN", "ORACLE",
    "ORANGE", "ORG", "ORGANIC", "ORIGINS", "OSAKA", "OTSUKA", "OTT", "OVH", "PA", "PAGE",
    "PANASONIC", "PARIS", "PARS", "PARTNERS", "PARTS", "PARTY", "PASSAGENS",
    "PAY", "PCCW", "PE", "PET", "PF", "PFIZER", "PG", "PH", "PHARMACY", "PHD", "PHILIPS",
    "PHONE", "PHOTO", "PHOTOGRAPHY", "PHOTOS", "PHYSIO", "PIAGET", "PICS", "PICTET",
    "PICTURES", "PID", "PIN", "PING", "PINK", "PIONEER", "PIZZA", "PK", "PL", "PLACE",
    "PLAY", "PLAYSTATION", "PLUMBING", "PLUS", "PM", "PN", "PNC", "POHL", "POKER", "POLITIE",
    "PORN", "POST", "PR", "PRAMERICA", "PRAXI", "PRESS", "PRIME", "PRO", "PROD",
    "PRODUCTIONS", "PROF", "PROGRESSIVE", "PROMO", "PROPERTIES", "PROPERTY", "PROTECTION",
    "PRU", "PRUDENTIAL", "PS", "PT", "PUB", "PW", "PWC", "PY", "QA", "QPON", "QUEBEC",
    "QUEST", "QVC", "RACING", "RADIO", "RAID", "RE", "READ", "REALESTATE", "REALTOR",
    "REALTY", "RECIPES", "RED", "REDSTONE", "REDUMBRELLA", "REHAB", "REISE", "REISEN",
    "REIT", "RELIANCE", "REN", "RENT", "RENTALS", "REPAIR", "REPORT", "REPUBLICAN", "REST",
    "RESTAURANT", "REVIEW", "REVIEWS", "REXROTH", "RICH", "RICHARDLI", "RICOH",
    "RIGHTATHOME", "RIL", "RIO", "RIP", "RMIT", "RO", "ROCHER", "ROCKS", "RODEO", "ROGERS",
    "ROOM", "RS", "RSVP", "RU", "RUGBY", "RUHR", "RUN", "RW", "RWE", "RYUKYU", "SA",
    "SAARLAND", "SAFE", "SAFETY", "SAKURA", "SALE", "SALON", "SAMSCLUB", "SAMSUNG",
    "SANDVIK", "SANDVIKCOROMANT", "SANOFI", "SAP", "SARL", "SAS", "SAVE", "SAXO",
    "SB", "SBI", "SBS", "SC", "SCA", "SCB", "SCHAEFFLER", "SCHMIDT", "SCHOLARSHIPS",
    "SCHOOL", "SCHULE", "SCHWARZ", "SCIENCE", "SCJOHNSON", "SCOR", "SCOT", "SD", "SE",
    "SEARCH", "SEAT", "SECURE", "SECURITY", "SEEK", "SELECT", "SENER", "SERVICES", "SES",
    "SEVEN", "SEW", "SEX", "SEXY", "SFR", "SG", "SH", "SHANGRILA", "SHARP", "SHAW", "SHELL",
    "SHIA", "SHIKSHA", "SHOES", "SHOP", "SHOPPING", "SHOUJI", "SHOW", "SHOWTIME", "SHRIRAM",
    "SI", "SILK", "SINA", "SINGLES", "SITE", "SJ", "SK", "SKI", "SKIN", "SKY", "SKYPE", "SL",
    "SLING", "SM", "SMART", "SMILE", "SN", "SNCF", "SO", "SOCCER", "SOCIAL", "SOFTBANK",
    "SOFTWARE", "SOHU", "SOLAR", "SOLUTIONS", "SONG", "SONY", "SOY", "SPACE", "SPORT",
    "SPOT", "SPREADBETTING", "SR", "SRL", "SRT", "SS", 
    "ST", "STADA", "STAPLES", "STAR", "STARHUB",
    "STATEBANK", "STATEFARM", "STC", "STCGROUP", "STOCKHOLM", "STORAGE", "STORE",
    "STREAM", "STUDIO", "STUDY", "STYLE", "SU", "SUCKS", "SUPPLIES", "SUPPLY", "SUPPORT",
    "SURF", "SURGERY", "SUZUKI", "SV", "SWATCH", "SWIFTCOVER", "SWISS", "SX", "SY", "SYDNEY",
    "SYMANTEC", "SYSTEMS", "SZ", "TAB", "TAIPEI", "TALK", "TAOBAO", "TARGET", "TATAMOTORS",
    "TATAR", "TATTOO", "TAX", "TAXI", "TC", "TCI", "TD", "TDK", "TEAM", "TECH", "TECHNOLOGY",
    "TEL", "TELEFONICA", "TEMASEK", "TENNIS", "TEVA", "TF", "TG", "TH", "THD",
    "THEATER", "THEATRE", "TIAA", "TICKETS", "TIENDA", "TIFFANY", "TIPS", "TIRES", "TIROL",
    "TJ", "TJMAXX", "TJX", "TK", "TKMAXX", "TL", "TM", "TMALL", "TN", "TO", "TODAY", "TOKYO",
    "TOOLS", "TOP", "TORAY", "TOSHIBA", "TOTAL", "TOURS", "TOWN", "TOYOTA", "TOYS", "TR",
    "TRADE", "TRADING", "TRAINING", "TRAVEL", "TRAVELCHANNEL", "TRAVELERS",
    "TRAVELERSINSURANCE", "TRUST", "TRV", "TT", "TUBE", "TUI", "TUNES", "TUSHU", "TV", "TVS",
    "TW", "TZ", "UA", "UBANK", "UBS", "UCONNECT", "UG", "UK", "UNICOM", "UNIVERSITY", "UNO",
    "UOL", "UPS", "US", "UY", "UZ", "VA", "VACATIONS", "VANA", "VANGUARD", "VC", "VE",
    "VEGAS", "VENTURES", "VERISIGN", "VERSICHERUNG", "VET", "VG", "VI", "VIAJES", "VIDEO",
    "VIG", "VIKING", "VILLAS", "VIN", "VIP", "VIRGIN", "VISA", "VISION",
    "VISTAPRINT", "VIVA", "VIVO", "VLAANDEREN", "VN", "VODKA", "VOLKSWAGEN", "VOLVO", "VOTE",
    "VOTING", "VOTO", "VOYAGE", "VU", "VUELOS", "WALES", "WALMART", "WALTER", "WANG",
    "WANGGOU", "WARMAN", "WATCH", "WATCHES", "WEATHER", "WEATHERCHANNEL", "WEBCAM", "WEBER",
    "WEBSITE", "WED", "WEDDING", "WEIBO", "WEIR", "WF", "WHOSWHO", "WIEN", "WIKI",
    "WILLIAMHILL", "WIN", "WINDOWS", "WINE", "WINNERS", "WME", "WOLTERSKLUWER", "WOODSIDE",
    "WORK", "WORKS", "WORLD", "WOW", "WS", "WTC", "WTF", "XBOX", "XEROX", "XFINITY",
    "XIHUAN", "XIN", "XN--11B4C3D", "XN--1CK2E1B", "XN--1QQW23A", "XN--2SCRJ9C",
    "XN--30RR7Y", "XN--3BST00M", "XN--3DS443G", "XN--3E0B707E", "XN--3HCRJ9C",
    "XN--3OQ18VL8PN36A", "XN--3PXU8K", "XN--42C2D9A", "XN--45BR5CYL", "XN--45BRJ9C",
    "XN--45Q11C", "XN--4GBRIM", "XN--54B7FTA0CC", "XN--55QW42G", "XN--55QX5D",
    "XN--5SU34J936BGSG", "XN--5TZM5G", "XN--6FRZ82G", "XN--6QQ986B3XL", "XN--80ADXHKS",
    "XN--80AO21A", "XN--80AQECDR1A", "XN--80ASEHDB", "XN--80ASWG", "XN--8Y0A063A",
    "XN--90A3AC", "XN--90AE", "XN--90AIS", "XN--9DBQ2A", "XN--9ET52U", "XN--9KRT00A",
    "XN--B4W605FERD", "XN--BCK1B9A5DRE4C", "XN--C1AVG", "XN--C2BR7G", "XN--CCK2B3B",
    "XN--CG4BKI", "XN--CLCHC0EA0B2G2A9GCD", "XN--CZR694B", "XN--CZRS0T", "XN--CZRU2D",
    "XN--D1ACJ3B", "XN--D1ALF", "XN--E1A4C", "XN--ECKVDTC9D", "XN--EFVY88H", "XN--ESTV75G",
    "XN--FCT429K", "XN--FHBEI", "XN--FIQ228C5HS", "XN--FIQ64B", "XN--FIQS8S", "XN--FIQZ9S",
    "XN--FJQ720A", "XN--FLW351E", "XN--FPCRJ9C3D", "XN--FZC2C9E2C", "XN--FZYS8D69UVGM",
    "XN--G2XX48C", "XN--GCKR3F0F", "XN--GECRJ9C", "XN--GK3AT1E", "XN--H2BREG3EVE",
    "XN--H2BRJ9C", "XN--H2BRJ9C8C", "XN--HXT814E", "XN--I1B6B1A6A2E", "XN--IMR513N",
    "XN--IO0A7I", "XN--J1AEF", "XN--J1AMH", "XN--J6W193G", "XN--JLQ61U9W7B", "XN--JVR189M",
    "XN--KCRX77D1X4A", "XN--KPRW13D", "XN--KPRY57D", "XN--KPU716F", "XN--KPUT3I",
    "XN--L1ACC", "XN--LGBBAT1AD8J", "XN--MGB9AWBF", "XN--MGBA3A3EJT", "XN--MGBA3A4F16A",
    "XN--MGBA7C0BBN0A", "XN--MGBAAKC7DVF", "XN--MGBAAM7A8H", "XN--MGBAB2BD",
    "XN--MGBAH1A3HJKRD",
    "XN--MGBAI9AZGQP6J", "XN--MGBAYH7GPA", "XN--MGBB9FBPOB", "XN--MGBBH1A", "XN--MGBBH1A71E",
    "XN--MGBC0A9AZCG", "XN--MGBCA7DZDO", "XN--MGBERP4A5D4AR", "XN--MGBGU82A",
    "XN--MGBI4ECEXP", "XN--MGBPL2FH", "XN--MGBT3DHD", "XN--MGBTX2B", "XN--MGBX4CD0AB",
    "XN--MIX891F", "XN--MK1BU44C", "XN--MXTQ1M", "XN--NGBC5AZD", "XN--NGBE9E0A", "XN--NGBRX",
    "XN--NODE", "XN--NQV7F", "XN--NQV7FS00EMA", "XN--NYQY26A", "XN--O3CW4H", "XN--OGBPF8FL", "XN--OTU796D",
    "XN--P1ACF", "XN--P1AI", "XN--PBT977C", "XN--PGBS0DH", "XN--PSSY2U", "XN--Q9JYB4C",
    "XN--QCKA1PMC", "XN--QXAM", "XN--RHQV96G", "XN--ROVU88B", "XN--RVC1E0AM3E",
    "XN--S9BRJ9C", "XN--SES554G", "XN--T60B56A", "XN--TCKWE", "XN--TIQ49XQYJ", "XN--UNUP4Y",
    "XN--VERMGENSBERATER-CTB", "XN--VERMGENSBERATUNG-PWB", "XN--VHQUV", "XN--VUQ861B",
    "XN--W4R85EL8FHU5DNRA", "XN--W4RS40L", "XN--WGBH1C", "XN--WGBL6A", "XN--XHQ521B",
    "XN--XKC2AL3HYE2A", "XN--XKC2DL3A5EE0H", "XN--Y9A3AQ", "XN--YFRO4I67O", "XN--YGBI2AMMX",
    "XN--ZFR164B", "XXX", "XYZ", "YACHTS", "YAHOO", "YAMAXUN", "YANDEX", "YE",
    "YODOBASHI", "YOGA", "YOKOHAMA", "YOU", "YOUTUBE", "YT", "YUN", "ZA", "ZAPPOS", "ZARA",
    "ZERO", "ZIP", "ZM", "ZONE", "ZUERICH", "ZW",

};

static uint32_t RegisteredTldNameNb = sizeof(RegisteredTldName) / sizeof(char const*);

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

        if (dnsstat_flags&dnsStateFlagCountQueryParms)
        {
            SubmitRegistryNumber(REGISTRY_DNS_Q_CLASSES, rrclass);
            SubmitRegistryNumber(REGISTRY_DNS_Q_RRType, rrtype);
        }

        if (dnsstat_flags&dnsStateFlagCountUnderlinedNames)
        {
            if (rrtype == DnsRtype_TXT)
            {
                SubmitRegistryString(REGISTRY_DNS_txt_underline, 3, (uint8_t *) "TXT");
                CheckForUnderline(packet, length, name_start);
            }
        }
    }
    else
    {
        error_flags |= DNS_REGISTRY_ERROR_FORMAT;
        start = length;
    }

    return start;
}

int DnsStats::SubmitRecord(uint8_t * packet, uint32_t length, uint32_t start, 
    uint32_t * e_rcode, uint32_t * e_length, bool is_response)
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
            if (ldata > 0 || rrtype == DnsRtype_OPT)
            {
                /* only record rrtypes and rrclass if valid response */
                if (rrtype != DnsRtype_OPT)
                {
                    if (is_response)
                    {
                        SubmitRegistryNumber(REGISTRY_DNS_CLASSES, rrclass);
                    }
                    else if (dnsstat_flags&dnsStateFlagCountQueryParms)
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
                else if (dnsstat_flags&dnsStateFlagCountQueryParms)
                {
                    SubmitRegistryNumber(REGISTRY_DNS_Q_RRType, rrtype);
                }

                /* For records of type RRSIG, NSEC, NSEC3, DNSKEY, DS,
                 * mark the domain as supporting DNSSEC */
                if (dnssec_name_index == 0 && (
                    rrtype == DnsRtype_DNSKEY ||
                    rrtype == DnsRtype_RRSIG ||
                    rrtype == DnsRtype_NSEC ||
                    rrtype == DnsRtype_NSEC3 || 
                    rrtype == DnsRtype_DS)) {
                    dnssec_name_index = name_start;
                }
             
                /* Further parsing for OPT, DNSKEY, RRSIG, DS,
                 * and maybe also AFSDB, NSEC3, DHCID, RSYNC types */
                switch (rrtype)
                {
                case (int)DnsRtype_OPT:
                    SubmitOPTRecord(ttl, &packet[start + 10], ldata, e_rcode);
                    break;
                case (int)DnsRtype_DNSKEY:
                    SubmitKeyRecord(&packet[start + 10], ldata);
                    break;
                case (int)DnsRtype_RRSIG:
                    SubmitRRSIGRecord(&packet[start + 10], ldata);
                    break;
                case (int)DnsRtype_DS:
                    SubmitDSRecord(&packet[start + 10], ldata);
                    break;
                case (int)DnsRtype_TLSA:
                    SubmitTLSARecord(&packet[start + 10], ldata);
                    break;
                default:
                    break;
                }
            }

            start += ldata + 10;
        }
    }

    return start;
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

    /* Process the flags and rcodes */
    if (e_rcode != NULL)
    {
        *e_rcode = (flags >> 24) & 0xFF;
    }

    /* Register that EDNS was used */
    is_using_edns = true;

    /* Register whether the DO bit was set */
    is_do_flag_set = (flags & (1<<15)) != 0;

    /* Add flags to registration */
    for (int i = 0; i < 16; i++)
    {
        if ((flags & (1 << i)) != 0)
        {
            SubmitRegistryNumber(REGISTRY_EDNS_Header_Flags, 15 - i);
        }
    }

    SubmitRegistryNumber(REGISTRY_EDNS_Version_number, (flags >> 16) & 0xFF);


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

            if (previous != 0)
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

int DnsStats::CompareDnsName(uint8_t * packet, uint32_t length, uint32_t start1, uint32_t start2)
{
    bool ret = false;

    while (start1 < length && start2 < length) {
        if (start1 == start2) {
            ret = true;
            break;
        } 
        else if ((packet[start1] & 0xC0) == 0xC0)
        {
            start1 = ((packet[start1] & 63) << 8) + packet[start1 + 1];
        }
        else if ((packet[start2] & 0xC0) == 0xC0)
        {
            start2 = ((packet[start2] & 63) << 8) + packet[start2 + 1];
        }
        else if (packet[start1] > 0x3f || packet[start2] > 0x3f)
        {
            break;
        }
        else if (packet[start1] != packet[start2])
        {
            break;
        }
        else
        {
            uint32_t l = packet[start1];
            start1++;
            start2++;

            if (l == 0)
            {
                ret = true;
                break;
            }
            else if (start1 + l > length || start2 + l >= length)
            {
                break;
            }
            else
            {
                bool cmp = true;
                for (uint32_t i = 0; cmp && i < l; i++) {
                    uint8_t c1 = packet[start1 + i];
                    uint8_t c2 = packet[start2 + i];

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

bool DnsStats::IsQNameMinimized(uint8_t * packet, uint32_t length, uint32_t nb_queries, int q_rclass, int q_rtype, uint32_t qr_index, uint32_t an_index, uint32_t ns_index)
{
    bool ret = false;
    uint32_t first_index = (an_index == 0) ? ns_index : an_index;

    if (nb_queries == 1 && qr_index != 0 && first_index != 0 && q_rclass == DnsRClass_IN &&
        (q_rtype == DnsRtype_A || q_rtype == DnsRtype_NS)) {
        ret = CompareDnsName(packet, length, qr_index, first_index);
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
            SubmitRegistryStringAndCount(registry_id,
                (uint32_t) lines[i]->tld_len, lines[i]->tld, lines[i]->count);
            export_count++;
        }
        else if (registry_id == REGISTRY_DNS_LeakedTLD)
        {
            /* Add count of leaks by length -- should replace by pattern match later */
            SubmitRegistryNumberAndCount(REGISTRY_DNS_LeakByLength, 
                (uint32_t) lines[i]->tld_len, lines[i]->count);
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

bool DnsStats::IsRfc6761Tld(uint8_t * tld, size_t length)
{
    bool ret = false;

    for (uint32_t i = 0; i < nb_rfc6771_tld; i++)
    {
        size_t j = 0;
        uint8_t * x = (uint8_t *)rfc6761_tld[i];
        bool match = true;

        for (; j < length; j++)
        {
            if (x[j] == 0 || (x[j] != tld[j] && (x[j] - 'A' + 'a') != tld[j]))
            {
                match = false;
                break;
            }
        }

        if (match && x[j] == 0 && j == length)
        {
            ret = true;
            break;
        }
    }
    return ret;
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
    *is_numeric = true;

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

bool DnsStats::LoadPcapFile(char const * fileName)
{
    bool ret = true;
    pcap_reader reader;
    size_t nb_records_read = 0;
    size_t nb_udp_dns_frag = 0;
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
            nb_records_read++;

            if (reader.tp_version == 17 &&
                (reader.tp_port1 == 53 || reader.tp_port2 == 53))
            {
                data_udp53 += reader.tp_length - 8;

                if (reader.is_fragment)
                {
                    nb_udp_dns_frag++;
                }
                else
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
    uint32_t e_rcode = 0;
    uint32_t qdcount = 0;
    uint32_t ancount = 0;
    uint32_t nscount = 0;
    uint32_t arcount = 0;
    uint32_t parse_index = 0;
    uint32_t e_length = 512;
    bool unfiltered = false;
    int query_rclass = 0;
    int query_rtype = 0;
    uint32_t first_query_index = 0;
    uint32_t first_answer_index = 0;
    uint32_t first_ns_index = 0;

    if (t_start_sec == 0 && t_start_usec == 0) {
        t_start_sec = ts.tv_sec;
        t_start_usec = ts.tv_usec;
    }
    else {
        int32_t delta_usec = ts.tv_usec - t_start_usec;
        int64_t delta_t = ts.tv_sec - t_start_sec;
        delta_t *= 1000000;
        delta_t += delta_usec;

        if (delta_t < 0) {
            t_start_sec = ts.tv_sec;
            t_start_usec = ts.tv_usec;
            duration_usec -= delta_t;
        }
        else if (delta_t > duration_usec) {
            duration_usec = delta_t;
        }
    }
    volume_53only += length;


    error_flags = 0;
    is_do_flag_set = false;
    is_using_edns = false;
    edns_options = NULL;
    edns_options_length = 0;
    is_qname_minimized = false;
    dnssec_name_index = 0;

    if (rootAddresses.GetCount() == 0)
    {
        rootAddresses.SetList(DefaultRootAddresses, sizeof(DefaultRootAddresses) / sizeof(char const *));
    }

    if (length < 12)
    {
        error_flags |= DNS_REGISTRY_ERROR_FORMAT;
        parse_index = length;
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

        SubmitRegistryNumber(REGISTRY_DNS_OpCodes, opcode);

        if (is_response && opcode == DNS_OPCODE_QUERY)
        {

            uint32_t tld_offset = 0;
            int nb_name_parts = 0;
            uint32_t previous_offset = 0;
            bool gotTld = GetTLD(packet, length, 12, &tld_offset, &previous_offset, &nb_name_parts);
            bool is_binary = false;
            bool is_bad_syntax = false;
            bool is_numeric = false;

            if (gotTld) {
                /* Verify that the TLD is valid, so as to exclude random traffic that would drown the stats */
                TldCheck(packet + tld_offset + 1, packet[tld_offset], &is_binary, &is_bad_syntax, &is_numeric);
                SetToUpperCase(packet + tld_offset + 1, packet[tld_offset]);
            }

            if (rootAddresses.IsInList(source_addr, source_addr_length))
            {
                /* Perform statistics on root traffic */

                SubmitRegistryNumber(REGISTRY_DNS_root_QR, rcode);

                if (gotTld)
                {
                    DnsStatsLeakType x_type = dnsLeakNoLeak;

                    if (rcode == DNS_RCODE_NXDOMAIN && packet[tld_offset] != 0)
                    {
                        /* Debug option, list all the erroneous addresses */
                        if (enable_erroneous_name_list) {
                            uint8_t name[1024];
                            size_t name_len = 0;

                            (void) GetDnsName(packet, length, 12, name, sizeof(name), &name_len);

                            if (name_len > 0) {
                                DnsStats::SetToUpperCase(name, name_len);
                                SubmitRegistryString(REGISTRY_DNS_ERRONEOUS_NAME_LIST, (uint32_t)name_len, name);
                            }
                        }
                        /* Analysis of domain leakage */
                        if (is_binary) {
                            SubmitRegistryNumber(REGISTRY_DNS_LEAK_BINARY, 0);
                            x_type = dnsLeakBinary;
                        }
                        else if (is_bad_syntax) {
                            SubmitRegistryNumber(REGISTRY_DNS_LEAK_SYNTAX, 0);
                            x_type = dnsLeakBadSyntax;
                        }
                        else if (is_numeric) {
                            if (IsIpv4Tld(packet, length, 12)) {
                                SubmitRegistryNumber(REGISTRY_DNS_LEAK_IPV4, 0);
                                x_type = dnsLeakIpv4;
                            }
                            else {
                                SubmitRegistryNumber(REGISTRY_DNS_LEAK_NUMERIC, 0);
                                x_type = dnsLeakNumeric;
                            }
                        }
                        else if (IsRfc6761Tld(packet + tld_offset + 1, packet[tld_offset]))
                        {
                            SubmitRegistryString(REGISTRY_DNS_RFC6761TLD, packet[tld_offset], packet + tld_offset + 1);
                            x_type = dnsLeakRfc6771;
                        }
                        else
                        {
                            /* Insert in leakage table */
                            TldAsKey key(packet + tld_offset + 1, packet[tld_offset]);
                            bool stored = false;
                            (void)tldLeakage.InsertOrAdd(&key, true, &stored);

                            x_type = (previous_offset == 0) ? dnsLeakSinglePart : dnsLeakMultiPart;

                            /* TODO: If full enough, remove the LRU, and account for it in the patterns catalog */
                            if (tldLeakage.GetCount() > max_tld_leakage_table_count)
                            {
                                TldAsKey * removed = tldLeakage.RemoveLRU();
                                if (removed != NULL)
                                {
                                    /* Add count of leaks by length -- should replace by pattern match later */
                                    SubmitRegistryNumber(REGISTRY_DNS_LeakByLength, (uint32_t) removed->tld_len);

                                    delete removed;
                                }
                            }

                            /* Insert the 2nd level name part */
                            uint8_t * key2_name;
                            uint8_t key2_length;
                            uint8_t should_keep = false;

                            if (previous_offset == 0) {
                                key2_name = (uint8_t *)"-- NONE --";
                                key2_length = 10;
                                should_keep = true;
                            }
                            else {
                                key2_name = packet + previous_offset + 1;
                                key2_length = packet[previous_offset];
                                if (IsNumericDomain(key2_name, key2_length)) {
                                    key2_name = (uint8_t *)"-- NUMBER --";
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
                                TldAsKey * removed = secondLdLeakage.RemoveLRU();
                                if (removed != NULL)
                                {
                                    TldAsKey key3((uint8_t *)"-- OTHERS --", 12);
                                    key3.count = removed->count;
                                    (void)secondLdLeakage.InsertOrAdd(&key3, true, &stored);
                                    delete removed;
                                }
                            }
                        }
                    }
                    else if (rcode == DNS_RCODE_NOERROR)
                    {
                        /* Analysis of useless traffic to the root */
                        TldAddressAsKey key(dest_addr, dest_addr_length, packet + tld_offset + 1, packet[tld_offset]);

                        if (queryUsage.GetCount() >= max_query_usage_count)
                        {
                            /* Table is full. Just keep counting the transactions that are present */
                            TldAddressAsKey * present = queryUsage.Retrieve(&key);
                            if (present != NULL)
                            {
                                present->count++;
                                SubmitRegistryNumber(REGISTRY_DNS_UsefulQueries, 0);
                            }
                        }
                        else
                        {
                            bool stored = false;
                            (void)queryUsage.InsertOrAdd(&key, true, &stored);

                            SubmitRegistryNumber(REGISTRY_DNS_UsefulQueries, (stored) ? 1 : 0);
                        }

                        if (dnsstat_flags&dnsStateFlagCountTld)
                        {
                            SubmitRegistryString(REGISTRY_TLD_response, packet[tld_offset], packet + tld_offset + 1);
                        }
                    }

                    if (enable_ip_address_report) {
                        uint8_t name[512];
                        size_t name_len = 0;

                        if (dest_addr_length == 4) {
#ifdef _WINDOWS
                            sprintf_s((char *)name, sizeof(name), "%d.%d.%d.%d/%d",
                                dest_addr[0], dest_addr[1], dest_addr[2], dest_addr[3], (int)x_type);
#else
                            sprintf((char *)name, "%d.%d.%d.%d/%d",
                                dest_addr[0], dest_addr[1], dest_addr[2], dest_addr[3], (int)x_type);
#endif
                        }
                        else if (dest_addr_length == 16) {
#ifdef _WINDOWS
                            sprintf_s((char *)name, sizeof(name), 
                                "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x/%d",
                                dest_addr[0], dest_addr[1], dest_addr[2], dest_addr[3], 
                                dest_addr[4], dest_addr[5], dest_addr[6], dest_addr[7],
                                dest_addr[8], dest_addr[9], dest_addr[10], dest_addr[11],
                                dest_addr[12], dest_addr[13], dest_addr[14], dest_addr[15],
                                (int)x_type);
#else
                            sprintf((char *)name,
                                "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x/%d",
                                dest_addr[0], dest_addr[1], dest_addr[2], dest_addr[3],
                                dest_addr[4], dest_addr[5], dest_addr[6], dest_addr[7],
                                dest_addr[8], dest_addr[9], dest_addr[10], dest_addr[11],
                                dest_addr[12], dest_addr[13], dest_addr[14], dest_addr[15],
                                (int)x_type);
#endif
                        }
                        name_len = strlen((char *)name);

                        if (name_len > 0) {
                            SubmitRegistryString(REGISTRY_DNS_ADDRESS_LIST, (uint32_t)name_len, name);
                        }
                    }
                }
            }
            else if (gotTld)
            {
                /* Perform statistics on user traffic */
                TldAsKey key(packet + tld_offset + 1, packet[tld_offset]);

                /* Check whether this TLD is in the registered list */
                if (registeredTld.GetCount() == 0)
                {
                    LoadRegisteredTLD_from_memory();
                }

                if (registeredTld.Retrieve(&key) != NULL)
                {
                    /* This is a registered TLD */
                    SubmitRegistryNumber(REGISTRY_DNS_TLD_Usage_Count, 1);
                    if ((dnsstat_flags&dnsStateFlagListTldUsed) != 0)
                    {
                        SubmitRegistryString(REGISTRY_DNS_Tld_Usage, packet[tld_offset], packet + tld_offset + 1);
                    }
                }
                else
                {
                    /* Keep a count */
                    SubmitRegistryNumber(REGISTRY_DNS_TLD_Usage_Count, 0);


                    /* Analysis of domain leakage */
                    if (IsRfc6761Tld(packet + tld_offset + 1, packet[tld_offset]))
                    {
                        SubmitRegistryString(REGISTRY_DNS_RFC6761_Usage, packet[tld_offset], packet + tld_offset + 1);
                    }
                    else
                    {
                        bool stored = false;



                        if (tldStringUsage.GetCount() >= max_tld_string_usage_count)
                        {
                            TldAsKey * removed = tldStringUsage.RemoveLRU();
                            if (removed != NULL)
                            {
                                delete removed;
                            }
                        }

                        if (is_binary) {
                            TldAsKey key2((uint8_t *)"-- BINARY --", 12);

                            tldStringUsage.InsertOrAdd(&key2, true, &stored);
                        }
                        else if (is_bad_syntax) {
                            TldAsKey key2((uint8_t *)"-- SYNTAX --", 12);

                            tldStringUsage.InsertOrAdd(&key2, true, &stored);
                        }
                        else if (is_numeric) {
                            if (IsIpv4Tld(packet, length, 12)) {
                                TldAsKey key2((uint8_t *)"-- IPV4 --", 10);

                                tldStringUsage.InsertOrAdd(&key2, true, &stored);
                            }
                            else {
                                TldAsKey key2((uint8_t *)"-- NUMBER --", 12);

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

        for (uint32_t i = 0; i < 7; i++)
        {
            if ((flags & (1 << i)) != 0)
            {
                SubmitRegistryNumber(REGISTRY_DNS_Header_Flags, i);
            }
        }

        parse_index = 12;
    }

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

    if (has_header && (dnsstat_flags&dnsStateFlagCountPacketSizes) != 0)
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

    SubmitRegistryNumber(REGISTRY_DNS_error_flag, error_flags);

    if (has_header && opcode == DNS_OPCODE_QUERY &&
        rcode == DNS_RCODE_NOERROR && error_flags == 0 )
    {
        /* Do not perform client statistic on root traffic, but do it
        * for all other sources of traffic */
        if (is_response) {
            /* if (!rootAddresses.IsInList(source_addr, source_addr_length)) */ {
                is_qname_minimized = IsQNameMinimized(packet, length, qdcount, query_rclass, query_rtype,
                    first_query_index, first_answer_index, first_ns_index);
                RegisterStatsByIp(dest_addr, dest_addr_length);

                if (is_do_flag_set) {
                    if (dnssec_name_index == 0) {
                        RegisterDnssecUsageByName(packet, length, 12, false);
                    }
                    else {
                        RegisterDnssecUsageByName(packet, length, dnssec_name_index, true);
                    }
                }
            }
        } else {
            uint32_t tld_offset = 0;
            int nb_name_parts = 0;
            uint32_t previous_offset = 0;
            bool gotTld = GetTLD(packet, length, 12, &tld_offset, &previous_offset, &nb_name_parts);

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
                        RegisterOptionsByIp(source_addr, source_addr_length);
                    }
                }
            }
        }
    } 
}

bool DnsStats::ExportToCaptureSummary(CaptureSummary * cs)
{
    DnsHashEntry *entry;

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

    /* Export the data */
    cs->Reserve(hashTable.GetCount()+1);

    /* Export the stored values */
    for (uint32_t i = 0; i < hashTable.GetSize(); i++)
    {
        entry = hashTable.GetEntry(i);

        while (entry != NULL)
        {
            CaptureLine line;

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
                char text[1024];
                size_t text_length = 0;
                bool previous_was_space = true; /* Cannot have space at beginning */

                /* escape any non printable character */
                for (uint32_t i = 0; i < entry->key_length && text_length+1 < sizeof(text); i++)
                {
                    int x = entry->key_value[i];
                    bool should_escape = false;

                    if (x > ' ' && x < 127 && x != '"' && x != ',' && x!= '"' && x != '\''
                        && (x != '=' || i > 0))
                    {
                        previous_was_space = false;
                    }
                    else if (x == ' ' && !previous_was_space && i != entry->key_length - 1)
                    {
                        /* Cannot have several spaces */
                        previous_was_space = true;
                    }
                    else
                    {
                        should_escape = true;
                        break;
                    }

                    if (should_escape) {
                        if (text_length + 5 < sizeof(text)) {
                            text[text_length++] = '\\';
                            text[text_length++] = '0' + (x/100);
                            text[text_length++] = '0' + (x%100)/10;
                            text[text_length++] = '0' + x%10;
                        }
                        else {
                            text[text_length++] = '!';
                        }
                    }
                    else {
                        text[text_length++] = (char) x;
                    }
                }

                memcpy(line.key_value, text, text_length);
                line.key_value[text_length] = 0;
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
    TldAsKey * ret = new TldAsKey(this->tld, this->tld_len);

    if (ret != NULL)
    {
        ret->count = count;
    }

    return ret;
}

void TldAsKey::Add(TldAsKey * key)
{
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


TldAddressAsKey::TldAddressAsKey(uint8_t * addr, size_t addr_len, uint8_t * tld, size_t tld_len)
    :
    HashNext(NULL),
    count(1),
    hash(0)
{
    if (addr_len > 16)
    {
        addr_len = 16;
    }

    memcpy(this->addr, addr, addr_len);
    this->addr_len = addr_len;

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
    TldAddressAsKey* ret = new TldAddressAsKey(addr, addr_len, tld, tld_len);

    if (ret != NULL)
    {
        ret->count = count;
    }

    return ret;
}

void TldAddressAsKey::Add(TldAddressAsKey * key)
{
    this->count += key->count;
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
        !is_qname_minimized);
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

void DnsStats::RegisterOptionsByIp(uint8_t * source_addr, size_t source_addr_length)
{
    StatsByIP x(source_addr, source_addr_length, false, false, false);
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

    StatsByIP x(source_addr, source_addr_length, false, false, false);
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

            key->domain = new char[domain_length+1];

            if (key->domain == NULL) {
                delete key;
                key = NULL;
            }
            else {
                memcpy(key->domain, domain, domain_length);
                domain[domain_length + 1] = 0;
            }
        }
    }

    return key;
}

void DomainEntry::Add(DomainEntry * key)
{
    count += key->count;
}
