#ifndef MCLAPPDHITREADER_H
#define MCLAPPDHITREADER_H

#include <vector>
#include "TTree.h"
#include "MCLAPPDHit.h"
#include "EMCLAPPDHits.h"

class MCLAPPDHitReader {
public:
    void Init(TTree* tree);
    void ReadEntry(Long64_t entry, EMCLAPPDHits& eventHits);
    void Print() const;

private:
    std::vector<int>* fTubeIDs = nullptr;
    std::vector<unsigned long>* fChankeys = nullptr;
    std::vector<double>* fTime = nullptr;
    std::vector<double>* fCharge = nullptr;
    std::vector<double>* fX = nullptr;
    std::vector<double>* fY = nullptr;
    std::vector<double>* fZ = nullptr;
    std::vector<double>* fParallelPos = nullptr;
    std::vector<double>* fTransversePos = nullptr;
};

#endif // MCLAPPDHITREADER_H

