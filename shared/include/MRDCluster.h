// MRDCluster.h
#ifndef MRDCLUSTER_H
#define MRDCLUSTER_H

#include <vector>
#include "MRDHit.h"

class MRDCluster {
public:
    void AddHit(const MRDHit& h)   { mHits.push_back(h); }
    const std::vector<MRDHit>& GetHits() const { return mHits; }
    void SetTime(double t)         { mTime = t; }
    void SetTimeSigma(double s)    { mTimeSigma = s; }
    void SetHitNumber(int n)       { mHitNumber = n; }

    double GetTime() const         { return mTime; }
    double GetTimeSigma() const    { return mTimeSigma; }
    int    GetHitNumber() const    { return mHitNumber; }
    void Print() const;
private:
    std::vector<MRDHit> mHits;
    double mTime       = 0.0;
    double mTimeSigma  = 0.0;
    int    mHitNumber  = 0;
};

#endif // MRDCLUSTER_H
