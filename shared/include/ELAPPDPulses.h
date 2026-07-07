#ifndef LAPPDEVENTPULSES_H
#define LAPPDEVENTPULSES_H

#include <vector>
#include "LAPPDPulse.h"

class ELAPPDPulses {
public:
    void AddPulse(const LAPPDPulse& pulse);
    const std::vector<LAPPDPulse>& GetPulses() const;
    void Clear();
    void Print() const;

private:
    std::vector<LAPPDPulse> mPulses;
};

#endif // LAPPDEVENTPULSES_H

