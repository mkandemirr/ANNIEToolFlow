#include "MRDRecoReader.h"
#include "MRDReco.h"

#include <iostream>
#include <algorithm>

void MRDRecoReader::Init(TTree* tree)
{
    if (!tree) {
        std::cerr << "[MRDRecoReader] Error: Input TTree pointer is null!" << std::endl;
        return;
    }

    fTree = tree;

    auto checkBranch = [&](const char* name, void* addr) {
        TBranch* b = fTree->GetBranch(name);
        if (!b) {
            std::cerr << "Warning: Branch '" << name << "' not found." << std::endl;
        } else {
            fTree->SetBranchAddress(name, addr);
            if (b->GetEntries() == 0) {
                std::cerr << "Warning: Branch '" << name << "' has no entries." << std::endl;
            }
        }
    };

    checkBranch("NumClusterTracks",     &fNumClusterTracks);
    checkBranch("MRDClusterIndex",      &fMRDClusterIndex);

    checkBranch("MRDTrackAngle",        &fMRDTrackAngle);
    checkBranch("MRDTrackAngleError",   &fMRDTrackAngleError);
    checkBranch("MRDPenetrationDepth",  &fMRDPenetrationDepth);
    checkBranch("MRDTrackLength",       &fMRDTrackLength);
    checkBranch("MRDEntryPointRadius",  &fMRDEntryPointRadius);
    checkBranch("MRDEnergyLoss",        &fMRDEnergyLoss);
    checkBranch("MRDEnergyLossError",   &fMRDEnergyLossError);

    checkBranch("MRDTrackStartX",       &fMRDTrackStartX);
    checkBranch("MRDTrackStartY",       &fMRDTrackStartY);
    checkBranch("MRDTrackStartZ",       &fMRDTrackStartZ);

    checkBranch("MRDTrackStopX",        &fMRDTrackStopX);
    checkBranch("MRDTrackStopY",        &fMRDTrackStopY);
    checkBranch("MRDTrackStopZ",        &fMRDTrackStopZ);

    checkBranch("MRDSide",              &fMRDSide);
    checkBranch("MRDStop",              &fMRDStop);
    checkBranch("MRDThrough",           &fMRDThrough);
}

void MRDRecoReader::ReadEntry(Long64_t entry, EMRDRecos& eventRecos)
{
    if (!fTree) return;

    fTree->GetEntry(entry);
    eventRecos.Clear();

    if (!fMRDTrackAngle) return;

    size_t nTracks = fMRDTrackAngle->size();

    for (size_t i = 0; i < nTracks; ++i) {

        MRDReco track(
            fMRDClusterIndex->at(i),

            fMRDTrackAngle->at(i),
            fMRDTrackAngleError->at(i),

            fMRDPenetrationDepth->at(i),
            fMRDTrackLength->at(i),
            fMRDEntryPointRadius->at(i),

            fMRDEnergyLoss->at(i),
            fMRDEnergyLossError->at(i),

            fMRDTrackStartX->at(i),
            fMRDTrackStartY->at(i),
            fMRDTrackStartZ->at(i),

            fMRDTrackStopX->at(i),
            fMRDTrackStopY->at(i),
            fMRDTrackStopZ->at(i),

            fMRDSide->at(i),
            fMRDStop->at(i),
            fMRDThrough->at(i)
        );

        eventRecos.AddTrack(track);
    }
}
