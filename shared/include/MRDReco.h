#ifndef MRDRECO_H
#define MRDRECO_H

class MRDReco {
public:
    MRDReco() = default;
    
    MRDReco(
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
    );
    
    void Print() const;

    int GetClusterIndex() const { return mClusterIndex; }

    double GetTrackAngle() const { return mTrackAngle; }
    double GetTrackAngleError() const { return mTrackAngleError; }
    double GetPenetrationDepth() const { return mPenetrationDepth; }
    double GetTrackLength() const { return mTrackLength; } //return cm
    double GetEntryPointRadius() const { return mEntryPointRadius; }
    double GetEnergyLoss() const { return mEnergyLoss; }
    double GetEnergyLossError() const { return mEnergyLossError; }

    //in units of meter
    double GetStartX() const { return mStartX; }
    double GetStartY() const { return mStartY; }
    double GetStartZ() const { return mStartZ; }

    //in units of meter
    double GetStopX() const { return mStopX; }
    double GetStopY() const { return mStopY; }
    double GetStopZ() const { return mStopZ; }

    bool IsSide() const { return mSide; }
    bool IsStop() const { return mStop; }
    bool IsThrough() const { return mThrough; }

private:
    int mClusterIndex = -1;

    double mTrackAngle = 0;
    double mTrackAngleError = 0;
    double mPenetrationDepth = 0;
    double mTrackLength = 0;
    double mEntryPointRadius = 0;
    double mEnergyLoss = 0;
    double mEnergyLossError = 0;

    double mStartX = 0;
    double mStartY = 0;
    double mStartZ = 0;

    double mStopX = 0;
    double mStopY = 0;
    double mStopZ = 0;

    bool mSide = false;
    bool mStop = false;
    bool mThrough = false;
};

#endif
