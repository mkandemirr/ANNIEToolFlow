#ifndef PMTEVENTHITS_H
#define PMTEVENTHITS_H

#include <vector>
#include "PMTHit.h"

class EPMTHits {
public:
    void AddHit(const PMTHit& hit);
    const std::vector<PMTHit>& GetHits() const;
    void Clear();
    void Print() const;

private:
    std::vector<PMTHit> mHits;
};

#endif // PMTEVENTHITS_H

