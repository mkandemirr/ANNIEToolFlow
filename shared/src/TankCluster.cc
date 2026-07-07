// TankCluster.cpp
#include "TankCluster.h"

void TankCluster::AddHit(const PMTHit& hit) {
    hits.push_back(hit);
}

const std::vector<PMTHit>& TankCluster::GetHits() const {
    return hits;
}

void TankCluster::SetClusterCharge(double val) {
    clusterCharge = val;
}

void TankCluster::SetClusterTime(double val) {
    clusterTime = val;
}

void TankCluster::SetClusterPE(double val) {
    clusterPE = val;
}

void TankCluster::SetMaxPE(double val) {
    maxPE = val;
}

void TankCluster::SetChargePoint(double x, double y, double z) {
    chargePointX = x;
    chargePointY = y;
    chargePointZ = z;
}

void TankCluster::SetChargeBalance(double val) {
    chargeBalance = val;
}

double TankCluster::GetClusterCharge() const {
    return clusterCharge;
}

double TankCluster::GetClusterTime() const {
    return clusterTime;
}

double TankCluster::GetClusterPE() const {
    return clusterPE;
}

double TankCluster::GetMaxPE() const {
    return maxPE;
}

double TankCluster::GetChargePointX() const {
    return chargePointX;
}

double TankCluster::GetChargePointY() const {
    return chargePointY;
}

double TankCluster::GetChargePointZ() const {
    return chargePointZ;
}

double TankCluster::GetChargeBalance() const {
    return chargeBalance;
}




void TankCluster::Print() const {
    std::cout << "Cluster Info:\n";
    std::cout << "  Time: " << clusterTime << " ns\n";
    std::cout << "  Charge: " << clusterCharge << " pC\n";
    std::cout << "  PE: " << clusterPE << "\n";
    std::cout << "  Max PE: " << maxPE << "\n";
    std::cout << "  Charge Point: (" << chargePointX << ", "
              << chargePointY << ", " << chargePointZ << ")\n";
    std::cout << "  Charge Balance: " << chargeBalance << "\n";
    std::cout << "  Number of Hits: " << hits.size() << "\n";

   /*
    for (size_t i = 0; i < hits.size(); ++i) {
        std::cout << "    Hit " << i + 1 << "\n";
        hits[i].Print(); // Assuming PMTHit class has a Print() method
    }
    */
}
