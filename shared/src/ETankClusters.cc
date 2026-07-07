// ETankClusters.cpp
#include "ETankClusters.h"
#include <iostream>

void ETankClusters::AddCluster(const TankCluster& cluster) {
    mClusters.push_back(cluster);
}

void ETankClusters::Clear() {
    mClusters.clear();
}


void ETankClusters::Print() const {
    std::cout << "== Tank Clusters ==" << std::endl;
    std::cout << "Number of tank clusters in this event: " << mClusters.size() << std::endl;
    
    for (size_t i = 0; i < mClusters.size(); ++i) {
        std::cout << "Cluster " << i + 1 << ":\n";
        mClusters[i].Print();
    }
}
