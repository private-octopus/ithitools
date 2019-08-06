
#!/usr/bin/python
# coding=utf-8
#
# This scripts tries to estimate the variance of a few M3 test metrics.

import codecs
import sys
import m3name

class metric_item:
    def __init__(self, name):
        self.name = name
        self.nb_measures = 0
        self.ratio_sum = 0.0
        self.ratio_s2 = 0.0
        self.sum_x = 0.0
        self.sum_x2 = 0.0
        self.sum_y = 0.0
        self.sum_y2 = 0.0
        self.metric = 0.0
        self.ratio = 0.0
        self.v_ratio = 0.0
        self.v_metric = 0.0
        self.v_est = 0.0
        self.v_est_2 = 0.0
     
    def metric_line(self):
        s = self.name + ","
        s += str(self.nb_measures) + ","
        s += str(self.ratio_sum) + ","
        s += str(self.ratio_s2) + ","
        s += str(self.sum_x) + ","
        s += str(self.sum_x2) + ","
        s += str(self.sum_y) + ","
        s += str(self.sum_y2)
        return s

    def variance_line(self):
        s = self.name + ","
        s += str(self.metric) + ","
        s += str(self.ratio) + ","
        s += str(self.v_ratio) + ","
        s += str(self.v_metric) + ","
        s += str(self.v_est) + ","
        s += str(self.v_est_2)
        return s

    def add_instance(self, x, y):
        ratio = 0.0
        ratio += x
        ratio /= y
        self.nb_measures += 1
        self.ratio_sum += ratio
        self.ratio_s2 += ratio*ratio
        self.sum_x += x
        self.sum_x2 += x*x
        self.sum_y += y
        self.sum_y2 += y*y

    def compute_variance(self):
        self.metric = self.sum_x/self.sum_y
        self.ratio = self.ratio_sum / self.nb_measures
        self.v_ratio = (self.ratio_s2 / self.nb_measures) - self.ratio*self.ratio
        e_x = self.sum_x / self.nb_measures
        v_x = self.sum_x2 / self.nb_measures - e_x*e_x
        e_y = self.sum_y / self.nb_measures
        v_y = self.sum_y2 / self.nb_measures - e_y*e_y
        v_correction = (1.0 + v_y/(e_y*e_y))
        self.v_metric = self.v_ratio * v_correction
        self.v_est = v_x / (e_y*e_y)
        self.v_est_2 = self.v_est * v_correction

class capture_line:
    def __init__(self):
        self.name = ""
        self.index_type = 0
        self.index_num = 0
        self.index_string = ""
        self.count = 0

    def load(self, m_line):
        ret = 0
        try:
            m_line = m_line.strip()
            cells = m_line.split(",")
            if (len(cells) < 4):
                ret = 0
            else:
                self.name = cells[0].strip()
                self.index_type = int(cells[1], base=10)
                if (self.index_type == 0):
                    self.index_num = int(cells[2], base=10)
                else:
                    self.index_string = cells[2].strip()
                self.count = int(cells[3].strip())
        except:
            ret = -1
        return ret


class capture_file:
    def __init__(self):
        self.list = []

    def load(self, file_name):
        try:
            self.list = []
            m_file = codecs.open(file_name, "r", "UTF-8")
        except:
            e = sys.exc_info()[0]
            print("Cannot open: " + file_name)
            print ("Error: " + str(e) + "\n")
            return -1
        for m_line in m_file:
            c_line = capture_line()
            if (c_line.load(m_line) == 0):
                self.list.append(c_line)
        m_file.close()
        return 0

    def find(self, index_name, index_type, index_num, index_string):
        for c_line in self.list:
            if (c_line.name == index_name and c_line.index_type == index_type):
                if ((c_line.index_type == 0 and c_line.index_num == index_num) or
                    (c_line.index_type == 1 and c_line.index_string == index_string)):
                    return c_line.count
        return 0

    def findtotal(self, index_name):
        sum = 0
        for c_line in self.list:
            if (c_line.name == index_name):
                sum += c_line.count
        return sum

def load_m3(file_name, metric_list, sum_m3):
    capture = capture_file()
    if (capture.load(file_name) != 0):
        return -1
    c0 = capture.find("root-QR", 0, 0, "")
    c1 = capture.find("root-QR", 0, 3, "")
    nb_queries = c0 + c1
    for m in metric_list:
        if (m.name == "M3.1"):
            if (nb_queries > 250000):
                m.add_instance(c1, nb_queries)
        elif (m.name == "M3.3.2.HOME"):
            c_tld_home = capture.find("LeakedTLD", 1, 0, "HOME")
            if (nb_queries > 250000):
                m.add_instance(c_tld_home, nb_queries)
        elif (m.name == "M3.3.2.CORP"):
            c_tld_corp = capture.find("LeakedTLD", 1, 0, "CORP") 
            if (nb_queries > 250000):
                m.add_instance(c_tld_corp, nb_queries)
        elif (m.name == "M3.3.2.MAIL"):
            c_tld_mail = capture.find("LeakedTLD", 1, 0, "MAIL")
            if (nb_queries > 250000):
                m.add_instance(c_tld_mail, nb_queries)      
    m3n = m3name.m3name()
    country_code = "??"
    city_code = "???"
    m3_date = "????????"
    m3_hour = "??????"
    duration = 0
    address_id = "????"
    if m3n.parse_file_id(file_name) != 0:
        country_code = m3n.country_code
        city_code = m3n.city_code
        m3_date = m3n.m3_date
        m3_hour = m3n.m3_hour
        duration = m3n.duration
        address_id = m3n.address_id

    sum_m3.write(str(address_id) + "," +
                 country_code + "," + city_code + "," + 
                 m3_date + "," + m3_hour + "," +
                 str(duration) + "," + str(nb_queries) + "," + 
                 str(c1) + "," + str(c_tld_home) + "," +
                 str(c_tld_corp) + "," + str(c_tld_mail) + "," +
                 file_name + ",\n")
    return 0

