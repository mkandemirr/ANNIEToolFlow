#ifndef ANNIEUTILITY_H
#define ANNIEUTILITY_H

#include "TankClusterReader.h"
#include "ETankClusters.h"

class ANNIEUtility
{
public:

    static double GetMaxPE(
        TankClusterReader& tankClusterReader,
        Long64_t entry,
        double minTime,
        double maxTime);

    static double GetTotalPE(
        TankClusterReader& tankClusterReader,
        Long64_t entry,
        double minTime,
        double maxTime);
        
   static double GetBestClusterPE(
    TankClusterReader& tankClusterReader,
    Long64_t entry,
    double minTime,
    double maxTime);     
    
    static void TankToGlobal(
        double& x,
        double& y,
        double& z);

    static void GlobalToTank(
        double& x,
        double& y,
        double& z);
        
};

#endif




