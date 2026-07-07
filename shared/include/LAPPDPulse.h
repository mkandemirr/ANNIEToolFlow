#ifndef LAPPDPULSE_H
#define LAPPDPULSE_H

#include "TObject.h"
#include <iostream>

class LAPPDPulse {
private:
    int mLAPPDID;
    int mChannelID;
    double mTime;
    double mHalfHeightTime;
    double mCharge;
    double mPeak;
    double mLowRange;
    double mHiRange;

public:
    LAPPDPulse();  // Default constructor
    LAPPDPulse(int lappdID, int channelID, double time, double halfHeightTime,
               double charge, double peak, double lowRange, double hiRange);


      void Print() const; 

    int GetLAPPDID() const;
    int GetChannelID() const;
    double GetTime() const;
    double GetHalfHeightTime() const;
    double GetCharge() const;
    double GetPeak() const;
    double GetLowRange() const;
    double GetHiRange() const;


};

#endif // LAPPDPULSE_H

