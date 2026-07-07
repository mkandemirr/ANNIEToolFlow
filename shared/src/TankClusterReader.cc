// TankClusterReader.cpp
#include "TankClusterReader.h"
#include "PMTHit.h"
#include <iostream>

void TankClusterReader::Init(TTree* tree) {
    if (!tree) {
        std::cerr << "[TankClusterReader] Error: Input TTree pointer is null!" << std::endl;
        return;
    }

    fTree = tree;

    auto checkBranch = [&](const char* name, void* address) {
        TBranch* b = fTree->GetBranch(name);
        if (!b) {
            std::cerr << "Warning: Branch '" << name << "' not found in the TTree." << std::endl;
            throw std::runtime_error("asdsadsad");
        } else {
            fTree->SetBranchAddress(name, address);
            Long64_t n = b->GetEntries();
            if (n == 0) {
                std::cerr << "Warning: Branch '" << name
                          << "' exists but contains no data (0 entries)." << std::endl;
                          
                throw std::runtime_error("no data in this branch!");          
            }
        }
    };

    checkBranch("numberOfClusters",      &fNumberOfClusters);
    checkBranch("clusterHits",           &fClusterHits);
    checkBranch("clusterCharge",         &fClusterChargeV);
    checkBranch("clusterTime",           &fClusterTimeV);
    checkBranch("clusterPE",             &fClusterPEV);
    checkBranch("clusterMaxPE",          &fClusterMaxPEV);
    checkBranch("clusterChargePointX",   &fClusterChargePointXV);
    checkBranch("clusterChargePointY",   &fClusterChargePointYV);
    checkBranch("clusterChargePointZ",   &fClusterChargePointZV);
    checkBranch("clusterChargeBalance",  &fClusterChargeBalanceV);

    checkBranch("Cluster_HitX",          &fCluster_HitX);
    checkBranch("Cluster_HitY",          &fCluster_HitY);
    checkBranch("Cluster_HitZ",          &fCluster_HitZ);
    checkBranch("Cluster_HitT",          &fCluster_HitT);
    checkBranch("Cluster_HitQ",          &fCluster_HitQ);
    checkBranch("Cluster_HitPE",         &fCluster_HitPE);
    checkBranch("Cluster_HitType",       &fCluster_HitType);
    checkBranch("Cluster_HitDetID",      &fCluster_HitDetID);
    checkBranch("Cluster_HitChankey",    &fCluster_HitChankey);
    checkBranch("Cluster_HitChankeyMC",  &fCluster_HitChankeyMC);

    
}

void TankClusterReader::ReadEntry(Long64_t entry, ETankClusters& eventClusters) {
    if (!fTree) return;
    fTree->GetEntry(entry);
    eventClusters.Clear();

   //std::cout<<"SEMELE: "<<fNumberOfClusters<<std::endl;
    for (int ic=0; ic < fNumberOfClusters; ++ic) {
        TankCluster cl;
        cl.SetClusterCharge(      fClusterChargeV->at(ic));
        cl.SetClusterTime(        fClusterTimeV->at(ic));
        cl.SetClusterPE(          fClusterPEV->at(ic));
        cl.SetMaxPE(             fClusterMaxPEV->at(ic));
        cl.SetChargePoint(
            fClusterChargePointXV->at(ic),
            fClusterChargePointYV->at(ic),
            fClusterChargePointZV->at(ic)
        );
        cl.SetChargeBalance(      fClusterChargeBalanceV->at(ic));

         
        for (size_t ih=0; ih < fCluster_HitX->at(ic).size(); ++ih) {
        
            PMTHit hit(
                fCluster_HitX->at(ic).at(ih),
                fCluster_HitY->at(ic).at(ih),
                fCluster_HitZ->at(ic).at(ih),
                fCluster_HitT->at(ic).at(ih),
                fCluster_HitQ->at(ic).at(ih),
                fCluster_HitPE->at(ic).at(ih),
                fCluster_HitType->at(ic).at(ih),
                fCluster_HitDetID->at(ic).at(ih),
                fCluster_HitChankey->at(ic).at(ih),
                fCluster_HitChankeyMC->at(ic).at(ih)
                
            );
            cl.AddHit(hit);
            
            
        }
        
        eventClusters.AddCluster(cl);
    
    }
}



