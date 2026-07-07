#include "ELAPPDPulses.h"
#include <iostream>

void ELAPPDPulses::AddPulse(const LAPPDPulse& pulse) {
    mPulses.push_back(pulse);
}

const std::vector<LAPPDPulse>& ELAPPDPulses::GetPulses() const {
    return mPulses;
}

void ELAPPDPulses::Clear() {
    mPulses.clear();
}

void ELAPPDPulses::Print() const {
    std::cout << "== LAPPD Pulses ==" << std::endl;
    std::cout << "Number of LAPPD Pulses: " << mPulses.size() << std::endl;
    for (const auto& pulse : mPulses) {
        std::cout << "------------------------" << std::endl;
        pulse.Print();
    }
}

