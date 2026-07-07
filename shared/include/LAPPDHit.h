#ifndef LAPPDHIT_H
#define LAPPDHIT_H

#include "TObject.h"
#include <iostream>

class LAPPDHit {
public:
    // Default constructor
    LAPPDHit() = default;

    // Parametrized constructor to initialize all member variables
    LAPPDHit(int LAPPDID, int channel, int strip, double time, double amplitude,
             double parallelPos, double transversePos, double p1StartTime, double p2StartTime,
             double p1EndTime, double p2EndTime, double p1PeakTime, double p2PeakTime,
             double p1PeakAmp, double p2PeakAmp, double p1HalfHeightTime, double p2HalfHeightTime,
             double p1HalfEndTime, double p2HalfEndTime, double p1Charge, double p2Charge);


      // Getters
      int    GetLAPPDID() const            { return mLAPPDID; }
      int    GetChannel() const            { return mChannel; }
      int    GetStrip() const              { return mStrip; }

      double GetTime() const               { return mTime; }
      double GetAmplitude() const          { return mAmplitude; }

      double GetParallelPos() const        { return mParallelPos; }
      double GetTransversePos() const      { return mTransversePos; }

      double GetP1StartTime() const        { return mP1StartTime; }
      double GetP2StartTime() const        { return mP2StartTime; }

      double GetP1EndTime() const          { return mP1EndTime; }
      double GetP2EndTime() const          { return mP2EndTime; }

      double GetP1PeakTime() const         { return mP1PeakTime; }
      double GetP2PeakTime() const         { return mP2PeakTime; }

      double GetP1PeakAmp() const          { return mP1PeakAmp; }
      double GetP2PeakAmp() const          { return mP2PeakAmp; }

      double GetP1HalfHeightTime() const   { return mP1HalfHeightTime; }
      double GetP2HalfHeightTime() const   { return mP2HalfHeightTime; }

      double GetP1HalfEndTime() const      { return mP1HalfEndTime; }
      double GetP2HalfEndTime() const      { return mP2HalfEndTime; }

      double GetP1Charge() const           { return mP1Charge; }
      double GetP2Charge() const           { return mP2Charge; }


    // Setters
    void SetLAPPDID(int id);
    void SetChannel(int channel);
    void SetStrip(int strip);
    void SetTime(double time);
    void SetAmplitude(double amp);
    void SetParallelPos(double pos);
    void SetTransversePos(double pos);
    void SetP1StartTime(double time);
    void SetP2StartTime(double time);
    void SetP1EndTime(double time);
    void SetP2EndTime(double time);
    void SetP1PeakTime(double time);
    void SetP2PeakTime(double time);
    void SetP1PeakAmp(double amp);
    void SetP2PeakAmp(double amp);
    void SetP1HalfHeightTime(double time);
    void SetP2HalfHeightTime(double time);
    void SetP1HalfEndTime(double time);
    void SetP2HalfEndTime(double time);
    void SetP1Charge(double charge);
    void SetP2Charge(double charge);

    // Debug
    void Print() const;

private:
    int mLAPPDID = -1;
    int mChannel = -1;
    int mStrip = -1;
    double mTime = 0.0;
    double mAmplitude = 0.0;
    double mParallelPos = 0.0;
    double mTransversePos = 0.0;
    double mP1StartTime = 0.0;
    double mP2StartTime = 0.0;
    double mP1EndTime = 0.0;
    double mP2EndTime = 0.0;
    double mP1PeakTime = 0.0;
    double mP2PeakTime = 0.0;
    double mP1PeakAmp = 0.0;
    double mP2PeakAmp = 0.0;
    double mP1HalfHeightTime = 0.0;
    double mP2HalfHeightTime = 0.0;
    double mP1HalfEndTime = 0.0;
    double mP2HalfEndTime = 0.0;
    double mP1Charge = 0.0;
    double mP2Charge = 0.0;
    
    
};

#endif // LAPPDHIT_H


