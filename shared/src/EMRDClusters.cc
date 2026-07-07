#include "EMRDClusters.h"
#include <iostream>

void EMRDClusters::AddCluster(const MRDCluster& cluster) { 
   mClusters.push_back(cluster); 
}

void EMRDClusters::Clear() { 
   mClusters.clear(); 
}

void EMRDClusters::Print() const {
    std::cout << "== MRD Clusters ==" << std::endl;
    std::cout << "Number of MRD clusters in this event: " << mClusters.size() << std::endl;
    for (const auto& c : mClusters) 
      c.Print();
}
