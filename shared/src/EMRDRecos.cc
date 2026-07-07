#include "EMRDRecos.h"
#include <iostream>

void EMRDRecos::AddTrack(const MRDReco& track)
{
    mTracks.push_back(track);
}

const std::vector<MRDReco>& EMRDRecos::Get() const
{
    return mTracks;
}

void EMRDRecos::Clear()
{
    mTracks.clear();
}

void EMRDRecos::Print() const
{
    std::cout << "== MRD Reco Tracks ==" << std::endl;
    std::cout << "Number of reconstructed tracks in this event: "
              << mTracks.size() << std::endl;

    for (const auto& tr : mTracks)
        tr.Print();
}
