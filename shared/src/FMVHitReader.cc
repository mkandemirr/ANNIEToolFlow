#include "FMVHitReader.h"
#include "FMVHit.h"
#include <iostream>

void FMVHitReader::Init(TTree* tree) {
    tree->SetBranchAddress("vetoHit",         &fVetoHit);
    tree->SetBranchAddress("FMVhitT",         &fFMVHitT);
    tree->SetBranchAddress("FMVhitDetID",     &fFMVHitDetID);
    tree->SetBranchAddress("FMVhitChankey",   &fFMVHitChankey);
    tree->SetBranchAddress("FMVhitChankeyMC", &fFMVHitChankeyMC);
}

void FMVHitReader::ReadEntry(Long64_t entry, EFMVHits& outHits) {
    outHits.Clear();
    // assume tree->GetEntry(entry) called before
    size_t n = fFMVHitT->size();
    for (size_t i = 0; i < n; ++i) {
        FMVHit hit(
            fFMVHitT->at(i),
            fFMVHitDetID->at(i),
            fFMVHitChankey->at(i),
            fFMVHitChankeyMC->at(i)
        );
        outHits.AddHit(hit);
    }
}

void FMVHitReader::Print() const {
    std::cout << "FMVHitReader: initialized." << std::endl;
}
