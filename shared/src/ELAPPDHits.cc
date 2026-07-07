#include "ELAPPDHits.h"
#include <iostream>

void ELAPPDHits::AddHit(const LAPPDHit& hit) {
    mHits.push_back(hit);
}

const std::vector<LAPPDHit>& ELAPPDHits::GetHits() const {
    return mHits;
}

void ELAPPDHits::Clear() {
    mHits.clear();
}

void ELAPPDHits::Print() const {
    std::cout << "== LAPPD Hits ==" << std::endl;
    std::cout << "Number of LAPPD Hits: " << mHits.size() << std::endl;
    
    for (const auto& hit : mHits) {
      std::cout << "------------------------" << std::endl;
      hit.Print();  // hit.Print() fonksiyonunun LAPPDHit içinde tanımlı olması gerekiyor.
    }
}


