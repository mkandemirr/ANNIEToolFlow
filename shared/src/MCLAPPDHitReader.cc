#include "MCLAPPDHitReader.h"

void MCLAPPDHitReader::Init(TTree* tree) {
    tree->SetBranchAddress("LAPPDMCHitTubeIDs", &fTubeIDs);
    tree->SetBranchAddress("LAPPDMCHitChankeys", &fChankeys);
    tree->SetBranchAddress("LAPPDMCHitTime", &fTime);
    tree->SetBranchAddress("LAPPDMCHitCharge", &fCharge);
    tree->SetBranchAddress("LAPPDMCHitX", &fX);
    tree->SetBranchAddress("LAPPDMCHitY", &fY);
    tree->SetBranchAddress("LAPPDMCHitZ", &fZ);
    tree->SetBranchAddress("LAPPDMCHitParallelPos", &fParallelPos);
    tree->SetBranchAddress("LAPPDMCHitTransversePos", &fTransversePos);
}

void MCLAPPDHitReader::ReadEntry(Long64_t entry, EMCLAPPDHits& eventHits) {
    eventHits.Clear();
    size_t nhits = fTubeIDs->size();
    for (size_t i = 0; i < nhits; ++i) {
        MCLAPPDHit hit(
            fTubeIDs->at(i), fChankeys->at(i), fTime->at(i), fCharge->at(i),
            fX->at(i), fY->at(i), fZ->at(i),
            fParallelPos->at(i), fTransversePos->at(i)
        );
        eventHits.AddHit(hit);
    }
}

void MCLAPPDHitReader::Print() const {
    std::cout << "MCLAPPDHitReader initialized with MC LAPPD hit branches." << std::endl;
}

