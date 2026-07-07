#ifndef LAPPDEVENTHITS_H
#define LAPPDEVENTHITS_H

#include <vector>
#include "LAPPDHit.h"

class ELAPPDHits {
public:
    void AddHit(const LAPPDHit& hit);
    const std::vector<LAPPDHit>& GetHits() const;
    void Clear();
    void Print() const;

private:
    std::vector<LAPPDHit> mHits;
};

#endif // LAPPDEVENTHITS_H

