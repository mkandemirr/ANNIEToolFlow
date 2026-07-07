// PMTHit.h
#ifndef PMTHIT_H
#define PMTHIT_H

#include "TObject.h"
#include <iostream>

class PMTHit {
public:
    //PMTHit() = default;
    PMTHit(double x, double y, double z, double t, double q, double pe,
           int type, int detID, int chankey, int chankeyMC);

    // setters
    void   SetFiltered(int f)   { mIsFiltered = f; }
    void   SetX(double v)       { mX = v; }
    void   SetY(double v)       { mY = v; }
    void   SetZ(double v)       { mZ = v; }
    void   SetT(double v)       { mT = v; }
    void   SetQ(double v)       { mQ = v; }
    void   SetPE(double v)      { mPE = v; }
    void   SetType(int t)       { mType = t; }
    void   SetDetID(int id)     { mDetID = id; }
    void   SetChankey(int k)    { mChankey = k; }
    void   SetChankeyMC(int k)  { mChankeyMC = k; }
    
    // getters
    int    GetFiltered() const  { return mIsFiltered; }
    
    //returns meter and given wrt tank center 
    double GetX() const         { return mX; }
    double GetY() const         { return mY; }
    double GetZ() const         { return mZ; }
    double GetT() const         { return mT; }
    double GetQ() const         { return mQ; }
    double GetPE() const        { return mPE; }
    int    GetType() const      { return mType; }
    int    GetDetID() const     { return mDetID; }
    int    GetChankey() const   { return mChankey; }
    int    GetChankeyMC() const { return mChankeyMC; }

    // debug
    void Print() const;

private:
    int    mIsFiltered   = 0;
    double mX            = 0.;
    double mY            = 0.;
    double mZ            = 0.;
    double mT            = 0.;
    double mQ            = 0.;
    double mPE           = 0.;
    int    mType         = 0;
    int    mDetID        = 0;
    int    mChankey      = 0;
    int    mChankeyMC    = 0;

    
};

#endif // PMTHIT_H

