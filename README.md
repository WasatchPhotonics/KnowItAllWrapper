# Overview

A simple command-line DOS program to exercise the Know-It-All ID-SDK Raman API from VC++.

This was adapted from the ID-SDK "sample code" posted here:

- https://info.bio-rad.com/KnowItAllSDK.html

The goal will be to extend this to make it easy to call from ENLIGHTEN via Python.

# API

See vendor docs here:

- https://info.bio-rad.com/KnowItAllSDK.html

Also see [SearchSDK.h](KIAConsole/SearchSDK.h) notes.

# Invocation

## Identify sample spectra

This runs an input dataset against the KIA SearchSDK matching algorithm and
outputs match results to console.

    $ cd data\good
    $ ..\..\KIAConsole\Debug\KIAConsole.exe > test.log

## Aggregate analysis of identification results

This runs a simple script to compare the captured match results against "known 
truth" data (assumed to be in spectrum filename), generating a CSV that can be 
readily viewed in Excel.

    $ python ..\..\scripts\analyze-log.py test.log > summary.csv

# References

- [KnowItAll QuickStart](http://www.bio-rad.com/webroot/web/pdf/spectroscopy/global/english/literature/docs/280076-KnowItAll_Quick_Start_Guide_English.pdf)
- [Building Custom Compound Databases](https://www.youtube.com/watch?v=rZ7ZhyrOLEg)

# Backlog

- add command-line options to specify max matches and min confidence
- process CSV files listed on cmd-line
- pick safer delimiter than @ (could be used in concentrations)

# History

- 2019-06-19 0.2.0
    - simplified text output a bit to help with parsing
    - successful preliminary integration into ENLIGHTEN 1.6.14
- 2019-06-19 0.1.3
    - added --streaming and --directory
- 2019-06-05 0.1.2
    - added split-spectra
- 2019-06-04 0.1.1
    - fixed negative wavenumber import
- 2019-06-04 0.1.0
    - added analyze-log script
    - added input data
- 2019-06-04 0.0.2
    - runs
    - added timestamped logging
- 2019-05-29 0.0.1
    - builds and runs (haven't tested against spectra)
