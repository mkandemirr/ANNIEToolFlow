#ifndef LAPPDMETAREADER_H
#define LAPPDMETAREADER_H

#include <TTree.h>
#include <vector>
#include <cstdint>
#include <iostream>

class LAPPDMetaReader {
public:
    void Init(TTree* tree);
    void ReadEntry(Long64_t entry);
    void Print() const;
    void PrintMetaEntrySummary() const;

//private:
    int mLAPPD_Count;
    std::vector<int>* mLAPPD_ID = nullptr;
    std::vector<uint64_t>* mLAPPD_Beamgate_ns = nullptr;
    std::vector<uint64_t>* mLAPPD_Timestamp_ns = nullptr;
    std::vector<uint64_t>* mLAPPD_Beamgate_Raw = nullptr;
    std::vector<uint64_t>* mLAPPD_Timestamp_Raw = nullptr;
    std::vector<uint64_t>* mLAPPD_Offset = nullptr;
    std::vector<int>* mLAPPD_TSCorrection = nullptr;
    std::vector<int>* mLAPPD_BGCorrection = nullptr;
    std::vector<int>* mLAPPD_OSInMinusPS = nullptr;

    std::vector<uint64_t>* mLAPPD_BGPPSBefore = nullptr;
    std::vector<uint64_t>* mLAPPD_BGPPSAfter = nullptr;
    std::vector<uint64_t>* mLAPPD_BGPPSDiff = nullptr;
    std::vector<int>* mLAPPD_BGPPSMissing = nullptr;
    std::vector<uint64_t>* mLAPPD_TSPPSBefore = nullptr;
    std::vector<uint64_t>* mLAPPD_TSPPSAfter = nullptr;
    std::vector<uint64_t>* mLAPPD_TSPPSDiff = nullptr;
    std::vector<int>* mLAPPD_TSPPSMissing = nullptr;

    std::vector<int>* mLAPPD_BG_switchBit0 = nullptr;
    std::vector<int>* mLAPPD_BG_switchBit1 = nullptr;
};

#endif

