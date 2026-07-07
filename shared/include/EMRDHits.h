// EMRDHits.h
#ifndef MRDEVENTHITS_H
#define MRDEVENTHITS_H

#include <vector>
#include "MRDHit.h"

class EMRDHits {
public:
    void AddHit(const MRDHit& hit);
    const std::vector<MRDHit>& GetHits() const;
    void Clear();
    void Print() const;

private:
    std::vector<MRDHit> mHits;
};

#endif // MRDEVENTHITS_H
