// MRDClusterReader.h
#ifndef MRDCLUSTERREADER_H
#define MRDCLUSTERREADER_H

#include <TTree.h>
#include "EMRDClusters.h"

class MRDClusterReader {
public:
    void Init(TTree* tree);
    void ReadEntry(Long64_t entry, EMRDClusters& cl);
private:
    ULong64_t               fEventTimeMRD       = 0;
    Int_t                   fMRDClusterNumber   = 0;
    std::vector<int>*       fMRDClusterHitNumber = nullptr;
    std::vector<double>*    fMRDClusterTime      = nullptr;
    std::vector<double>*    fMRDClusterTimeSigma = nullptr;

    // hit branches
    Int_t                   fVetoHit            = 0;
    std::vector<int>*       fMRDHitClusterIndex = nullptr;
    std::vector<double>*    fMRDHitT             = nullptr;
    std::vector<double>*    fMRDHitCharge        = nullptr;
    std::vector<int>*       fMRDHitDigitPMT      = nullptr;
    std::vector<int>*       fMRDHitDetID         = nullptr;
    std::vector<int>*       fMRDHitChankey       = nullptr;
    std::vector<int>*       fMRDHitChankeyMC     = nullptr;

    TTree*                  fTree                = nullptr;
};

#endif // MRDCLUSTERREADER_H
