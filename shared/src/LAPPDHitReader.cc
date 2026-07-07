#include "LAPPDHitReader.h"

void LAPPDHitReader::Init(TTree* tree) {
    tree->SetBranchAddress("LAPPDID_Hit", &mLAPPDHit_IDs);
    tree->SetBranchAddress("LAPPDHitChannel", &mLAPPDHitChannel);
    tree->SetBranchAddress("LAPPDHitStrip", &mLAPPDHitStrip);
    tree->SetBranchAddress("LAPPDHitTime", &mLAPPDHitTime);
    tree->SetBranchAddress("LAPPDHitAmp", &mLAPPDHitAmp);
    tree->SetBranchAddress("LAPPDHitParallelPos", &mLAPPDHitParallelPos);
    tree->SetBranchAddress("LAPPDHitTransversePos", &mLAPPDHitTransversePos);
    tree->SetBranchAddress("LAPPDHitP1StartTime", &mLAPPDHitP1StartTime);
    tree->SetBranchAddress("LAPPDHitP2StartTime", &mLAPPDHitP2StartTime);
    tree->SetBranchAddress("LAPPDHitP1EndTime", &mLAPPDHitP1EndTime);
    tree->SetBranchAddress("LAPPDHitP2EndTime", &mLAPPDHitP2EndTime);
    tree->SetBranchAddress("LAPPDHitP1PeakTime", &mLAPPDHitP1PeakTime);
    tree->SetBranchAddress("LAPPDHitP2PeakTime", &mLAPPDHitP2PeakTime);
    tree->SetBranchAddress("LAPPDHitP1PeakAmp", &mLAPPDHitP1PeakAmp);
    tree->SetBranchAddress("LAPPDHitP2PeakAmp", &mLAPPDHitP2PeakAmp);
    tree->SetBranchAddress("LAPPDHitP1HalfHeightTime", &mLAPPDHitP1HalfHeightTime);
    tree->SetBranchAddress("LAPPDHitP2HalfHeightTime", &mLAPPDHitP2HalfHeightTime);
    tree->SetBranchAddress("LAPPDHitP1HalfEndTime", &mLAPPDHitP1HalfEndTime);
    tree->SetBranchAddress("LAPPDHitP2HalfEndTime", &mLAPPDHitP2HalfEndTime);
    tree->SetBranchAddress("LAPPDHitP1Charge", &mLAPPDHitP1Charge);
    tree->SetBranchAddress("LAPPDHitP2Charge", &mLAPPDHitP2Charge);
}

void LAPPDHitReader::ReadEntry(Long64_t entry, ELAPPDHits& eventHits) {
    // Clear the event hits for the current entry
    eventHits.Clear();

    size_t nhits = mLAPPDHit_IDs->size();
    for (size_t i = 0; i < nhits; ++i) {
        LAPPDHit hit(
            //mLAPPDHit_IDs->at(i), mLAPPDHitChannel->at(i), mLAPPDHitStrip->at(i),
            mLAPPDHit_IDs->at(i), -1, mLAPPDHitStrip->at(i),
            mLAPPDHitTime->at(i), mLAPPDHitAmp->at(i),
            mLAPPDHitParallelPos->at(i), mLAPPDHitTransversePos->at(i),
            mLAPPDHitP1StartTime->at(i), mLAPPDHitP2StartTime->at(i),
            mLAPPDHitP1EndTime->at(i), mLAPPDHitP2EndTime->at(i),
            mLAPPDHitP1PeakTime->at(i), mLAPPDHitP2PeakTime->at(i),
            mLAPPDHitP1PeakAmp->at(i), mLAPPDHitP2PeakAmp->at(i),
            mLAPPDHitP1HalfHeightTime->at(i), mLAPPDHitP2HalfHeightTime->at(i),
            mLAPPDHitP1HalfEndTime->at(i), mLAPPDHitP2HalfEndTime->at(i),
            mLAPPDHitP1Charge->at(i), mLAPPDHitP2Charge->at(i)
        );

        eventHits.AddHit(hit);
    }
}

void LAPPDHitReader::Print() const {
    std::cout << "LAPPDHitReader initialized with all hit branches." << std::endl;
}

