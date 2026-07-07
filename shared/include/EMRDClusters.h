// EMRDClusters.h
#ifndef EVENTMRDCLUSTERS_H
#define EVENTMRDCLUSTERS_H

#include <vector>
#include "MRDCluster.h"

class EMRDClusters {
public:
    void AddCluster(const MRDCluster& cluster);
    const std::vector<MRDCluster>& Get() { return mClusters; } 
    void Clear();
    void Print() const;

private:
    std::vector<MRDCluster> mClusters;
};

#endif // EVENTMRDCLUSTERS_H
