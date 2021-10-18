# Overview

A simple command-line DOS program to exercise the Know-It-All ID-SDK Raman API from VC++.

This was adapted from the ID-SDK "sample code" posted here:

- https://sciencesolutions.wiley.com/knowitall-sdk/

The goal will be to extend this to make it easy to call from ENLIGHTEN via Python.

# Dependencies

- Wiley KnowItAll
    - https://get.knowitall.com 

# API

- see [SearchSDK.h](KIAConsole/SearchSDK.h) notes.

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

# Backlog

- add command-line options to specify max matches and min confidence
- process CSV files listed on cmd-line
- pick safer delimiter than @ (could be used in concentrations)

# History

- 2021-10-18 1.0.2
    - docs
- 2020-03-25 1.0.1
    - rebuilt under new Visual Studio
- 2019-10-11 1.0.0
    - report valid count instead of all matches
- 2019-06-19 0.5.0
    - added start/end markers
    - added quit
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
