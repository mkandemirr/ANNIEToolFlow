#include "EPMTHits.h"
#include <iostream>

void EPMTHits::AddHit(const PMTHit& hit) {
    mHits.push_back(hit);
}

const std::vector<PMTHit>& EPMTHits::GetHits() const {
    return mHits;
}

void EPMTHits::Clear() {
    mHits.clear();
}

void EPMTHits::Print() const {
    std::cout << "== PMT Hits ==" << std::endl;
    std::cout << "Number of PMT Hits: " << mHits.size() << std::endl;

    for (const auto& hit : mHits) {
        std::cout << "------------------------" << std::endl;
        hit.Print();  // Requires PMTHit to have a Print() method
    }
}

