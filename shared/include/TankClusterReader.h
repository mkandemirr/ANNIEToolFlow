// TankClusterReader.h
#ifndef TANKCLUSTERREADER_H
#define TANKCLUSTERREADER_H

#include <TTree.h>
#include "ETankClusters.h"

class TankClusterReader {
public:
    void Init(TTree* tree);
    void ReadEntry(Long64_t entry, ETankClusters& cluster);
    

private:
    Int_t fNumberOfClusters = 0;
    std::vector<int>*    fClusterHits           = nullptr;
    std::vector<double>* fClusterChargeV        = nullptr;
    std::vector<double>* fClusterTimeV          = nullptr;
    std::vector<double>* fClusterPEV            = nullptr;
    std::vector<double>* fClusterMaxPEV         = nullptr;
    std::vector<double>* fClusterChargePointXV  = nullptr;
    std::vector<double>* fClusterChargePointYV  = nullptr;
    std::vector<double>* fClusterChargePointZV  = nullptr;
    std::vector<double>* fClusterChargeBalanceV = nullptr;

    std::vector<std::vector<double>>* fCluster_HitX     = nullptr;
    std::vector<std::vector<double>>* fCluster_HitY     = nullptr;
    std::vector<std::vector<double>>* fCluster_HitZ     = nullptr;
    std::vector<std::vector<double>>* fCluster_HitT     = nullptr;
    std::vector<std::vector<double>>* fCluster_HitQ     = nullptr;
    std::vector<std::vector<double>>* fCluster_HitPE    = nullptr;
    std::vector<std::vector<int>>*    fCluster_HitType  = nullptr;
    std::vector<std::vector<int>>*    fCluster_HitDetID = nullptr;
    std::vector<std::vector<int>>*    fCluster_HitChankey   = nullptr;
    std::vector<std::vector<int>>*    fCluster_HitChankeyMC = nullptr;

    TTree* fTree = nullptr;
};

#endif // TANKCLUSTERREADER_H
