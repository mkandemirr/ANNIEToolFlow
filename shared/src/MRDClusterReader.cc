#include "MRDClusterReader.h"
#include "MRDHit.h"
#include <iostream>

void MRDClusterReader::Init(TTree* tree) {
    if (!tree) {
        std::cerr << "[MRDClusterReader] Error: Input TTree pointer is null!" << std::endl;
        return;
    }
    fTree = tree;

    auto checkBranch = [&](const char* name, void* addr) {
        TBranch* b = fTree->GetBranch(name);
        if (!b) {
            std::cerr << "Warning: Branch '" << name << "' not found." << std::endl;
        } else {
            fTree->SetBranchAddress(name, addr);
            if (b->GetEntries() == 0) {
                std::cerr << "Warning: Branch '" << name << "' has no entries." << std::endl;
            }
        }
    };

    // cluster branches
    checkBranch("eventTimeMRD",        &fEventTimeMRD);
    checkBranch("MRDClusterNumber",    &fMRDClusterNumber);
    checkBranch("MRDClusterHitNumber", &fMRDClusterHitNumber);
    checkBranch("MRDClusterTime",      &fMRDClusterTime);
    checkBranch("MRDClusterTimeSigma", &fMRDClusterTimeSigma);

    // hit branches
    checkBranch("vetoHit",             &fVetoHit);
    checkBranch("MRDHitClusterIndex",  &fMRDHitClusterIndex);
    checkBranch("MRDhitT",             &fMRDHitT);
    checkBranch("MRDHitCharge",        &fMRDHitCharge);
    checkBranch("MRDHitDigitPMT",      &fMRDHitDigitPMT);
    checkBranch("MRDhitDetID",         &fMRDHitDetID);
    checkBranch("MRDhitChankey",       &fMRDHitChankey);
    checkBranch("MRDhitChankeyMC",     &fMRDHitChankeyMC);
}

void MRDClusterReader::ReadEntry(Long64_t entry, EMRDClusters& eventClusters) {
    if (!fTree) return;
    fTree->GetEntry(entry);
    eventClusters.Clear();

    // build each cluster
    for (int ic = 0; ic < fMRDClusterNumber; ++ic) {
        MRDCluster cl;
        cl.SetTime(      fMRDClusterTime->at(ic));
        cl.SetTimeSigma( fMRDClusterTimeSigma->at(ic));
        cl.SetHitNumber( fMRDClusterHitNumber->at(ic));

        // attach hits
        size_t nh = fMRDHitClusterIndex->size();
        for (size_t ih = 0; ih < nh; ++ih) {
            if (fMRDHitClusterIndex->at(ih) != ic) continue;
            MRDHit hit(
                fMRDHitClusterIndex->at(ih),
                fMRDHitT->at(ih),
                fMRDHitCharge->at(ih),
                fMRDHitDigitPMT->at(ih),
                fMRDHitDetID->at(ih),
                fMRDHitChankey->at(ih),
                fMRDHitChankeyMC->at(ih)
            );
            cl.AddHit(hit);
        }
        eventClusters.AddCluster(cl);
    }
}



