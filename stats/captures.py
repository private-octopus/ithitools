
#!/usr/bin/python
# coding=utf-8
#
# This scripts tries to estimate the variance of a few M3 test metrics.
import sys
import codecs

# load a line of an ITHI captue file
class capture_line:
    def __init__(self):
        self.name = ""
        self.index_type = 0
        self.index_num = 0
        self.index_string = ""
        self.count = 0

    def load_filtered(self, m_line, load_all):
        ret = 0
        try:
            m_line = m_line.strip()
            cells = m_line.split(",")
            if (len(cells) < 4):
                print("Too few cells:" + str(len(cells)))
                ret = -1
            else:
                self.name = cells[0].strip()
                if not load_all and (self.name == "ADDRESS_DELAY" or self.name == "ADDRESS_LIST" or self.name == "FULL_NAME_LIST"):
                    ret = -1
                else:
                    self.index_type = int(cells[1], base=10)
                    if (self.index_type == 0):
                        self.index_num = int(cells[2], base=10)
                    else:
                        self.index_string = cells[2].strip()
                    self.count = int(cells[3].strip())
        except Exception as e:
            print("Fail: " + str(e))
            print("error parsing: <" + m_line + ">")
            ret = -1
        return ret

    def load(self, m_line):
        return self.load_filtered(m_line, False)

# load an ITHI capture file in memory
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