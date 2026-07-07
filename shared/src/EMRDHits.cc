// EMRDHits.cpp
#include "EMRDHits.h"
#include <iostream>

void EMRDHits::AddHit(const MRDHit& hit) { mHits.push_back(hit); }

const std::vector<MRDHit>& EMRDHits::GetHits() const { return mHits; }

void EMRDHits::Clear() { mHits.clear(); }

void EMRDHits::Print() const {
    std::cout << "== MRD Hits (" << mHits.size() << ") ==" << std::endl;
    for (const auto& h : mHits) h.Print();
}
