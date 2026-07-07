# filterCCEvent

## Overview

`filterCCEvent` selects CC-like neutrino events by applying a configurable set of selection cuts. 
All cuts are controlled through an INI configuration file and can be enabled or disabled independently.

## Output Modes

* **filtered** – writes only events passing all enabled cuts.
* **annotated** – writes all events and stores pass/fail flags for each selection cut.

## Available Cuts

* True CC (MC only)
* LAPPD requirement
* Paired event
* Prompt PMT cluster
* High-quality PMT cluster
* MRD track reconstruction
* Tank–MRD coincidence
* Muon topology
* No-veto

