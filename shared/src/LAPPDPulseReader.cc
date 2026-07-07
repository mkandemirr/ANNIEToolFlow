#include "LAPPDPulseReader.h"

void LAPPDPulseReader::Init(TTree* tree) {
    tree->SetBranchAddress("LAPPD_PulseIDs", &mLAPPD_IDs);
    tree->SetBranchAddress("LAPPD_ChannelID", &mChannelID);
    tree->SetBranchAddress("LAPPD_PeakTime", &mPulsePeakTime);
    tree->SetBranchAddress("LAPPD_PulseHalfHeightTime", &mHalfHeightTime);
    tree->SetBranchAddress("LAPPD_Charge", &mPulseCharge);
    tree->SetBranchAddress("LAPPD_PeakAmp", &mPulsePeakAmp);
    tree->SetBranchAddress("LAPPD_PulseStart", &mPulseStart);
    tree->SetBranchAddress("LAPPD_PulseEnd", &mPulseEnd);
}

void LAPPDPulseReader::ReadEntry(Long64_t entry, ELAPPDPulses& eventPulses) {
    // Clear the event pulses for the current entry
    eventPulses.Clear();

    // Read the current entry (user should call tree->GetEntry(entry))
    size_t nPulses = mLAPPD_IDs->size();
    for (size_t i = 0; i < nPulses; ++i) {
        LAPPDPulse pulse(
            mLAPPD_IDs->at(i),
            mChannelID->at(i),
            mPulsePeakTime->at(i),
            mHalfHeightTime->at(i),
            mPulseCharge->at(i),
            mPulsePeakAmp->at(i),
            mPulseStart->at(i),
            mPulseEnd->at(i)
        );
        // Add the pulse to the event's pulse list
        eventPulses.AddPulse(pulse);
    }
}

void LAPPDPulseReader::Print() const {
    std::cout << "LAPPDPulseReader initialized with all pulse branches." << std::endl;
}

