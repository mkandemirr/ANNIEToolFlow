// EFMVHits.cpp
#include "EFMVHits.h"
#include <iostream>

void EFMVHits::AddHit(const FMVHit& hit) { mHits.push_back(hit); }

const std::vector<FMVHit>& EFMVHits::GetHits() const { return mHits; }

void EFMVHits::Clear() { mHits.clear(); }

void EFMVHits::Print() const {
    std::cout << "== FMV Hits (" << mHits.size() << ") ==" << std::endl;
    for (const auto& h : mHits) h.Print();
}
