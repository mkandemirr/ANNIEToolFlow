#ifndef EVENTMRDRECO_H
#define EVENTMRDRECO_H

#include <vector>
#include "MRDReco.h"

class EMRDRecos {
public:
    void AddTrack(const MRDReco& track);
    const std::vector<MRDReco>& Get() const;
    void Clear();
    void Print() const;

private:
    std::vector<MRDReco> mTracks;
};

#endif
