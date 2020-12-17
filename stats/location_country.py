#!/usr/bin/python
# coding=utf-8
#
# The "location_country" class computes statistics by country and location.



class location_country:
    def __init__(self):
        self.loc_cc = dict()

    def add_loc_cc_entry(self, loc, cc, nb_queries):
        loc_cc_key = cc + loc

        if loc_cc_key in self.loc_cc:
            self.loc_cc[loc_cc_key] += nb_queries
        else:
            self.loc_cc[loc_cc_key] = nb_queries

    def to_csv(self, csv_out):
        ret = True
        key_list = sorted(list(self.loc_cc.keys()))
        try:
            with open(csv_out,"wt") as wf:
                csv_head = "cc, Location, count"
                wf.write(csv_head + "\n")
                for key in key_list:
                    cc = key[:2]
                    loc = key[2:]
                    csv_string = cc + "," + loc + "," + str(self.loc_cc[loc_cc_key])
                    wf.write(csv_string + "\n")
        except:
            traceback.print_exc()
            print("Cannot produce csv file: " + csv_out)
            ret = False
        return ret
