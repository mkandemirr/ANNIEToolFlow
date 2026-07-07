#include "ANNIEUtility.h"

namespace
{
    constexpr double tankCenterX = 0.0;     // m
    constexpr double tankCenterY = -0.144;  // m
    constexpr double tankCenterZ = 1.681;   // m
}

double ANNIEUtility::GetMaxPE(
    TankClusterReader& tankClusterReader,
    Long64_t entry,
    double minTime,
    double maxTime)
{
    ETankClusters eTankClusters;
    tankClusterReader.ReadEntry(entry, eTankClusters);

    double maxPE = -1.0;

    for (const TankCluster& cluster : eTankClusters.Get())
    {
        const double t = cluster.GetClusterTime();

        if (t >= minTime && t <= maxTime)
        {
            if (cluster.GetMaxPE() > maxPE)
            {
                maxPE = cluster.GetMaxPE();
            }
        }
    }

    return maxPE;
}

double ANNIEUtility::GetTotalPE(
    TankClusterReader& tankClusterReader,
    Long64_t entry,
    double minTime,
    double maxTime)
{
    ETankClusters eTankClusters;
    tankClusterReader.ReadEntry(entry, eTankClusters);

    double totalPE = 0.0;

    for (const TankCluster& cluster : eTankClusters.Get())
    {
        const double t = cluster.GetClusterTime();

        if (t >= minTime && t <= maxTime)
        {
            for (const PMTHit& hit : cluster.GetHits())
            {
                totalPE += hit.GetPE();
            }
        }
    }

    return totalPE;
}


double ANNIEUtility::GetBestClusterPE(
    TankClusterReader& tankClusterReader,
    Long64_t entry,
    double minTime,
    double maxTime)
{
    ETankClusters eTankClusters;
    tankClusterReader.ReadEntry(entry, eTankClusters);

    const TankCluster* bestCluster = nullptr;
    double maxClusterPE = -1.0;

    for (const TankCluster& cluster : eTankClusters.Get())
    {
        const double t = cluster.GetClusterTime();

        if (t < minTime || t > maxTime) continue;

        if (cluster.GetClusterPE() > maxClusterPE)
        {
            maxClusterPE = cluster.GetClusterPE();
            bestCluster = &cluster;
        }
    }

    if (!bestCluster) return -1.0;

    double totalPE = 0.0;

    for (const PMTHit& hit : bestCluster->GetHits())
    {
        totalPE += hit.GetPE();
    }

    return totalPE;
}


void ANNIEUtility::TankToGlobal(
    double& x,
    double& y,
    double& z)
{
    x += tankCenterX;
    y += tankCenterY;
    z += tankCenterZ;
}


//////////////////
void ANNIEUtility::GlobalToTank(
    double& x,
    double& y,
    double& z)
{
    x -= tankCenterX;
    y -= tankCenterY;
    z -= tankCenterZ;
}


