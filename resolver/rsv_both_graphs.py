# Try to understand the split of traffic between Public DNS resolvers
#
# The first step is to split traffic by UID, and separate 3 kinds:
#
# - UIDs for which all none of the queries come from Public DNS resolvers
# - UIDs for which all the traffic comes from Public DNS resolvers
# - UIDs for which the traffic comes from both Public DNS resolvers and non public resolvers.
#
# The "public resolver" focus comes from the assumption that no traffic
# comes from third party resolvers that are not "public resolvers". Some traffic
# does come from "other AS in the same country" and "resolvers in a different country".
# At first sight, most "other AS in the same country" come prom ASes under the
# same authority as the ISP. very few traffic comes from "another country", and
# that too seem to come from ASes under the control of a multinational ISP.
# We might refine that hypothesis later.
#
# Once we have that data, we will focus on the share of traffic that comes from
# the 3rd category, "both ISP and Public DNS Resolvers". We want to analyse the
# timing of that traffic. For each category (ISP, or each public resolver), we
# will compute the delay between the first response to the query and the response
# in that category. We can plot a point cloud, using different colors per category,
# with X axis = delay and Y axis = some identifier of the query -- maybe the
# time of arrival of the first response.
#
# We use existing code that compute a PPQ panda frame as:
# 
# [ 'query_AS',  'query_user_id', 'rr_type',  'first_tag', 'first_time', ['Same_AS', 'Same_CC',
#  'Others', 'googlepdns', 'cloudflare', 'opendns', 'quad9', 'level3', 'neustar', 'he'] ]

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

cat_list = ['Same_AS', 'Same_CC', 'Others', 'googlepdns', 'cloudflare', 'opendns', 'quad9', 'level3', 'neustar', 'he']
color_list = [ 'blue', 'magenta', 'indigo', 'green', 'orange', 'red', 'violet', 'yellow', 'yellow', 'yellow', 'yellow' ]
dot_headers = [ 'rsv_type', 'rank', 'first_time', 'delay' ]

class per_as_analysis:
    def __init__(self, asn, dfdt):
        self.asn = asn
        self.asn_df = dfdt[dfdt['query_AS'] == asn]
        self.dots = []
        self.n_isp = 0
        self.n_public = 0
        self.n_both = 0
        self.total = 0
        self.sums = []
        for rsv in cat_list:
            self.sums.append(0)

    def process_uuid(self, x):
        has_isp = False
        has_public = False
        for rsv in cat_list[0:3]:
            dt = x[rsv]
            if dt < 1000000:
                has_isp = True
                break
        for rsv in cat_list[3:]:
            dt = x[rsv]
            if dt < 1000000:
                has_public = True
                break
        if has_public and has_isp:
            self.n_both += 1
            for rsv in cat_list:
                dt = x[rsv]
                if dt < 1000000:
                    dot_line = [ rsv, self.n_both,  x['first_time'], dt ]
                    self.dots.append(dot_line)
        elif has_public:
            self.n_public += 1
        else:
            self.n_isp += 1
        for i in range(0, len(cat_list)):
            rsv = cat_list[i]
            nb = 0
            dt = x[rsv]
            if dt < 1000000:
                nb = 1
                self.sums[i] += 1
            self.total += nb

    def compute_both(self):
        self.asn_df.apply(lambda x: self.process_uuid(x), axis=1)

    def list_headers():
        h = [ 'query_as',
              'total',
              'n_isp',
              'n_public',
              'n_both' ]
        for n in cat_list:
            h.append(n)
        return h

    def to_list(self):
        l = [ self.asn,
             self.total,
             self.n_isp,
             self.n_public,
             self.n_both]
        for r in self.sums:
            l.append(r)
        return l

    def do_graph(self, image_file="", x_delay=False, log_y=False):
        # get a frame from the list 
        dot_df = pd.DataFrame(self.dots,columns=dot_headers)
        is_first = True
        sub_df = []
        x_value = "rank"
        if x_delay:
            x_value = "first_time"

        for rsv in cat_list:
            sub_df.append(dot_df[dot_df['rsv_type'] == rsv])

        legend_list = []
        for i in range(0, len(cat_list)):
            rsv = cat_list[i]
            rsv_color = color_list[i]
            if len(sub_df[i]) > 0:
                if is_first:
                    axa = sub_df[i].plot.scatter(x=x_value, y="delay", logy=log_y, alpha=0.25, color=rsv_color)
                else:
                    sub_df[i].plot.scatter(ax=axa, x=x_value, y="delay", logy=log_y, alpha=0.25, color=rsv_color)
                is_first = False
                legend_list.append(rsv)
        plt.title("Delay (seconds) per provider for " + self.asn)
        plt.legend(legend_list)
        if len(image_file) == 0:
            plt.show()
        else:
            plt.savefig(image_file)

    
    def do_hist(self, image_file=""):
        # get a frame from the list 
        dot_df = pd.DataFrame(self.dots,columns=dot_headers)
        is_first = True
        clrs = []
        legend_list = []
        row_list = []
        x_min = 1000000
        x_max = 0.00001

        for i in range(0, len(cat_list)):
            rsv = cat_list[i]
            sdf_all = dot_df[dot_df['rsv_type'] == rsv]
            sdf = sdf_all['delay']
            sdf_max = sdf.max()
            if sdf_max > x_max:
                x_max = sdf_max
            sdf_min = sdf.min()
            if sdf_min < x_min:
                x_min = sdf_min
            l = sdf.values.tolist()
            if len(l) > 0:
                row_list.append(np.array(l))
                clrs.append(color_list[i])
                legend_list.append(rsv)
                is_first = False

        if x_min == 0:
            x_min = 0.00001

        if not is_first:
            logbins = np.logspace(np.log10(x_min),np.log10(x_max), num=20)
            axa = plt.hist(row_list, logbins, histtype='bar', color=clrs)
            plt.title("Histogram of delays (seconds) per provider for " + self.asn)
            plt.legend(legend_list)  
            plt.xscale('log')
            if len(image_file) == 0:
                plt.show()
            else:
                plt.savefig(image_file)