# The "delay query" class keeps track of delays and repetitions for a CC/AS tuple

class delay_query_as:
    def __init__(self, query_cc, query_AS):
        self.query_cc = query_cc
        self.query_AS=query_AS
        self.late_repeats = 0
        self.nb_uids = 0
        self.nb_uids_repeated = 0
        self.nb_repeats = 0
        self.max_repeats = 0
        self.max_delay = 0
        self.first_repeat_slice = []

    def add_hit_at_index(self, delay_index):
        while delay_index >= len(self.first_repeat_slice):
            self.first_repeat_slice.append(0)
        self.first_repeat_slice[delay_index] += 1

    def compute_index(delay):
        delay_index = 0
        while delay > 0:
            if delay < 0.1:
                break
            elif delay < 0.2:
                delay_index += 1
                break
            elif delay < 0.5:
                delay_index += 2
                break
            else:
                delay_index += 3
                if delay_index == 15:
                    break
                delay /= 10
        return delay_index

    def tabulate(self, y):
        self.nb_uids += 1
        self.nb_repeats += y.nb_repeats
        if y.nb_repeats > 0:
            self.nb_uids_repeated += 1
            self.add_hit_at_index(delay_query_as.compute_index(y.first_delay))
        if y.nb_repeats > self.max_repeats:
            self.max_repeats = y.nb_repeats
        if y.max_delay > self.max_delay:
            self.max_delay =  y.max_delay

    def get_row_header():
        h = [
            "query_cc",
            "query_AS",
            "late_repeats",
            "nb_uids",
            "nb_uids_repeated",
            "nb_repeats",
            "max_repeats",
            "max_delay" ]

        h.append("<0.1")
        h.append("<0.2")
        h.append("<0.5")
        delay_index = 3
        delay = 1
        while delay_index < 15:
            h.append("<" + str(delay))
            h.append("<" + str(2*delay))
            h.append("<" + str(5*delay))
            delay *= 10
            delay_index += 3
        h.append(">= " + str(int(delay/2)))
        return h

    def get_row(self):
        r = [
            self.query_cc,
            self.query_AS,
            self.late_repeats,
            self.nb_uids,
            self.nb_uids_repeated,
            self.nb_repeats,
            self.max_repeats,
            self.max_delay ]
        r += self.first_repeat_slice
        for delay_index in range(len(self.first_repeat_slice), 16):
            r.append(0)
        return r

    def from_row(row):
        dqa = delay_query_as(row['query_cc'], row['query_AS'])
        
        dqa.late_repeats = row["late_repeats"]
        dqa.nb_uids = row["nb_uids"]
        dqa.nb_uids_repeated = row["nb_uids_repeated"]
        dqa.nb_repeats = row["nb_repeats"]
        dqa.max_repeats = row["max_repeats"]
        dqa.max_delay = row["max_delay"]
        slice_header = ["<0.1", "<0.2", "<0.5", "<1", "<2", "<5", "<10",
                             "<20", "<50", "<100", "<200", "<500", "<1000", "<2000", "<5000", ">= 5000" ]
        while len(slice_header) > len(dqa.first_repeat_slice):
            dqa.first_repeat_slice.append(0)
        for i in range(0, len(slice_header)):
            dqa.first_repeat_slice[i] = row[slice_header[i]]
        return dqa

    def add(self, other):
        self.late_repeats += other.late_repeats
        self.nb_uids += other.nb_uids
        self.nb_uids_repeated += other.nb_uids_repeated
        self.nb_repeats += other.nb_repeats
        if self.max_repeats < other.max_repeats:
            self.max_repeats = other.max_repeats
        if self.max_delay < other.max_delay:
            self.max_delay = other.max_delay
        for i in range(0, len(self.first_repeat_slice)):
            self.first_repeat_slice[i] += other.first_repeat_slice[i]

