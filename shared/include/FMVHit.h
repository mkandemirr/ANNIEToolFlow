// FMVHit.h
#ifndef FMVHIT_H
#define FMVHIT_H

#include "TObject.h"
#include <iostream>

class FMVHit {
public:
    FMVHit() = default;
    FMVHit(double t, int detID, int chankey, int chankeyMC);

    void SetTime(double v)      { mTime = v; }
    void SetDetID(int v)        { mDetID = v; }
    void SetChankey(int v)      { mChankey = v; }
    void SetChankeyMC(int v)    { mChankeyMC = v; }

    double GetTime() const      { return mTime; }
    int    GetDetID() const     { return mDetID; }
    int    GetChankey() const   { return mChankey; }
    int    GetChankeyMC() const { return mChankeyMC; }

    void Print() const;

private:
    double mTime      = 0.0;
    int    mDetID     = -1;
    int    mChankey   = -1;
    int    mChankeyMC = -1;

   
};

#endif // FMVHIT_H
