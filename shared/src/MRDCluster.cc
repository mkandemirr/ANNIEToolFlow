#include "MRDCluster.h"
#include <iostream>

void MRDCluster::Print() const {
    std::cout << "MRDCluster: time=" << mTime
              << " sigma=" << mTimeSigma
              << " hits=" << mHitNumber
              << std::endl;
    for (const auto& h : mHits) h.Print();
}