#self test functions
def capture_line_test():
    test_line = ["CLASS,0,1,10490435900", "Frequent-TLD-usage,1,ALARMSERVER,197,"]
    test_name = ["CLASS", "Frequent-TLD-usage"]
    test_type = [0, 1]
    test_num = [1, 0]
    test_string = ["", "ALARMSERVER"]
    test_count = [10490435900, 197]
    i = 0
    while (i < len(test_line)):
        result = 0
        cl = capture_line()
        if (cl.load(test_line[i]) != 0):
            print("Error! cannot load <" + test_line[i] + ">\n")
            return(-1)
        if (cl.name != test_name[i]):
            print("For <" + test_line[i] + "> got name = <" + cl.name + ">\n")
        elif (cl.index_type != test_type[i]):
            print("For <" + test_line[i] + "> got type = <" + str(cl.index_type) + ">\n")
        elif (cl.index_type == 0 and cl.index_num != test_num[i]):
            print("For <" + test_line[i] + "> got num = <" + str(cl.index_num) + ">\n")
        elif (cl.index_type == 1 and cl.index_string != test_string[i]):
            print("For <" + test_line[i] + "> got string = <" + cl.index_string + ">\n")
        elif (cl.count != test_count[i]):
            print("For <" + test_line[i] + "> got count = <" + str(cl.count) + ">\n")
        else:
            result = 1
        if (result != 1):
            print("Error!")
            return(-1)
        i += 1
    return(0)

def capture_test(file_name, nb_lines):
    capture = capture_file()
    if (capture.load(file_name) != 0):
        print("Error: Cannot load <" + file_name + ">\n") 
        return -1
    if (len(capture.list) != nb_lines):
        print("Error: In <" + file_name + "> found <" + str(len(capture.list)) + ">\n")
        return -1
    return 0

def m3_test(file_name, metric_list, m3_test_sum):
    for x in [1, 2]:
        if (load_m3(file_name, metric_list, m3_test_sum) != 0):
            print("Error: Cannot load <" + file_name + ">\n") 
            return -1
        for m_line in metric_list:
            print(m_line.metric_line() + "\n")
    return 0

def metric_test():
    m_line = metric_item("test")
    m_line.add_instance(12, 100)
    m_line.add_instance(30, 200)
    print(m_line.metric_line() + "\n")
    m_line.compute_variance()
    print(m_line.variance_line() + "\n")

# Main program
# Load a list of files from argv[1], and for each file compute the
# list of metrics and their contribution to variances. Then,
# compute the final values of the metrics and variances.

metric_names = ["M3.1", "M3.3.2.HOME", "M3.3.2.CORP", "M3.3.2.MAIL"]
metric_list = []
for item_name in metric_names: 
    metric_list.append(metric_item(item_name));

if len(sys.argv) >= 4 and sys.argv[1] == "!":
    # perform the self test.
    ret = m3name.m3name_test()
    if ret == 0:
        print("m3name_test passes")
    else:
        print("m3name_test fails")
    if ret == 0:
        ret = capture_line_test()
        if (ret == 0):
            print("Capture line test passes.\n")
            ret = capture_test(sys.argv[2], int(sys.argv[3]))
            if (ret == 0):
                print("Capture file test passes.\n")
                m3_test_sum = codecs.open("test_sum_f3.csv", "w", "UTF-8")
                ret = m3_test(sys.argv[2], metric_list, m3_test_sum)
                if (ret == 0):
                    print("M3 file test passes.\n")
                    metric_test()
                m3_test_sum.close()

    exit(ret)

name_sum_f3 = "sum_f3.csv"
if len(sys.argv) == 3:
    name_sum_f3 = sys.argv[2]
elif len(sys.argv) != 2:
    print("Usage: " + sys.argv[0] + " <file-with-list-of-captures>\n")
    exit(1)

file_m3 = codecs.open(sys.argv[1], "r", "UTF-8")
sum_m3 = codecs.open(name_sum_f3, "w", "UTF-8")

for line in file_m3:
    try:
        line = line.strip()
        load_m3(line, metric_list, sum_m3)
    except:
        e = sys.exc_info()[0]
        print ( "Error" + str(e) + "\n")
sum_m3.close()

for m_line in metric_list:
    print(m_line.metric_line() + "\n")
for m_line in metric_list:
    m_line.compute_variance()
    print(m_line.variance_line() + "\n")