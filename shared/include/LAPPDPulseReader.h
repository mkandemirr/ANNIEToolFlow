#ifndef LAPPDPULSEREADER_H
#define LAPPDPULSEREADER_H

#include <vector>
#include "TTree.h"
#include "LAPPDPulse.h"
#include "ELAPPDPulses.h"
#include <iostream>

class LAPPDPulseReader {
public:
    void Init(TTree* tree);
    void ReadEntry(Long64_t entry, ELAPPDPulses& eventPulses);
    void Print() const;

private:
    std::vector<int>* mLAPPD_IDs = nullptr;
    std::vector<int>* mChannelID = nullptr;
    std::vector<double>* mPulsePeakTime = nullptr;
    std::vector<double>* mHalfHeightTime = nullptr;
    std::vector<double>* mPulseCharge = nullptr;
    std::vector<double>* mPulsePeakAmp = nullptr;
    std::vector<double>* mPulseStart = nullptr;
    std::vector<double>* mPulseEnd = nullptr;
};

#endif // LAPPDPULSEREADER_H


