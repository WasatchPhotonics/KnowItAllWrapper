#!/usr/bin/env python

# This script parses and aggregates the output of KIAConsole, producing a
# CSV report on each spectrum processed by Wiley KnowItAll.  At the
# end of the report is a summary table showing overall matching performance
# for each sample compound (based on the "truth" name inferred from the
# input filename).

import sys
import re

class Tally(object):
    def __init__(self):
        self.count = 0
        self.sec = 0
        self.matches = 0
        self.tops = 0
        self.positions = 0
        self.confidence = 0
        self.distractors = {}

class Analyzer(object):

    def __init__(self, pathname):
        self.pathname = pathname

        self.filename = None
        self.datapoints = None
        self.elapsed_sec = None
        self.matches = None
        self.confidence = None

        self.totals = {}

        self.synonyms = []
        self.synonyms.append(("Acetaminophen", "4-Acetamidophenol"))
        self.synonyms.append(("BMSB", "1,4-Bis(2-methylstyryl)benzene"))
        self.synonyms.append(("isopropanol", "2-propanol", "lsopropyl alcohol"))  # yes that seems to be in their database
        self.synonyms.append(("MEK", "2-Butanone"))

    def print_header(self):
        print("%s, %s, %s, %s, %s, %s, %s, %s, %s" % (
            "file", "sample", "datapoints", "seconds", "match_count", "matched", "top_match", "match_pos", "reported"))

    ##
    # Print an aggregate report with one row for each compound found within the 
    # input dataset.
    def report_totals(self):
        print("\n\nTotals")
        print("Sample, Count, Avg Time, Avg Matched, Avg Confidence, Avg Top Match, Avg Match Position, Top Distractor")
        for sample in sorted(self.totals):
            total = self.totals[sample]
            distractor = None if len(total.distractors) == 0 else max(total.distractors, key=total.distractors.get)
            print("%s, %d, %.2lf, %.2lf, %.2lf, %.2lf, %.2lf, %s" % (
                sample, 
                total.count, 
                total.sec       / total.count, 
                total.matches   / total.count,
                total.confidence/ total.count,
                total.tops      / total.count,
                total.positions / total.count,
                distractor))

    ## report whether either string appears within the other (applying some light
    # normalization first)
    def similar(self, a, b):
        a = re.sub(" ", "", a).lower()
        b = re.sub(" ", "", b).lower()
        if a in b or b in a:
            return True

    ## report whether the two compound names appear to match, using the "synonyms"
    # table to compare alternate names
    def equivalent(self, a, b):
        if self.similar(a, b):
            return True

        for syn in self.synonyms:
            for x_index in range(len(syn)):
                for y_index in range(len(syn)):
                    if x_index != y_index:
                        x = syn[x_index]
                        y = syn[y_index]
                        if self.similar(x, a) and self.similar(y, b):
                            return True

    ## print a single comma-delimited report row for a single measurement
    # processed by KIA
    def report(self):
        if self.matches is None:
            return

        m = re.search(r'([^\\/]+)-\d+\.csv', self.filename)
        if m:
            sample = m.group(1)
        else:
            sample = None

        match_pos = -1
        confidence = 0
        reported = None

        if sample is not None:
            for index, match in enumerate(self.matches):
                compound = match[0]
                confidence = match[1]
                if index == 0:
                    reported = compound
                if self.equivalent(sample, compound):
                    match_pos = index
                    break

        matched = match_pos >= 0
        top_match = matched and match_pos == 0
        match_count = len(self.matches)

        print("%s, %s, %d, %.2lf, %d, %s, %s, %d, %s" % (
            self.filename,
            sample,
            self.datapoints,
            self.elapsed_sec,
            match_count,
            matched,
            top_match,
            match_pos,
            reported))

        if not sample in self.totals:
            self.totals[sample] = Tally()

        total = self.totals[sample]
        total.count += 1
        total.sec += self.elapsed_sec
        if matched:
            total.matches += 1
            total.positions += match_pos
            total.confidence += confidence
        else:
            if reported not in total.distractors:
                total.distractors[reported] = 1
            else:
                total.distractors[reported] += 1
            
        if top_match:
            total.tops += 1

    ## Iterate through the KIAConsole output file, post-processing the
    # debug output messages and generating an aggregate table of matching 
    # performance
    def run(self):
        self.print_header()
        with open(self.pathname, encoding='ISO-8859-1') as f:
            for line in f:
                # strip timestamp if found
                m = re.search(r'\S{3} \S{3} \s*\d+ \s*\d+:\d+:\d+ \d{4} (.*)', line)
                if m:
                    line = m.group(1)

                m = re.search(r'loading (.*)', line)
                if m:
                    if self.matches is not None:
                        self.report()
                    self.filename = m.group(1)
                    self.matches = []
                    continue

                m = re.search(r'opening search with (\d+) datapoints', line, re.I)
                if m:
                    self.datapoints = int(m.group(1))
                    continue

                m = re.search(r'(\d+) matches found in ([.0-9]+) sec', line, re.I)
                if m:
                    self.match_count = int(m.group(1))
                    self.elapsed_sec = float(m.group(2))
                    continue

                m = re.search(r'Match \s*\d+: (.*) \(([.0-9]+)% confidence\)', line, re.I)
                if m:
                    compound = m.group(1)
                    confidence = float(m.group(2))
                    self.matches.append((compound, confidence))
                    continue
        self.report()
        self.report_totals()

if len(sys.argv) < 2:
    print("Usage: analyze-log log.txt")
    sys.exit(1)

analyzer = Analyzer(sys.argv[1])
analyzer.run()
