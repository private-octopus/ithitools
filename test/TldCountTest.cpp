/*
* Author: Christian Huitema
* Copyright (c) 2018, Private Octopus, Inc.
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
#include <algorithm>
#include <vector>
#include "DnsStats.h"
#include "TldCountTest.h"



TldCountTest::TldCountTest()
{
}


TldCountTest::~TldCountTest()
{
}

static char const * TargetNames[] = {
    "AAA", "AARP", "ABARTH", "ABB", "ABBOTT", "ABBVIE", "ABC", "ABLE", "ABOGADO",
    "ABUDHABI", "AC", "ACADEMY", "ACCENTURE", "ACCOUNTANT", "ACCOUNTANTS", "ACO",
    "ACTIVE", "ACTOR", "AD", "ADAC", "ADS", "ADULT", "AE", "AEG", "AERO", "AETNA",
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
    "BING", "BINGO", "BIO", "BIZ", "BJ", "BLACK", "BLACKFRIDAY", "BLANCO", "BLOCKBUSTER",
    "BLOG", "BLOOMBERG", "BLUE", "BM", "BMS", "BMW", "BN", "BNL", "BNPPARIBAS", "BO", "BOATS",
    "BOEHRINGER", "BOFA", "BOM", "BOND", "BOO", "BOOK", "BOOKING", "BOOTS", "BOSCH", "BOSTIK",
    "BOSTON", "BOT", "BOUTIQUE", "BOX", "BR", "BRADESCO", "BRIDGESTONE", "BROADWAY", "BROKER",
    "BROTHER", "BRUSSELS", "BS", "BT", "BUDAPEST", "BUGATTI", "BUILD", "BUILDERS", "BUSINESS",
    "BUY", "BUZZ", "BV", "BW", "BY", "BZ", "BZH", "CA", "CAB", "CAFE", "CAL", "CALL",
    "CALVINKLEIN", "CAM", "CAMERA", "CAMP", "CANCERRESEARCH", "CANON", "CAPETOWN", "CAPITAL",
    "CAPITALONE", "CAR", "CARAVAN", "CARDS", "CARE", "CAREER", "CAREERS", "CARS", "CARTIER",
    "CASA", "CASE", "CASEIH", "CASH", "CASINO", "CAT", "CATERING", "CATHOLIC", "CBA", "CBN",
    "CBRE", "CBS", "CC", "CD", "CEB", "CENTER", "CEO", "CERN", "CF", "CFA", "CFD", "CG", "CH",
    "CHANEL", "CHANNEL", "CHASE", "CHAT", "CHEAP", "CHINTAI", "CHRISTMAS", "CHROME", "CHRYSLER",
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
    "DO", "DOCS", "DOCTOR", "DODGE", "DOG", "DOHA", "DOMAINS", "DOT", "DOWNLOAD", "DRIVE",
    "DTV", "DUBAI", "DUCK", "DUNLOP", "DUNS", "DUPONT", "DURBAN", "DVAG", "DVR", "DZ",
    "EARTH", "EAT", "EC", "ECO", "EDEKA", "EDU", "EDUCATION", "EE", "EG", "EMAIL", "EMERCK",
    "ENERGY", "ENGINEER", "ENGINEERING", "ENTERPRISES", "EPOST", "EPSON", "EQUIPMENT", "ER",
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
    "GOLD", "GOLDPOINT", "GOLF", "GOO", "GOODHANDS", "GOODYEAR", "GOOG", "GOOGLE", "GOP",
    "GOT", "GOV", "GP", "GQ", "GR", "GRAINGER", "GRAPHICS", "GRATIS", "GREEN", "GRIPE",
    "GROCERY", "GROUP", "GS", "GT", "GU", "GUARDIAN", "GUCCI", "GUGE", "GUIDE", "GUITARS",
    "GURU", "GW", "GY", "HAIR", "HAMBURG", "HANGOUT", "HAUS", "HBO", "HDFC", "HDFCBANK",
    "HEALTH", "HEALTHCARE", "HELP", "HELSINKI", "HERE", "HERMES", "HGTV", "HIPHOP",
    "HISAMITSU", "HITACHI", "HIV", "HK", "HKT", "HM", "HN", "HOCKEY", "HOLDINGS", "HOLIDAY",
    "HOMEDEPOT", "HOMEGOODS", "HOMES", "HOMESENSE", "HONDA", "HONEYWELL", "HORSE",
    "HOSPITAL", "HOST", "HOSTING", "HOT", "HOTELES", "HOTELS", "HOTMAIL", "HOUSE", "HOW",
    "HR", "HSBC", "HT", "HTC", "HU", "HUGHES", "HYATT", "HYUNDAI", "IBM", "ICBC", "ICE",
    "ICU", "ID", "IE", "IEEE", "IFM", "IKANO", "IL", "IM", "IMAMAT", "IMDB", "IMMO",
    "IMMOBILIEN", "IN", "INDUSTRIES", "INFINITI", "INFO", "ING", "INK", "INSTITUTE",
    "INSURANCE", "INSURE", "INT", "INTEL", "INTERNATIONAL", "INTUIT", "INVESTMENTS", "IO",
    "IPIRANGA", "IQ", "IR", "IRISH", "IS", "ISELECT", "ISMAILI", "IST", "ISTANBUL", "IT",
    "ITAU", "ITV", "IVECO", "IWC", "JAGUAR", "JAVA", "JCB", "JCP", "JE", "JEEP", "JETZT",
    "JEWELRY", "JIO", "JLC", "JLL", "JM", "JMP", "JNJ", "JO", "JOBS", "JOBURG", "JOT", "JOY",
    "JP", "JPMORGAN", "JPRS", "JUEGOS", "JUNIPER", "KAUFEN", "KDDI", "KE", "KERRYHOTELS",
    "KERRYLOGISTICS", "KERRYPROPERTIES", "KFH", "KG", "KH", "KI", "KIA", "KIM", "KINDER",
    "KINDLE", "KITCHEN", "KIWI", "KM", "KN", "KOELN", "KOMATSU", "KOSHER", "KP", "KPMG",
    "KPN", "KR", "KRD", "KRED", "KUOKGROUP", "KW", "KY", "KYOTO", "KZ", "LA", "LACAIXA",
    "LADBROKES", "LAMBORGHINI", "LAMER", "LANCASTER", "LANCIA", "LANCOME", "LAND",
    "LANDROVER", "LANXESS", "LASALLE", "LAT", "LATINO", "LATROBE", "LAW", "LAWYER", "LB",
    "LC", "LDS", "LEASE", "LECLERC", "LEFRAK", "LEGAL", "LEGO", "LEXUS", "LGBT", "LI",
    "LIAISON", "LIDL", "LIFE", "LIFEINSURANCE", "LIFESTYLE", "LIGHTING", "LIKE", "LILLY",
    "LIMITED", "LIMO", "LINCOLN", "LINDE", "LINK", "LIPSY", "LIVE", "LIVING", "LIXIL", "LK",
    "LOAN", "LOANS", "LOCKER", "LOCUS", "LOFT", "LOL", "LONDON", "LOTTE", "LOTTO", "LOVE",
    "LPL", "LPLFINANCIAL", "LR", "LS", "LT", "LTD", "LTDA", "LU", "LUNDBECK", "LUPIN",
    "LUXE", "LUXURY", "LV", "LY", "MA", "MACYS", "MADRID", "MAIF", "MAISON", "MAKEUP", "MAN",
    "MANAGEMENT", "MANGO", "MAP", "MARKET", "MARKETING", "MARKETS", "MARRIOTT", "MARSHALLS",
    "MASERATI", "MATTEL", "MBA", "MC", "MCKINSEY", "MD", "ME", "MED", "MEDIA", "MEET",
    "MELBOURNE", "MEME", "MEMORIAL", "MEN", "MENU", "MEO", "MERCKMSD", "METLIFE", "MG", "MH",
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
    "PANASONIC", "PANERAI", "PARIS", "PARS", "PARTNERS", "PARTS", "PARTY", "PASSAGENS",
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
    "SANDVIK", "SANDVIKCOROMANT", "SANOFI", "SAP", "SAPO", "SARL", "SAS", "SAVE", "SAXO",
    "SB", "SBI", "SBS", "SC", "SCA", "SCB", "SCHAEFFLER", "SCHMIDT", "SCHOLARSHIPS",
    "SCHOOL", "SCHULE", "SCHWARZ", "SCIENCE", "SCJOHNSON", "SCOR", "SCOT", "SD", "SE",
    "SEARCH", "SEAT", "SECURE", "SECURITY", "SEEK", "SELECT", "SENER", "SERVICES", "SES",
    "SEVEN", "SEW", "SEX", "SEXY", "SFR", "SG", "SH", "SHANGRILA", "SHARP", "SHAW", "SHELL",
    "SHIA", "SHIKSHA", "SHOES", "SHOP", "SHOPPING", "SHOUJI", "SHOW", "SHOWTIME", "SHRIRAM",
    "SI", "SILK", "SINA", "SINGLES", "SITE", "SJ", "SK", "SKI", "SKIN", "SKY", "SKYPE", "SL",
    "SLING", "SM", "SMART", "SMILE", "SN", "SNCF", "SO", "SOCCER", "SOCIAL", "SOFTBANK",
    "SOFTWARE", "SOHU", "SOLAR", "SOLUTIONS", "SONG", "SONY", "SOY", "SPACE", "SPIEGEL",
    "SPOT", "SPREADBETTING", "SR", "SRL", "SRT", "ST", "STADA", "STAPLES", "STAR", "STARHUB",
    "STATEBANK", "STATEFARM", "STATOIL", "STC", "STCGROUP", "STOCKHOLM", "STORAGE", "STORE",
    "STREAM", "STUDIO", "STUDY", "STYLE", "SU", "SUCKS", "SUPPLIES", "SUPPLY", "SUPPORT",
    "SURF", "SURGERY", "SUZUKI", "SV", "SWATCH", "SWIFTCOVER", "SWISS", "SX", "SY", "SYDNEY",
    "SYMANTEC", "SYSTEMS", "SZ", "TAB", "TAIPEI", "TALK", "TAOBAO", "TARGET", "TATAMOTORS",
    "TATAR", "TATTOO", "TAX", "TAXI", "TC", "TCI", "TD", "TDK", "TEAM", "TECH", "TECHNOLOGY",
    "TEL", "TELECITY", "TELEFONICA", "TEMASEK", "TENNIS", "TEVA", "TF", "TG", "TH", "THD",
    "THEATER", "THEATRE", "TIAA", "TICKETS", "TIENDA", "TIFFANY", "TIPS", "TIRES", "TIROL",
    "TJ", "TJMAXX", "TJX", "TK", "TKMAXX", "TL", "TM", "TMALL", "TN", "TO", "TODAY", "TOKYO",
    "TOOLS", "TOP", "TORAY", "TOSHIBA", "TOTAL", "TOURS", "TOWN", "TOYOTA", "TOYS", "TR",
    "TRADE", "TRADING", "TRAINING", "TRAVEL", "TRAVELCHANNEL", "TRAVELERS",
    "TRAVELERSINSURANCE", "TRUST", "TRV", "TT", "TUBE", "TUI", "TUNES", "TUSHU", "TV", "TVS",
    "TW", "TZ", "UA", "UBANK", "UBS", "UCONNECT", "UG", "UK", "UNICOM", "UNIVERSITY", "UNO",
    "UOL", "UPS", "US", "UY", "UZ", "VA", "VACATIONS", "VANA", "VANGUARD", "VC", "VE",
    "VEGAS", "VENTURES", "VERISIGN", "VERSICHERUNG", "VET", "VG", "VI", "VIAJES", "VIDEO",
    "VIG", "VIKING", "VILLAS", "VIN", "VIP", "VIRGIN", "VISA", "VISION", "VISTA",
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
    "XN--MGBAI9AZGQP6J", "XN--MGBAYH7GPA", "XN--MGBB9FBPOB", "XN--MGBBH1A", "XN--MGBBH1A71E",
    "XN--MGBC0A9AZCG", "XN--MGBCA7DZDO", "XN--MGBERP4A5D4AR", "XN--MGBGU82A",
    "XN--MGBI4ECEXP", "XN--MGBPL2FH", "XN--MGBT3DHD", "XN--MGBTX2B", "XN--MGBX4CD0AB",
    "XN--MIX891F", "XN--MK1BU44C", "XN--MXTQ1M", "XN--NGBC5AZD", "XN--NGBE9E0A", "XN--NGBRX",
    "XN--NODE", "XN--NQV7F", "XN--NQV7FS00EMA", "XN--NYQY26A", "XN--O3CW4H", "XN--OGBPF8FL",
    "XN--P1ACF", "XN--P1AI", "XN--PBT977C", "XN--PGBS0DH", "XN--PSSY2U", "XN--Q9JYB4C",
    "XN--QCKA1PMC", "XN--QXAM", "XN--RHQV96G", "XN--ROVU88B", "XN--RVC1E0AM3E",
    "XN--S9BRJ9C", "XN--SES554G", "XN--T60B56A", "XN--TCKWE", "XN--TIQ49XQYJ", "XN--UNUP4Y",
    "XN--VERMGENSBERATER-CTB", "XN--VERMGENSBERATUNG-PWB", "XN--VHQUV", "XN--VUQ861B",
    "XN--W4R85EL8FHU5DNRA", "XN--W4RS40L", "XN--WGBH1C", "XN--WGBL6A", "XN--XHQ521B",
    "XN--XKC2AL3HYE2A", "XN--XKC2DL3A5EE0H", "XN--Y9A3AQ", "XN--YFRO4I67O", "XN--YGBI2AMMX",
    "XN--ZFR164B", "XPERIA", "XXX", "XYZ", "YACHTS", "YAHOO", "YAMAXUN", "YANDEX", "YE",
    "YODOBASHI", "YOGA", "YOKOHAMA", "YOU", "YOUTUBE", "YT", "YUN", "ZA", "ZAPPOS", "ZARA",
    "ZERO", "ZIP", "ZIPPO", "ZM", "ZONE", "ZUERICH", "ZW",
};

static const size_t nbTargetNames = sizeof(TargetNames) / sizeof(char const *);

typedef struct st_tld_count_test_hash_input_t {
    int registry_id;
    int key_type;
    char const * key_text;
    int key_number;
    int count;
} tld_count_test_hash_input_t;

static tld_count_test_hash_input_t hash_input_table[] = {
    {REGISTRY_DNS_CLASSES, 0, NULL, 1, 19282420},
    { REGISTRY_DNS_CLASSES,0,NULL,3,38 },
    { REGISTRY_DNSSEC_Algorithm_Numbers,0,NULL,5,83454 },
    { REGISTRY_DNSSEC_Algorithm_Numbers,0,NULL,7,11587 },
    { REGISTRY_DNSSEC_Algorithm_Numbers,0,NULL,8,359955 },
    { REGISTRY_DNSSEC_Algorithm_Numbers,0,NULL,10,943 },
    { REGISTRY_DNSSEC_Algorithm_Numbers,0,NULL,13,217 },
    { REGISTRY_EDNS_Header_Flags,0,NULL,0,8632252 },
    { REGISTRY_EDNS_Header_Flags,0,NULL,8,1 },
    { REGISTRY_EDNS_Header_Flags,0,NULL,9,2 },
    { REGISTRY_EDNS_OPT_CODE,0,NULL,3,1 },
    { REGISTRY_EDNS_OPT_CODE,0,NULL,8,77504 },
    { REGISTRY_EDNS_OPT_CODE,0,NULL,9,1 },
    { REGISTRY_EDNS_OPT_CODE,0,NULL,10,129653 },
    { REGISTRY_EDNS_OPT_CODE,0,NULL,100,2 },
    { REGISTRY_EDNS_OPT_CODE,0,NULL,65001,80 },
    { REGISTRY_EDNS_Version_number,0,NULL,0,8729785 },
    { REGISTRY_EDNS_Version_number,0,NULL,1,2 },
    { REGISTRY_DNS_Header_Flags,0,NULL,4,8433638 },
    { REGISTRY_DNS_Header_Flags,0,NULL,5,15393 },
    { REGISTRY_DNS_Header_Flags,0,NULL,6,4550902 },
    { REGISTRY_DNS_LabelType,0,NULL,0,7352191 },
    { REGISTRY_DNS_LabelType,0,NULL,192,16862187 },
    { REGISTRY_DNS_OpCodes,0,NULL,0,17250000 },
    { REGISTRY_DNS_RCODES,0,NULL,0,11802316 },
    { REGISTRY_DNS_RCODES,0,NULL,1,1 },
    { REGISTRY_DNS_RCODES,0,NULL,2,400287 },
    { REGISTRY_DNS_RCODES,0,NULL,3,3560937 },
    { REGISTRY_DNS_RCODES,0,NULL,5,1486457 },
    { REGISTRY_DNS_RCODES,0,NULL,16,2 },
    { REGISTRY_DNS_RFC6761_Usage,1,"LOCAL",0,16 },
    { REGISTRY_DNS_RRType,0,NULL,1,4382558 },
    { REGISTRY_DNS_RRType,0,NULL,2,6487796 },
    { REGISTRY_DNS_RRType,0,NULL,5,174867 },
    { REGISTRY_DNS_RRType,0,NULL,6,4431681 },
    { REGISTRY_DNS_RRType,0,NULL,12,456825 },
    { REGISTRY_DNS_RRType,0,NULL,15,1583235 },
    { REGISTRY_DNS_RRType,0,NULL,16,2321 },
    { REGISTRY_DNS_RRType,0,NULL,28,1104162 },
    { REGISTRY_DNS_RRType,0,NULL,41,4362255 },
    { REGISTRY_DNS_RRType,0,NULL,43,12832 },
    { REGISTRY_DNS_RRType,0,NULL,46,409021 },
    { REGISTRY_DNS_RRType,0,NULL,47,175187 },
    { REGISTRY_DNS_RRType,0,NULL,48,34303 },
    { REGISTRY_DNS_RRType,0,NULL,50,27670 },
    { REGISTRY_DNS_TLD_Usage_Count,0,NULL,1,8573279 },
    { REGISTRY_DNS_UsefulQueries,0,NULL,0,19 },
    { REGISTRY_DNS_UsefulQueries,0,NULL,1,8 },
    { REGISTRY_DNS_error_flag,0,NULL,0,17249987 },
    { REGISTRY_DNS_error_flag,0,NULL,256,1 },
    { REGISTRY_DNS_error_flag,0,NULL,384,12 },
    { REGISTRY_DNS_root_QR,0,NULL,0,0 }
};

static const size_t nb_hash_input_table = sizeof(hash_input_table) / sizeof(tld_count_test_hash_input_t);

static void fill_initial_table(BinHash<DnsHashEntry> * hashTable) {
    for (size_t i = 0; i < nb_hash_input_table; i++) {
        DnsHashEntry key;
        bool stored = false;

        key.count = hash_input_table[i].count;
        key.registry_id = hash_input_table[i].registry_id;
        if (hash_input_table[i].key_type == 0) {
            key.key_length = sizeof(uint32_t);
            key.key_type = 0; /* number */
            key.key_number = hash_input_table[i].key_number;
        } else {
            key.key_length = (uint32_t)strlen(hash_input_table[i].key_text);
            key.key_type = 1; /* string */
            memcpy(key.key_value, hash_input_table[i].key_text, key.key_length);
            key.key_value[key.key_length] = 0;
        }

        (void)hashTable->InsertOrAdd(&key, true, &stored);
    }
}


