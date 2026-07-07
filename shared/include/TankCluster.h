// TankCluster.h
#ifndef TANKCLUSTER_H
#define TANKCLUSTER_H

#include <vector>
#include "PMTHit.h"

class TankCluster {
public:
    void AddHit(const PMTHit& hit);
    const std::vector<PMTHit>& GetHits() const;
    void Print() const;
    
    
    void SetClusterCharge(double val);
    void SetClusterTime(double val);
    void SetClusterPE(double val);
    void SetMaxPE(double val);
    void SetChargePoint(double x, double y, double z);
    void SetChargeBalance(double val);

    double GetClusterCharge() const;
    double GetClusterTime() const;
    double GetClusterPE() const;
    double GetMaxPE() const;
    double GetChargePointX() const;
    double GetChargePointY() const;
    double GetChargePointZ() const;
    double GetChargeBalance() const;

private:
    std::vector<PMTHit> hits;
    double clusterCharge = 0;
    double clusterTime = 0;
    double clusterPE = 0;
    double maxPE = 0;
    double chargePointX = 0, chargePointY = 0, chargePointZ = 0;
    double chargeBalance = 0;
};

#endif // TANKCLUSTER_H
