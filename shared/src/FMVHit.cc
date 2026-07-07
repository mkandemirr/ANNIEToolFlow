// FMVHit.cpp
#include "FMVHit.h"

FMVHit::FMVHit(double t, int detID, int chankey, int chankeyMC)
  : mTime(t), mDetID(detID), mChankey(chankey), mChankeyMC(chankeyMC) {}

void FMVHit::Print() const {
    std::cout << "FMVHit: t=" << mTime
              << " detID=" << mDetID
              << " chankey=" << mChankey
              << " chankeyMC=" << mChankeyMC << std::endl;
}
