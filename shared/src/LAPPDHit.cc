#include "LAPPDHit.h"

// Parametrized constructor to initialize all member variables
LAPPDHit::LAPPDHit(int LAPPDID, int channel, int strip, double time, double amplitude,
                   double parallelPos, double transversePos, double p1StartTime, double p2StartTime,
                   double p1EndTime, double p2EndTime, double p1PeakTime, double p2PeakTime,
                   double p1PeakAmp, double p2PeakAmp, double p1HalfHeightTime, double p2HalfHeightTime,
                   double p1HalfEndTime, double p2HalfEndTime, double p1Charge, double p2Charge)
    : mLAPPDID(LAPPDID), mChannel(channel), mStrip(strip), mTime(time), mAmplitude(amplitude),
      mParallelPos(parallelPos), mTransversePos(transversePos), mP1StartTime(p1StartTime),
      mP2StartTime(p2StartTime), mP1EndTime(p1EndTime), mP2EndTime(p2EndTime), mP1PeakTime(p1PeakTime),
      mP2PeakTime(p2PeakTime), mP1PeakAmp(p1PeakAmp), mP2PeakAmp(p2PeakAmp), mP1HalfHeightTime(p1HalfHeightTime),
      mP2HalfHeightTime(p2HalfHeightTime), mP1HalfEndTime(p1HalfEndTime), mP2HalfEndTime(p2HalfEndTime),
      mP1Charge(p1Charge), mP2Charge(p2Charge) {}

// Setters
void LAPPDHit::SetLAPPDID(int id) { mLAPPDID = id; }
void LAPPDHit::SetChannel(int channel) { mChannel = channel; }
void LAPPDHit::SetStrip(int strip) { mStrip = strip; }
void LAPPDHit::SetTime(double time) { mTime = time; }
void LAPPDHit::SetAmplitude(double amp) { mAmplitude = amp; }
void LAPPDHit::SetParallelPos(double pos) { mParallelPos = pos; }
void LAPPDHit::SetTransversePos(double pos) { mTransversePos = pos; }
void LAPPDHit::SetP1StartTime(double time) { mP1StartTime = time; }
void LAPPDHit::SetP2StartTime(double time) { mP2StartTime = time; }
void LAPPDHit::SetP1EndTime(double time) { mP1EndTime = time; }
void LAPPDHit::SetP2EndTime(double time) { mP2EndTime = time; }
void LAPPDHit::SetP1PeakTime(double time) { mP1PeakTime = time; }
void LAPPDHit::SetP2PeakTime(double time) { mP2PeakTime = time; }
void LAPPDHit::SetP1PeakAmp(double amp) { mP1PeakAmp = amp; }
void LAPPDHit::SetP2PeakAmp(double amp) { mP2PeakAmp = amp; }
void LAPPDHit::SetP1HalfHeightTime(double time) { mP1HalfHeightTime = time; }
void LAPPDHit::SetP2HalfHeightTime(double time) { mP2HalfHeightTime = time; }
void LAPPDHit::SetP1HalfEndTime(double time) { mP1HalfEndTime = time; }
void LAPPDHit::SetP2HalfEndTime(double time) { mP2HalfEndTime = time; }
void LAPPDHit::SetP1Charge(double charge) { mP1Charge = charge; }
void LAPPDHit::SetP2Charge(double charge) { mP2Charge = charge; }

// Debug method to print hit details
void LAPPDHit::Print() const {
    std::cout << "LAPPDHit: \n"
              << "LAPPDID: " << mLAPPDID << "\n"
              << "Channel: " << mChannel << "\n"
              << "Strip: " << mStrip << "\n"
              << "Time: " << mTime << "\n"
              << "Amplitude: " << mAmplitude << "\n"
              << "Position (parallel, transverse): (" << mParallelPos << ", " << mTransversePos << ")\n"
              << "P1 Start Time: " << mP1StartTime << "\n"
              << "P1 End Time: " << mP1EndTime << "\n"
              << "P1 Peak Time: " << mP1PeakTime << "\n"
              << "P1 Peak Amplitude: " << mP1PeakAmp << "\n"
              << "P1 Half Height Time: " << mP1HalfHeightTime << "\n"
              << "P1 Half End Time: " << mP1HalfEndTime << "\n"
              << "P1 Charge: " << mP1Charge << "\n"
              << "P2 Start Time: " << mP2StartTime << "\n"
              << "P2 End Time: " << mP2EndTime << "\n"
              << "P2 Peak Time: " << mP2PeakTime << "\n"
              << "P2 Peak Amplitude: " << mP2PeakAmp << "\n"
              << "P2 Half Height Time: " << mP2HalfHeightTime << "\n"
              << "P2 Half End Time: " << mP2HalfEndTime << "\n"
              << "P2 Charge: " << mP2Charge << "\n"
              << std::endl;
}



