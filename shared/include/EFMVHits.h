// EFMVHits.h
#ifndef FMVEVENTHITS_H
#define FMVEVENTHITS_H

#include <vector>
#include "FMVHit.h"

class EFMVHits {
public:
    void AddHit(const FMVHit& hit);
    const std::vector<FMVHit>& GetHits() const;
    void Clear();
    void Print() const;

private:
    std::vector<FMVHit> mHits;
};

#endif // FMVEVENTHITS_H
