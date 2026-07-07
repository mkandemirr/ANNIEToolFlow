// PMTHitReader.h
#ifndef PMTHITREADER_H
#define PMTHITREADER_H

#include <vector>
#include "TTree.h"
#include "PMTHit.h"
#include "EPMTHits.h"  // your container for per-event hits

class PMTHitReader {
public:
    void Init(TTree* tree);
    void ReadEntry(Long64_t entry, EPMTHits& eventHits);
    void Print() const;

private:
    std::vector<int>*    mIsFiltered    = nullptr;
    std::vector<double>* mHitX          = nullptr;
    std::vector<double>* mHitY          = nullptr;
    std::vector<double>* mHitZ          = nullptr;
    std::vector<double>* mHitT          = nullptr;
    std::vector<double>* mHitQ          = nullptr;
    std::vector<double>* mHitPE         = nullptr;
    std::vector<int>*    mHitType       = nullptr;
    std::vector<int>*    mHitDetID      = nullptr;
    std::vector<int>*    mHitChankey    = nullptr;
    std::vector<int>*    mHitChankeyMC  = nullptr;
};

#endif // PMTHITREADER_H

