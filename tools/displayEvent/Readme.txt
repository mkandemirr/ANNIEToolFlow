============================================================
What this event display does
============================================================

This tool visualizes the detector response in time for each event in an ANNIEvent root file.

By setting time windows and thresholds (e.g. clusterTimeMin/Max, lappdHitTimeMin/Max, tankPMTsPEThreshold), you control both the time range of the detector response and the signal level (e.g. PE) that is emphasized.

The event display:

 - does NOT apply any cuts or filtering
 - shows all events in the tree sequentially

This means the typical workflow is:

1. First apply your physics selection (filter events using another tool)
2. Then use this event display to inspect selected events visually

Data and MC usage

The tool works for both real data and Monte Carlo (MC).

Use separate configuration files for data and MC (config.ini and mcConfig.ini)

Note: This event display is designed for ROOT files with the ANNIEEventTree structure.
The input file must follow the same tree/branch structure to be compatible.

# ============================================================
# Event display quantities ((top-left panel))
# ============================================================

# Tank cluster time:
#   Among clusters in [clusterTimeMin, clusterTimeMax],
#   the one with the highest PE is selected.
#   The displayed "Tank cluster time" corresponds to this cluster.
#
#   If no cluster exists in this time window:
#   "No cluster in [min, max] ns" is shown.


# MRD cluster time:
#   Among MRD clusters with time difference Δt (MRD - Tank)
#   within [dtMRDPMTMin, dtMRDPMTMax],
#   the one closest to dtMRDPMTHistPeak is selected.
#   The displayed "MRD cluster time" corresponds to this cluster.
#
#   If no time-correlated MRD cluster exists:
#   "No correlated cluster" is shown.


# MRD display window:
#   From the selected MRD cluster, hit times are examined and
#   the densest time window of width mrdDisplayWindow is found.
#   The displayed range (min–max) corresponds to this window.
#
#   If no MRD cluster exists:
#   no meaningful window can be defined (default values are used).


# Δt_{MRD,PMT}:
#   Time difference between MRD cluster and Tank cluster:
#   Δt = t_MRD - t_Tank
#   The displayed range shows the accepted time window used in selection.


# Δt_{FMV,PMT}:
#   Time difference between FMV hit and Tank cluster:
#   Δt = t_FMV - t_Tank
#   The displayed range shows the accepted time window used in selection.


# PMTs triggered:
#   Number of PMTs in the selected Tank cluster with
#   PE >= tankPMTsPEThreshold.
#   Only PMTs above this threshold are colored.


# LAPPDs triggered:
#   Number of LAPPDs with hit count > lappdHitThreshold
#   within [lappdHitTimeMin, lappdHitTimeMax].
#   Only LAPPDs above this threshold are colored.
