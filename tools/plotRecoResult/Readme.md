# plotSimpleReco

## Description

`plotSimpleReco` is a validation tool for the SimpleReco reconstruction algorithm. It compares reconstructed vertex, direction, and energy quantities with the corresponding MC truth information stored in the ANNIE ROOT tree.

The tool reads a ROOT file containing SimpleReco branches and produces a multi-page PDF with reconstruction performance histograms.

Required branches:

* SimpleRecoX
* SimpleRecoY
* SimpleRecoZ
* SimpleRecoDirX
* SimpleRecoDirY
* SimpleRecoDirZ
* SimpleRecoEnergy
* SimpleRecoFlag
* SimpleRecoFV

Required MC truth information:

* True vertex position
* True muon direction
* True muon energy

## Output

A PDF file containing the following histograms:

1. RecoX − TrueX
2. RecoY − TrueY
3. RecoZ − TrueZ
4. RecoDirX − TrueDirX
5. RecoDirY − TrueDirY
6. RecoDirZ − TrueDirZ
7. Vertex distance:

   * |Reco Vertex − True Vertex|
8. Angular difference:

   * Angle between reconstructed and true directions
9. RecoEnergy − TrueMuonEnergy



## Notes

* True vertex coordinates are converted from tank-centered coordinates to global detector coordinates before residual calculations.
* Vertex residuals and vertex distance are reported in meters.
* Direction angle is calculated using the normalized dot product between reconstructed and true direction vectors.
* Energy residuals are reported in MeV.

