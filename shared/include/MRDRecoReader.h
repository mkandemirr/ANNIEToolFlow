#ifndef MRDRECOREADER_H
#define MRDRECOREADER_H

#include <TTree.h>
#include <vector>
#include "EMRDRecos.h"

class MRDRecoReader {
public:
    void Init(TTree* tree);
    void ReadEntry(Long64_t entry, EMRDRecos& reco);

private:
    std::vector<double>* fMRDTrackAngle       = nullptr;
    std::vector<double>* fMRDTrackAngleError  = nullptr;
    std::vector<double>* fMRDPenetrationDepth = nullptr;
    std::vector<double>* fMRDTrackLength      = nullptr;
    std::vector<double>* fMRDEntryPointRadius = nullptr;
    std::vector<double>* fMRDEnergyLoss       = nullptr;
    std::vector<double>* fMRDEnergyLossError  = nullptr;

    std::vector<double>* fMRDTrackStartX = nullptr;
    std::vector<double>* fMRDTrackStartY = nullptr;
    std::vector<double>* fMRDTrackStartZ = nullptr;

    std::vector<double>* fMRDTrackStopX = nullptr;
    std::vector<double>* fMRDTrackStopY = nullptr;
    std::vector<double>* fMRDTrackStopZ = nullptr;

    std::vector<bool>* fMRDSide    = nullptr;
    std::vector<bool>* fMRDStop    = nullptr;
    std::vector<bool>* fMRDThrough = nullptr;

    std::vector<int>* fMRDClusterIndex = nullptr;

    Int_t fNumClusterTracks = 0;

    TTree* fTree = nullptr;
};

#endif
