// PMTHitReader.cpp
#include "PMTHitReader.h"
#include <iostream>

void PMTHitReader::Init(TTree* tree) {
    tree->SetBranchAddress("filter",    &mIsFiltered); // no data in this branch!!!ask Yue
    tree->SetBranchAddress("hitX",      &mHitX);
    tree->SetBranchAddress("hitY",      &mHitY);
    tree->SetBranchAddress("hitZ",      &mHitZ);
    tree->SetBranchAddress("hitT",      &mHitT);
    tree->SetBranchAddress("hitQ",      &mHitQ);
    tree->SetBranchAddress("hitPE",     &mHitPE);
    tree->SetBranchAddress("hitType",   &mHitType);
    tree->SetBranchAddress("hitDetID",  &mHitDetID);
    tree->SetBranchAddress("hitChankey",&mHitChankey);
    tree->SetBranchAddress("hitChankeyMC",&mHitChankeyMC);
}

void PMTHitReader::ReadEntry(Long64_t entry, EPMTHits& eventHits) {
    eventHits.Clear();
    // assume the TTree::GetEntry(entry) was called before this
    size_t n = mHitX->size();
    for (size_t i = 0; i < n; ++i) {
        PMTHit hit(
            mHitX->at(i), mHitY->at(i), mHitZ->at(i), mHitT->at(i),
            mHitQ->at(i), mHitPE->at(i),
            mHitType->at(i), mHitDetID->at(i),
            mHitChankey->at(i), mHitChankeyMC->at(i)
        );
        eventHits.AddHit(hit);
    }
}

void PMTHitReader::Print() const {
    std::cout << "PMTHitReader: branches bound for all PMT hit vectors." << std::endl;
}

