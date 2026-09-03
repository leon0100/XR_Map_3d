#include "data_horizon.h"

#include <QDebug>


DataHorizon::DataHorizon() :
    QObject(),
    emitChanges_(true),
    isFileOpening_(false),
    isSeparateReading_(false),
    isAttitudeExpected_(false),
    epochIndx_(0),
    positionIndx_(0),
    chartIndx_(0),
    attitudeIndx_(0),
    bottomTrackIndx_(0),
    mosaicIndx_(0),
    sonarIndx_(0)
{
    qRegisterMetaType<uint64_t>("uint64_t");
}

void DataHorizon::clear()
{
    isFileOpening_ = false;

    epochIndx_ = 0;
    positionIndx_ = 0;
    chartIndx_ = 0;
    attitudeIndx_ = 0;
    bottomTrackIndx_ = 0;
    mosaicIndx_ = 0;
    sonarIndx_ = 0;
}

void DataHorizon::onAddedEpoch(uint64_t indx)
{
    //qDebug() << "DataHorizon::onAddedEpoch" << indx;
    bool beenChanged = epochIndx_ != indx;

    epochIndx_ = indx;

    if (canEmitHorizon(beenChanged)) {
        emit epochAdded(indx);
    }
}

void DataHorizon::onAddedPosition(uint64_t indx)
{
    // qDebug() << "DataHorizon::onAddedPosition indx: " << indx;
    bool beenChanged = positionIndx_ != indx;

    positionIndx_ = indx;

    if (canEmitHorizon(beenChanged)) {
        // qDebug() << "positionIndx_: isisisis   " << positionIndx_;
        emit positionAdded(positionIndx_);
        tryCalcAndEmitSonarPosIndx();
    }
}

void DataHorizon::onAddedChart(uint64_t indx)
{
    //qDebug() << "DataHorizon::onAddedChart" << indx;
    bool beenChanged = indx != chartIndx_; // TODO: delete this (fix on processing)

    chartIndx_ = indx;

    if (canEmitHorizon(beenChanged)) {
        emit chartAdded(chartIndx_);
        tryCalcAndEmitMosaicIndx();
    }
}

void DataHorizon::onAddedAttitude(uint64_t indx)
{
    bool beenChanged = attitudeIndx_ != indx;

    attitudeIndx_ = indx;

    if (canEmitHorizon(beenChanged)) {
        emit attitudeAdded(attitudeIndx_);
        tryCalcAndEmitSonarPosIndx();
        tryCalcAndEmitMosaicIndx();
    }
}


void DataHorizon::onAddedBottomTrack3D(const QVector<int>& epIndxs, const QVector<int>& vertIndx, bool isManual)
{
    qDebug() << "onAddedBottomTrack3D.............";
    bool beenChanged = true;

    if (canEmitHorizon(beenChanged)) {
        emit bottomTrack3DAdded(epIndxs, vertIndx, isManual);
    }
}

bool DataHorizon::canEmitHorizon(bool beenChanged) const
{
    bool retVal = false;

    if (!emitChanges_) {
        return retVal;
    }

    if (isSeparateReading_) {
        if (beenChanged) {
            retVal = true;
        }
    }
    else {
        if (!isFileOpening_ && beenChanged) {
            retVal = true;
        }
    }

    return retVal;
}

void DataHorizon::tryCalcAndEmitMosaicIndx()
{
    uint64_t minMosaicHorizon = std::min(std::min(bottomTrackIndx_, chartIndx_), attitudeIndx_);
    if (minMosaicHorizon > mosaicIndx_) {
        mosaicIndx_ = minMosaicHorizon;
        emit mosaicCanCalc(mosaicIndx_);
    }
}

void DataHorizon::tryCalcAndEmitSonarPosIndx()
{
    uint64_t minSonarIndx = isAttitudeExpected_ ? std::min(positionIndx_, attitudeIndx_) : positionIndx_;
    if (minSonarIndx > sonarIndx_) {
        sonarIndx_ = minSonarIndx;
        emit sonarPosCanCalc(sonarIndx_);
    }
}
