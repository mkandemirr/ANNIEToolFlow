#include "MRDReco.h"
#include <iostream>

MRDReco::MRDReco(
    int clusterIndex,
    double trackAngle,
    double trackAngleError,
    double penetrationDepth,
    double trackLength,
    double entryPointRadius,
    double energyLoss,
    double energyLossError,
    double startX,
    double startY,
    double startZ,
    double stopX,
    double stopY,
    double stopZ,
    bool side,
    bool stop,
    bool through
)
    : mClusterIndex(clusterIndex),
      mTrackAngle(trackAngle),
      mTrackAngleError(trackAngleError),
      mPenetrationDepth(penetrationDepth),
      mTrackLength(trackLength),
      mEntryPointRadius(entryPointRadius),
      mEnergyLoss(energyLoss),
      mEnergyLossError(energyLossError),
      mStartX(startX),
      mStartY(startY),
      mStartZ(startZ),
      mStopX(stopX),
      mStopY(stopY),
      mStopZ(stopZ),
      mSide(side),
      mStop(stop),
      mThrough(through)
{
}


void MRDReco::Print() const
{
    std::cout << "MRDReco:" << std::endl;

    std::cout << "  ClusterIndex     : " << mClusterIndex << std::endl;

    std::cout << "  TrackAngle       : " << mTrackAngle << std::endl;
    std::cout << "  TrackAngleError  : " << mTrackAngleError << std::endl;

    std::cout << "  PenetrationDepth : " << mPenetrationDepth << std::endl;
    std::cout << "  TrackLength      : " << mTrackLength << std::endl;

    std::cout << "  EntryPointRadius : " << mEntryPointRadius << std::endl;

    std::cout << "  EnergyLoss       : " << mEnergyLoss << std::endl;
    std::cout << "  EnergyLossError  : " << mEnergyLossError << std::endl;

    std::cout << "  StartX           : " << mStartX << std::endl;
    std::cout << "  StartY           : " << mStartY << std::endl;
    std::cout << "  StartZ           : " << mStartZ << std::endl;

    std::cout << "  StopX            : " << mStopX << std::endl;
    std::cout << "  StopY            : " << mStopY << std::endl;
    std::cout << "  StopZ            : " << mStopZ << std::endl;

    std::cout << "  Side             : " << mSide << std::endl;
    std::cout << "  Stop             : " << mStop << std::endl;
    std::cout << "  Through          : " << mThrough << std::endl;
}
