#!/usr/bin/env python

# A simple script to split ENLIGHTEN's "export" CSV files back into individual 
# column-ordered CSVs.  Requires ENLIGHTEN and Wasatch.PY source distributions.
# Names extracted spectra using the "label" row from the export.
#
# This script was created to generate an initial dataset for testing the Bio-Rad 
# Know-It-All SearchSDK.  It is not expected to be required in normal operations,
# and similar functionality is planned for addition to the main ENLIGHTEN GUI via
# a "re-save all" checkbox.

# invocation:
# $ export PYTHONPATH=~/work/code/enlighten:~/work/code/Wasatch.PY
# $ conda activate wasatch3
# $ conda install xlwt          # easier to patch wasatch3 than get conda_enlighten3 working on a Mac
# $ python split-spectra.py /path/to/*.csv

import re
import sys
import logging

from enlighten.ExportFileParser import ExportFileParser
from wasatch import applog

log = logging.getLogger(__name__)
logger = applog.MainLogger("DEBUG")

label_counts = {}

# process each file on the command-line
filenames = sys.argv[1:]
for filename in filenames:
    print("Processing %s" % filename)

    parser = ExportFileParser(filename, file_manager=None, save_options=None)
    measurements = parser.parse()
    print("  %d measurements parsed" % len(measurements))

    for m in measurements:
        label = m.label

        # normalize label
        label = re.sub(r'\d{4}[-_]*\d{2}[-_]*\d{2}[-_ ]\d{2}:\d{2}:\d{2}', '', label) # timestamps
        label = re.sub(r'Spectrum', '', label, re.I) 
        label = re.sub("\d+x\d+m?s", "", label)
        label = re.sub("/", "", label)
        label = re.sub(" ", "", label)
        label = label.strip()

        # append -NN suffix to label
        if label not in label_counts:
            label_counts[label] = 0
        label_counts[label] += 1

        # use label as new basename when saving
        m.basename = "%s-%02d" % (label, label_counts[label])

        print("  extracting %s" % m.basename)
        m.save_csv_file_by_column()

log.info(None)
logger.close()
applog.explicit_log_close()
