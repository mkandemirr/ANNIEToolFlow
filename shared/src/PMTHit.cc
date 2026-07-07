// PMTHit.cpp
#include "PMTHit.h"


PMTHit::PMTHit(double x, double y, double z, double t, double q, double pe,
               int type, int detID, int chankey, int chankeyMC)
  : mX(x), mY(y), mZ(z), mT(t), mQ(q), mPE(pe),
    mType(type), mDetID(detID), mChankey(chankey), mChankeyMC(chankeyMC)
{}

void PMTHit::Print() const {
    std::cout << "PMTHit\n"
              << " Filtered: "   << mIsFiltered << "\n"
              << " Position: ("  << mX << ", " << mY << ", " << mZ << ")\n"
              << " Time: "       << mT << "\n"
              << " Charge: "     << mQ << "\n"
              << " PE: "         << mPE << "\n"
              << " Type: "       << mType << "\n"
              << " DetID: "      << mDetID << "\n"
              << " Chankey: "    << mChankey << "\n"
              << " ChankeyMC: "  << mChankeyMC << "\n"
              << std::endl;
}