static uint32_t xorshift_state = 0xdeadbeef;

static uint32_t xorshift32()
{
    /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
    uint32_t x = xorshift_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    xorshift_state = x;
    return x;
}

static uint32_t random_uniform(uint32_t range_max)
{
    uint32_t delta = 0xFFFFFFFF % range_max;
    uint32_t rnd;

    do {
        rnd = xorshift32();
    } while (rnd < delta);

    return rnd % range_max;
}

#define RANDOM_TLD_LENGTH 16

static char * random_tld(char n[RANDOM_TLD_LENGTH + 1])
{
    for (int i = 0; i < RANDOM_TLD_LENGTH; i++) {
        n[i] = 'A' + random_uniform(26);
    }
    n[RANDOM_TLD_LENGTH] = 0;

    return n;
}

bool TldCountTest::DoTest()
{
    LruHash<TldAsKey> tldStringUsage;
    BinHash<DnsHashEntry> hashTable;
    uint64_t count_per_string[nbTargetNames+1];
    const uint64_t initial_count = 0x10000;
    const uint32_t max_hash_size = 0x8000;
    const int max_delta_count = 0x80;
    const uint32_t max_tld_leakage_count = 0x80;
    uint64_t current_count = initial_count;
    uint64_t total_keys = 0;
    uint64_t dropped_keys = 0;
    uint64_t kept_keys = 0;
    uint64_t expected_count = 0;
    uint64_t hashed_count = 0;
    uint64_t hashed_total = 0;
    bool ret = true;
    size_t * rand_table = NULL;

    fill_initial_table(&hashTable);

    for (size_t i = 1; i <= nbTargetNames; i++) {
        total_keys += current_count;
        if (i <= max_tld_leakage_count)
        {
            expected_count += current_count;
        }
        count_per_string[i] = current_count;
        if (current_count >= 2) {
            current_count >>= 1;
        }
    }

    count_per_string[0] = 3*total_keys;
    total_keys *= 4;

    /* Simulate random arrivals */
    rand_table = new size_t[(size_t)total_keys];

    if (rand_table == NULL) {
        ret = false;
        TEST_LOG("Tld count test, cannot allocate rand_table[%d]\n", total_keys);
    } else {
        uint64_t table_rank = 0;
        /* Build a list of ID with desired size and repetition */
        for (size_t i = 0; i <= nbTargetNames; i++) {
            for (uint64_t c = 0; c < count_per_string[i] && table_rank < total_keys; c++) {
                rand_table[table_rank++] = i;
            }
        }

        /* Random shuffle of the list to simulate random arrivals */
        for (uint64_t r = total_keys - 1; r >= 1; r--) {
            uint32_t x = random_uniform((uint32_t)(r + 1));

            if (x != r) {
                size_t rtx = rand_table[x];
                rand_table[x] = rand_table[r];
                rand_table[r] = rtx;
            }
        }

        /* Simulate insertion */
        for (uint64_t i = 0; i < total_keys; i++)
        {
            bool stored = false;
            char rand_name[RANDOM_TLD_LENGTH + 1];
            char const * tld = (rand_table[i] == 0)? random_tld(rand_name): TargetNames[rand_table[i]-1];
            TldAsKey key((uint8_t *)tld, strlen(tld));
            TldAsKey * inserted = NULL;
            DnsHashEntry counter_entry;

            counter_entry.registry_id = REGISTRY_DNS_TLD_Usage_Count;
            counter_entry.key_type = 0;
            counter_entry.key_length = sizeof(uint32_t);
            counter_entry.key_number = 0;
            counter_entry.count = 1;

            (void)hashTable.InsertOrAdd(&counter_entry, true, &stored);

            if (tldStringUsage.GetCount() >= max_hash_size)
            {
                TldAsKey * removed = tldStringUsage.RemoveLRU();
                if (removed != NULL)
                {
                    dropped_keys += removed->count;
                    delete removed;
                }
            }
            inserted = tldStringUsage.InsertOrAdd(&key, true, &stored);
            if (inserted != NULL && rand_table[i] > 0 && inserted->count > count_per_string[rand_table[i]]) {
                TEST_LOG("TLD %s, count %d instead of %d\n",
                    TargetNames[rand_table[i] - 1], (int)inserted->count, (int)count_per_string[rand_table[i]]);
            }
        }

        /* Simulate extraction */
        if (tldStringUsage.GetCount() > 0) {
            TldAsKey *tld_entry;
            std::vector<TldAsKey *> lines(tldStringUsage.GetCount());
            int vector_index = 0;
            uint32_t export_count = 0;


            for (uint32_t i = 0; i < tldStringUsage.GetSize(); i++)
            {
                tld_entry = tldStringUsage.GetEntry(i);

                while (tld_entry != NULL)
                {
                    int tld_index = GetTldIndex(tld_entry->tld, tld_entry->tld_len);
                    if (tld_index >= 0) {
                        if (((uint32_t)tld_index) < max_tld_leakage_count &&
                            count_per_string[tld_index + 1] != tld_entry->count) {
                            TEST_LOG("TLD %s, count %d instead of %d\n",
                                TargetNames[tld_index], (int)tld_entry->count, (int)count_per_string[tld_index + 1]);
                            if (tld_entry->count > count_per_string[tld_index + 1]) {
                                ret = false;
                            }
                        }
                    }

                    lines[vector_index] = tld_entry;
                    vector_index++;
                    tld_entry = tld_entry->HashNext;
                }
            }

            std::sort(lines.begin(), lines.end(), TldAsKey::CompareTldEntries);

            /* Retain the N most interesting values */
            for (size_t i = 0; i < lines.size(); i++)
            {
                if (export_count < max_tld_leakage_count)
                {
                    kept_keys += lines[i]->count;
                    export_count++;
                    int tld_index = GetTldIndex(lines[i]->tld, lines[i]->tld_len);
                    DnsHashEntry key;
                    bool stored = false;

                    key.count = lines[i]->count;
                    key.registry_id = REGISTRY_DNS_Frequent_TLD_Usage;
                    key.key_length = (uint32_t)lines[i]->tld_len;
                    key.key_type = 1; /* string */
                    memcpy(key.key_value, lines[i]->tld, lines[i]->tld_len);
                    key.key_value[lines[i]->tld_len] = 0;

                    (void)hashTable.InsertOrAdd(&key, true, &stored);

                    if (tld_index < 0) {
                        if (lines[i]->count > 1) {
                            TEST_LOG("rank %d, random name, length %d, count %d\n",
                                (int)i, lines[i]->tld_len, (int)lines[i]->count);
                        }
                    } else {
                        if (count_per_string[tld_index + 1] != lines[i]->count) {
                            TEST_LOG("rank %d, %s, count %d instead of %d\n",
                                (int)i, TargetNames[tld_index], (int)lines[i]->count, (int)count_per_string[tld_index + 1]);
                            if (lines[i]->count > count_per_string[tld_index + 1]) {
                                ret = false;
                            }
                        }
                    }
                }
                else
                {
                    dropped_keys += lines[i]->count;
                }
            }
        }

        /* Verify count */
        ret &= ((kept_keys + dropped_keys) == total_keys);

        if (!ret) {
            TEST_LOG("Tld count test, kept_keys + dropped_keys: %d + %d = %d != total: %d\n",
                (int)kept_keys, (int)dropped_keys, (int)kept_keys + (int)dropped_keys, (int)total_keys);
        } else {
            int64_t delta_count = expected_count - kept_keys;

            ret = (delta_count <= max_delta_count) && (-delta_count <= max_delta_count);
            if (!ret) {
                TEST_LOG("Tld count test, expected_count: %d >> kept_keys: %d\n",
                    (int)expected_count, (int)kept_keys);
            }
        }

        if (ret){
            /* Verify the values from the hash table */
            DnsHashEntry *entry;

            for (uint32_t i = 0; i < hashTable.GetSize(); i++)
            {
                entry = hashTable.GetEntry(i);

                while (entry != NULL)
                {
                    if (entry->registry_id == REGISTRY_DNS_Frequent_TLD_Usage) {
                        hashed_count += entry->count;
                    }
                    else if (entry->registry_id == REGISTRY_DNS_TLD_Usage_Count) {
                        hashed_total = entry->count;
                    }

                    entry = entry->HashNext;
                }
            }

            if (hashed_count != kept_keys) {
                TEST_LOG("Tld sum count after hash: %d  != kept_keys: %d\n",
                    hashed_count, kept_keys);
                ret = false;
            } else if (hashed_total != total_keys) {
                TEST_LOG("Tld total from hash: %d  != total_keys: %d\n",
                    hashed_total, total_keys);
                ret = false;
            }
        }

        delete[] rand_table;
    }
    return ret;
}

int TldCountTest::GetTldIndex(uint8_t * tld, size_t tld_len)
{
    int low = -1;
    int high = nbTargetNames;
    int middle;
    int found = -1;

    while (found == -1 && low < high) {
        uint8_t * r;
        size_t r_len;
        int cmp = 0;

        middle = low + (high - low) / 2;

        if (middle == low || middle == high) {
            break;
        }

        r = (uint8_t *) TargetNames[middle];
        r_len = strlen(TargetNames[middle]);


        for (size_t i = 0; cmp == 0 &&  i < tld_len; i++) {
            if (i > r_len || tld[i] > r[i]) {
                cmp = 1; /* Tld is below middle */
            }
            else if (tld[i] < r[i]) {
                cmp = -1; /* Tld is above middle */
            }
        }
        if (cmp == 0 && r_len > tld_len) {
            cmp = -1; /* name is longer, tld is below it  */
        }

        switch (cmp) {
        case -1:
            high = middle;
            break;
        case 0:
            found = middle;
            break;
        case 1:
            low = middle;
            break;
        }
    }

    return found;
}
