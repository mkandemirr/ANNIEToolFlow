#ifndef EMCLAPPDHits_H
#define EMCLAPPDHits_H

#include <vector>
#include "MCLAPPDHit.h"

class EMCLAPPDHits {
public:
    void AddHit(const MCLAPPDHit& hit);
    const std::vector<MCLAPPDHit>& GetHits() const;
    void Clear();
    void Print() const;

private:
    std::vector<MCLAPPDHit> mHits;
};

#endif // EMCLAPPDHits_H

