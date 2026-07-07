#include "MCLAPPDHit.h"

MCLAPPDHit::MCLAPPDHit(int tubeID, unsigned long chankey, double time, double charge,
                       double x, double y, double z,
                       double parallelPos, double transversePos)
    : fTubeID(tubeID), fChankey(chankey), fTime(time), fCharge(charge),
      fX(x), fY(y), fZ(z), fParallelPos(parallelPos), fTransversePos(transversePos) {}

int MCLAPPDHit::GetTubeID() const { return fTubeID; }
unsigned long MCLAPPDHit::GetChankey() const { return fChankey; }
double MCLAPPDHit::GetTime() const { return fTime; }
double MCLAPPDHit::GetCharge() const { return fCharge; }
double MCLAPPDHit::GetX() const { return fX; }
double MCLAPPDHit::GetY() const { return fY; }
double MCLAPPDHit::GetZ() const { return fZ; }
double MCLAPPDHit::GetParallelPos() const { return fParallelPos; }
double MCLAPPDHit::GetTransversePos() const { return fTransversePos; }

void MCLAPPDHit::Print() const {
    std::cout << "MCLAPPDHit:\n"
              << "TubeID: " << fTubeID << "\n"
              << "Chankey: " << fChankey << "\n"
              << "Time: " << fTime << "\n"
              << "Charge: " << fCharge << "\n"
              << "Position (X,Y,Z): (" << fX << ", " << fY << ", " << fZ << ")\n"
              << "Parallel Pos: " << fParallelPos << ", Transverse Pos: " << fTransversePos << "\n"
              << std::endl;
}

