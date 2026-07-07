// ETankClusters.h
#ifndef EVENTTANKCLUSTERS_H
#define EVENTTANKCLUSTERS_H

#include <vector>
#include "TankCluster.h"

class ETankClusters {
public:
    void AddCluster(const TankCluster& cluster);
    const std::vector<TankCluster>& Get() { return mClusters; } 
    void Clear();
    void Print() const;

private:
    std::vector<TankCluster> mClusters;
};

#endif // EVENTTANKCLUSTERS_H
