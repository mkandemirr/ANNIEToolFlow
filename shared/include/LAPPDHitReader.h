#ifndef LAPPDHITREADER_H
#define LAPPDHITREADER_H

#include <vector>
#include "TTree.h"
#include "LAPPDHit.h"
#include "ELAPPDHits.h"
#include <iostream>

class LAPPDHitReader {
public:
    void Init(TTree* tree);
    void ReadEntry(Long64_t entry, ELAPPDHits& eventHits);
    void Print() const;

private:
    std::vector<int>* mLAPPDHit_IDs = nullptr;
    std::vector<int>* mLAPPDHitChannel = nullptr;
    std::vector<int>* mLAPPDHitStrip = nullptr;
    std::vector<double>* mLAPPDHitTime = nullptr;
    std::vector<double>* mLAPPDHitAmp = nullptr;
    std::vector<double>* mLAPPDHitParallelPos = nullptr;
    std::vector<double>* mLAPPDHitTransversePos = nullptr;
    std::vector<double>* mLAPPDHitP1StartTime = nullptr;
    std::vector<double>* mLAPPDHitP2StartTime = nullptr;
    std::vector<double>* mLAPPDHitP1EndTime = nullptr;
    std::vector<double>* mLAPPDHitP2EndTime = nullptr;
    std::vector<double>* mLAPPDHitP1PeakTime = nullptr;
    std::vector<double>* mLAPPDHitP2PeakTime = nullptr;
    std::vector<double>* mLAPPDHitP1PeakAmp = nullptr;
    std::vector<double>* mLAPPDHitP2PeakAmp = nullptr;
    std::vector<double>* mLAPPDHitP1HalfHeightTime = nullptr;
    std::vector<double>* mLAPPDHitP2HalfHeightTime = nullptr;
    std::vector<double>* mLAPPDHitP1HalfEndTime = nullptr;
    std::vector<double>* mLAPPDHitP2HalfEndTime = nullptr;
    std::vector<double>* mLAPPDHitP1Charge = nullptr;
    std::vector<double>* mLAPPDHitP2Charge = nullptr;
};

#endif // LAPPDHITREADER_H

