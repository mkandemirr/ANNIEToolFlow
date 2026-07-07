// MRDHit.h
#ifndef MRDHIT_H
#define MRDHIT_H

#include "TObject.h"
#include <iostream>



//MRD icin sadece cluster hitler var!Tank daki gibi degil.

class MRDHit {
public:
    MRDHit() = default;
    MRDHit(int clusterIndex, double t, double charge,
           int digitPMT, int detID, int chankey, int chankeyMC);

    void SetClusterIndex(int idx) { mClusterIndex = idx; }
    void SetTime(double v)        { mTime = v; }
    void SetCharge(double v)      { mCharge = v; }
    void SetDigitPMT(int v)       { mDigitPMT = v; }
    void SetDetID(int v)          { mDetID = v; }
    void SetChankey(int v)        { mChankey = v; }
    void SetChankeyMC(int v)      { mChankeyMC = v; }

    int    GetClusterIndex() const { return mClusterIndex; }
    double GetTime() const         { return mTime; }
    double GetCharge() const       { return mCharge; }
    int    GetDigitPMT() const     { return mDigitPMT; }
    int    GetDetID() const        { return mDetID; }
    int    GetChankey() const      { return mChankey; }
    int    GetChankeyMC() const    { return mChankeyMC; }

    void Print() const;

private:
    int    mClusterIndex = -1;
    double mTime         = 0.0;
    double mCharge       = 0.0;
    int    mDigitPMT     = -1;
    int    mDetID        = -1;
    int    mChankey      = -1;
    int    mChankeyMC    = -1;

};

#endif // MRDHIT_H

