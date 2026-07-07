#include "MRDHit.h"

MRDHit::MRDHit(int clusterIndex, double t, double charge,
               int digitPMT, int detID, int chankey, int chankeyMC)
  : mClusterIndex(clusterIndex), mTime(t), mCharge(charge),
    mDigitPMT(digitPMT), mDetID(detID), mChankey(chankey), mChankeyMC(chankeyMC)
{}

void MRDHit::Print() const {
    std::cout << "MRDHit: cluster=" << mClusterIndex
              << " t=" << mTime << " ns"
              << " charge=" << mCharge
              << " digitPMT=" << mDigitPMT
              << " detID=" << mDetID
              << " chankey=" << mChankey
              << " chankeyMC=" << mChankeyMC
              << std::endl;
}
