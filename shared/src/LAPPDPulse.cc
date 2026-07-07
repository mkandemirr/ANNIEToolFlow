#include "LAPPDPulse.h"

LAPPDPulse::LAPPDPulse()
    : mLAPPDID(0), mChannelID(0), mTime(0.0), mHalfHeightTime(0.0),
      mCharge(0.0), mPeak(0.0), mLowRange(0.0), mHiRange(0.0) {}

LAPPDPulse::LAPPDPulse(int lappdID, int channelID, double time, double halfHeightTime,
                       double charge, double peak, double lowRange, double hiRange)
    : mLAPPDID(lappdID), mChannelID(channelID), mTime(time), mHalfHeightTime(halfHeightTime),
      mCharge(charge), mPeak(peak), mLowRange(lowRange), mHiRange(hiRange) {}

int LAPPDPulse::GetLAPPDID() const { return mLAPPDID; }
int LAPPDPulse::GetChannelID() const { return mChannelID; }
double LAPPDPulse::GetTime() const { return mTime; }
double LAPPDPulse::GetHalfHeightTime() const { return mHalfHeightTime; }
double LAPPDPulse::GetCharge() const { return mCharge; }
double LAPPDPulse::GetPeak() const { return mPeak; }
double LAPPDPulse::GetLowRange() const { return mLowRange; }
double LAPPDPulse::GetHiRange() const { return mHiRange; }


void LAPPDPulse::Print() const {
    std::cout << "LAPPDPulse: \n"
              << "LAPPDID: " << mLAPPDID << "\n"
              << "ChannelID: " << mChannelID << "\n"
              << "Time: " << mTime << "\n"
              << "Half Height Time: " << mHalfHeightTime << "\n"
              << "Charge: " << mCharge << "\n"
              << "Peak Amplitude: " << mPeak << "\n"
              << "Low Range: " << mLowRange << "\n"
              << "High Range: " << mHiRange << "\n"
              << std::endl;
}


