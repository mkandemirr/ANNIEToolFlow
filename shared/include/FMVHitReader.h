// FMVHitReader.h
#ifndef FMVHITREADER_H
#define FMVHITREADER_H

#include <TTree.h>
#include "EFMVHits.h"

class FMVHitReader {
public:
    void Init(TTree* tree);
    void ReadEntry(Long64_t entry, EFMVHits& outHits);
    void Print() const;

private:
    int                 fVetoHit            = 0;
    std::vector<double>* fFMVHitT           = nullptr;
    std::vector<int>*    fFMVHitDetID       = nullptr;
    std::vector<int>*    fFMVHitChankey     = nullptr;
    std::vector<int>*    fFMVHitChankeyMC   = nullptr;
};

#endif // FMVHITREADER_H
