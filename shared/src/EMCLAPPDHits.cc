#include "EMCLAPPDHits.h"
#include <iostream>

void EMCLAPPDHits::AddHit(const MCLAPPDHit& hit) {
    mHits.push_back(hit);
}

const std::vector<MCLAPPDHit>& EMCLAPPDHits::GetHits() const {
    return mHits;
}

void EMCLAPPDHits::Clear() {
    mHits.clear();
}

void EMCLAPPDHits::Print() const {
    std::cout << "== MC LAPPD Hits ==\n";
    std::cout << "Number of Hits: " << mHits.size() << "\n";
    for (const auto& hit : mHits) {
        std::cout << "------------------------" << std::endl;
        hit.Print();
    }
}

