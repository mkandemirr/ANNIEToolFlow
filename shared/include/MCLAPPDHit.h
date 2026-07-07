// MCLAPPDHit.h
#ifndef MCLAPPDHIT_H
#define MCLAPPDHIT_H

#include <iostream>

class MCLAPPDHit {
public:
    MCLAPPDHit(int tubeID, unsigned long chankey, double time, double charge,
               double x, double y, double z,
               double parallelPos, double transversePos);

    // Getters
    int GetTubeID() const;
    unsigned long GetChankey() const;
    double GetTime() const;
    double GetCharge() const;
    double GetX() const;
    double GetY() const;
    double GetZ() const;
    double GetParallelPos() const;
    double GetTransversePos() const;

    void Print() const;

private:
    int fTubeID;
    unsigned long fChankey;
    double fTime;
    double fCharge;
    double fX, fY, fZ;
    double fParallelPos;
    double fTransversePos;
};

#endif // MCLAPPDHIT_H

