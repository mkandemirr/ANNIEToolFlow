# filterCCEvent

## Overview

`filterCCEvent` selects CC-like neutrino events by applying a configurable set of selection cuts. All cuts are controlled through an INI configuration file and can be enabled or disabled independently.

## Output Modes

* **filtered** – writes only events passing all enabled cuts.
* **annotated** – writes all events and stores pass/fail flags for each enabled selection cut.

## Selection Cuts

A total of **16 configurable selection cuts** are available.

### MC-only (1)

* True CC

### Data-only (3)

* Beam OK
* BRF window
* Missing PPS tick

### Common to MC and data (12)

* LAPPD requirement
* Paired event
* Prompt PMT cluster
* High-quality PMT cluster
* MRD track reconstruction
* MRDReco track
* MRD side-exiting track
* MRD through-going track
* MRD stopping track
* Tank–MRD coincidence
* Muon topology
* No-veto


