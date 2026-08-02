# Simple script to extract a set of candidate dates
import datetime
import os
import sys

def date_of_day(year, month, day):
    iso_day = str(year) + "-" + str(month) + "-" + str(day)
    d = datetime.date.fromisoformat(iso_day)
    return d

def getNdays(end_date, N):
    d = end_date
    nd = [ d ]
    for x in range(1, N):
        d -= datetime.timedelta(days=1)
        nd.append(d)
    return nd

def month_folder(prefix, d):
    yf = os.path.join(prefix, str(d.year))
    mf = os.path.join(yf, "{:02d}".format(d.month))
    return mf

def day_folder(prefix, d):
    mf = month_folder(prefix, d)

    df = os.path.join(mf,  "{:02d}".format(d.day))
    return df

#
#
#d = date_of_day(sys.argv[1], sys.argv[2], sys.argv[3])
#print(d)
#nd = getNdays(d, 15)
#for x in nd:
#    print(x)
#    print(month_folder("result", x))
#    print(day_folder("data", x))
