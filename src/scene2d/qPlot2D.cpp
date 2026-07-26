#include "qPlot2D.h"

#include <QMutex>
#include <QPixmap>
#include <QPainter>
#include <QSGSimpleTextureNode>
#include <QQuickWindow>
#include "epoch_event.h"
#include "console.h"



qPlot2D::qPlot2D(QQuickItem* parent) : QQuickPaintedItem(parent), m_updateTimer(new QTimer(this))
{
    qRegisterMetaType<ChannelId>("ChannelId");
    qRegisterMetaType<DatasetChannel>("DatasetChannel");

    m_updateTimer->start(30);
    setFlag(ItemHasContents);
    setAcceptedMouseButtons(Qt::AllButtons);

    _isHorizontal = false;
}

void qPlot2D::paint(QPainter *painter)
{
    // qDebug() << "qPlot2D::paint(.........." << Plot2D::plotEnabled();
    if (!Plot2D::plotEnabled() || dataset_ == nullptr) {
        return;
    }

    int totalWidth = static_cast<int>(width());
    int waveWidth = totalWidth / WAVE_WIDTH_RATIO_DENOM;
    if(waveWidth < 1) {
        waveWidth = 1;
    }

    Plot2D::getImage(totalWidth - waveWidth, (int)height(), painter, _isHorizontal);
    Plot2D::draw(painter);
    if (Plot2D::getIsContactChanged()) {
        emit contactChanged();
    }
}

void qPlot2D::setPlot(Dataset *dataset) {
    if(dataset == nullptr) { return; }
    dataset_ = dataset;
    setDataset(dataset);
    connect(dataset, &Dataset::dataUpdate, this, &qPlot2D::dataUpdate);
    // connect(dataset, &Dataset::updateMinMaxLoRng, this, &qPlot2D::updateMinMaxLoRng);
}

void qPlot2D::setDataProcessor(DataProcessor *dataProcessorPtr)
{
    if (!dataProcessorPtr) {
        return;
    }

    Plot2D::setDataProcessorPtr(dataProcessorPtr);
}

void qPlot2D::plotUpdate()
{
    // qDebug() << "qPlot2D::plotUpdate()..........";
    if (!Plot2D::plotEnabled()) {
        return;
    }

    static QMutex mutex;
    if(!mutex.tryLock()) {
        return;
    }
    emit timelinePositionChanged();

    update();

    if(dataset_ && !dataset_->vec_CSV_.empty()) {
        tempViewMaxLoRng_ = (int)(currentViewMaxLoRng_ * 1.5f);
        setMaxLoRng(tempViewMaxLoRng_);
    }

    mutex.unlock();
}

bool qPlot2D::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);

    if (event->type() == EpochSelected3d) {
        auto epochEvent = static_cast<EpochEvent*>(event);
        //qDebug() << QString("[Plot 2d]: catched event from 3d view (epoch index is %1)").arg(epochEvent->epochIndex());
        setAimEpochEventState(true);
        setTimelinePositionByEpoch(epochEvent->epochIndex());
    }
    return false;
}

void qPlot2D::sendSyncEvent(int epoch_index, QEvent::Type eventType)
{
    //qDebug() << "qPlot2D::sendSyncEvent: epoch_index: " << epoch_index;
    if (eventType == EpochSelected2d) {
        cursor_.selectEpochIndx = -1;
    }

    auto epochEvent = new EpochEvent(eventType, datasetPtr_->fromIndex(epoch_index), epoch_index, DatasetChannel(cursor_.channel1, cursor_.subChannel1));
    QCoreApplication::postEvent(this, epochEvent);
}

void qPlot2D::horScrollEvent(int delta)
{
    cursor_.selectEpochIndx = -1;

    if(_isHorizontal) {
        scrollPosition(-delta);
    } else {
        scrollPosition(delta);
    }
}

void qPlot2D::verZoomEvent(int delta)
{
    zoomDistance(delta);
}

void qPlot2D::verScrollEvent(int delta)
{
    scrollDistance(delta);
}

void qPlot2D::plotMouseTool(int mode)
{
    setMouseTool((MouseTool)mode);
}

bool qPlot2D::setContact(int indx, const QString& text)
{
    return Plot2D::setContact(indx, text);
}

bool qPlot2D::setActiveContact(int indx)
{
    return Plot2D::setActiveContact(indx);
}

int qPlot2D::getMinUpRng()
{
    return currentUpRng_;
}

int qPlot2D::getMaxLoRng()
{
    return currentLoRng_;
}

void qPlot2D::setMinUpRng(int minUpRng)
{
    currentUpRng_ = minUpRng;
    grid_.setLoRngRange(currentUpRng_, currentLoRng_);
    echogram_.setUpperRng(minUpRng);
    emit minUpRngChanged();
    plotUpdate();
}

void qPlot2D::setMaxLoRng(int maxLoRng)
{
    currentLoRng_ = maxLoRng;
    grid_.setLoRngRange(currentUpRng_, currentLoRng_);
    echogram_.setLowerRng(maxLoRng);
    emit maxLoRngChanged();
    plotUpdate();
}

void qPlot2D::setSoundVelocity(int soundVelocity, int draftOffset)
{
    echogram_.setSoundVelocity(soundVelocity, draftOffset);
    plotUpdate();
}

void qPlot2D::setDepthFilterVisible(bool visible, int value)
{
    echogram_.setDepthFilterVisible(visible, value);
    plotUpdate();
}

void qPlot2D::setBatchCorrect(bool batch)
{
    echogram_.setBatchCorrect(batch);
}

void qPlot2D::setDepthCorrect(bool depthCorrect)
{
    echogram_.setDepthCorrect(depthCorrect);
}
void qPlot2D::resetUpLoRng(int upper, int lower)
{
    qDebug() << "upper........" << upper << "  " << lower;

    currentUpRng_ = upper * 100;
    currentLoRng_ = lower * 100;
    setMinUpRng(currentUpRng_);
    setMaxLoRng(currentLoRng_);
    grid_.setLoRngRange(currentUpRng_, currentLoRng_);
    echogram_.setUpperRng(currentUpRng_);
    echogram_.setLowerRng(currentLoRng_);

    plotUpdate();
}

void qPlot2D::setSensitivity(int sensitive)
{
    echogram_.setSensitivity(sensitive);
    plotUpdate();

}


bool qPlot2D::deleteContact(int indx)
{
    return Plot2D::deleteContact(indx);
}

void qPlot2D::updateContact()
{
    Plot2D::updateContact();
}

void qPlot2D::setBottomLineVisible(bool isVisible)
{
    echogram_.setBottomLineVisible(isVisible);
    plotUpdate();
}

void qPlot2D::drawBatchCorrect(int x, int y)
{
    echogram_.addBatchCorrect(QPoint(x, y));
    plotUpdate();
}

void qPlot2D::clearBatchCorrect()
{
    echogram_.clearBatchCorrect();
    plotUpdate();
}

void qPlot2D::updateBatchCorrect()
{
    if(prompt_) {
        GIF->dialogCheck(tr("Confirm to Clear Isobaths?"),[this](bool confirmed, bool prompt) {
            if(confirmed) {
                prompt_ = !prompt;
                echogram_.setUpdateBatchCorrect(true);
                plotUpdate();
            }
        }, tr("Don't prompt again"));
    }
    else {
        echogram_.setUpdateBatchCorrect(true);
        plotUpdate();
    }
}

void qPlot2D::drawDepthCorrect(int x, int y)
{
    qDebug() << "x........" << x << "  y:" << y;
    if (dataset_ == nullptr) return;
    int totalWidth = static_cast<int>(width());
    int waveWidth = totalWidth / WAVE_WIDTH_RATIO_DENOM;
    int imageWidth = totalWidth - waveWidth;
    int h = static_cast<int>(height());
    echogram_.applyDepthCorrect(this, dataset_, x, y, imageWidth, h);
}

void qPlot2D::setMarkDistTimeVisible(bool visible, int dist0time1, int distInterval, int timeInterval,
                                bool isFrame, bool isTime, bool isDepth, bool isCoordinate)
{
    echogram_.setMarkDistTimeVisible(visible, dist0time1, distInterval, timeInterval,isFrame, isTime, isDepth, isCoordinate);
    plotUpdate();
}

float qPlot2D::getLowEchogramLevel() const
{
    return Plot2D::getEchogramLowLevel();
}

float qPlot2D::getHighEchogramLevel() const
{
    return Plot2D::getEchogramHighLevel();
}

int qPlot2D::getThemeId() const
{
    return Plot2D::getThemeId();
}

void qPlot2D::drawPolygonOutline(bool outlineMode)
{
    if(datasetPtr_ != nullptr) {
       emit datasetPtr_->signalDrawOutline(outlineMode);
    }
}


void qPlot2D::doDistProcessing(int preset, int window_size, float vertical_gap, float range_min, float range_max,
                        float gain_slope, float threshold, float offsetx, float offsety, float offsetz, bool manual) {
    // qDebug() << "开始绘制等高线 qPlot2D::doDistProcessing.........";
    if (datasetPtr_ != nullptr) {
        QVector<Epoch> pool = datasetPtr_->getPool();
        if(pool.isEmpty()) {
            GIF->dialogInfo(Dialog_OK, tr("No Track Data Found!"));
            return;
        }
        if (auto btpPtr = datasetPtr_->getBottomTrackParamPtr(); btpPtr) {
            btpPtr->preset      = static_cast<BottomTrackPreset>(preset);
            btpPtr->gainSlope   = gain_slope;
            btpPtr->threshold   = threshold;
            btpPtr->windowSize  = window_size;
            btpPtr->verticalGap = vertical_gap;
            btpPtr->minDistance = range_min;
            btpPtr->maxDistance = range_max;
            btpPtr->indexFrom   = 0;
            btpPtr->indexTo     = datasetPtr_->size();
            btpPtr->offset.x    = offsetx;
            btpPtr->offset.y    = offsety;
            btpPtr->offset.z    = offsetz;

            QMetaObject::invokeMethod(dataProcessorPtr_, "bottomTrackProcessing", Qt::QueuedConnection,
                            Q_ARG(DatasetChannel, DatasetChannel(cursor_.channel1, cursor_.subChannel1)),
                            Q_ARG(DatasetChannel, DatasetChannel(cursor_.channel2, cursor_.subChannel2)),
                            Q_ARG(BottomTrackParam, *btpPtr), Q_ARG(bool, manual), Q_ARG(bool, true));
        }
    }
    // plotUpdate();
}

void qPlot2D::refreshDistParams(int preset, int windowSize, float verticalGap, float rangeMin, float rangeMax, float gainSlope, float threshold, float offsetX, float offsetY, float offsetZ)
{
    auto btPRefreshFunc = [this, preset, windowSize, verticalGap, rangeMin, rangeMax, gainSlope, threshold, offsetX, offsetY, offsetZ]() {
        if (datasetPtr_) {
            if (auto btpPtr =datasetPtr_->getBottomTrackParamPtr(); btpPtr) {
                btpPtr->preset = static_cast<BottomTrackPreset>(preset);
                btpPtr->gainSlope = gainSlope;
                btpPtr->threshold = threshold;
                btpPtr->windowSize = windowSize;
                btpPtr->verticalGap = verticalGap;
                btpPtr->minDistance = rangeMin;
                btpPtr->maxDistance = rangeMax;
                btpPtr->indexFrom = 0;
                btpPtr->indexTo = datasetPtr_->size();
                btpPtr->offset.x = offsetX;
                btpPtr->offset.y = offsetY;
                btpPtr->offset.z = offsetZ;
            }
        }
    };

    if (!datasetPtr_) {
        // pendingBtpLambda_ = btPRefreshFunc;
    } else {
        btPRefreshFunc();
    }
}

void qPlot2D::setPreset(int value)
{
    if (datasetPtr_) {
        if (auto* btpPtr =datasetPtr_->getBottomTrackParamPtr(); btpPtr) {
            btpPtr->preset = static_cast<BottomTrackPreset>(value);
        }
    }
}

void qPlot2D::setWindowSize(int value)
{
    if (datasetPtr_) {
        if (auto* btpPtr =datasetPtr_->getBottomTrackParamPtr(); btpPtr) {
            btpPtr->windowSize = value;
        }
    }
}

void qPlot2D::setVerticalGap(float value)
{
    if (datasetPtr_) {
        if (auto* btpPtr =datasetPtr_->getBottomTrackParamPtr(); btpPtr) {
            btpPtr->verticalGap = value;
        }
    }
}

void qPlot2D::setRangeMin(float value)
{
    if (datasetPtr_) {
        if (auto* btpPtr =datasetPtr_->getBottomTrackParamPtr(); btpPtr) {
            btpPtr->minDistance = value;
        }
    }
}

void qPlot2D::setRangeMax(float value)
{
    if (datasetPtr_) {
        if (auto* btpPtr =datasetPtr_->getBottomTrackParamPtr(); btpPtr) {
            btpPtr->maxDistance = value;
        }
    }
}

void qPlot2D::setGainSlope(float value)
{
    if (datasetPtr_) {
        if (auto* btpPtr =datasetPtr_->getBottomTrackParamPtr(); btpPtr) {
            btpPtr->gainSlope = value;
        }
    }
}

void qPlot2D::setThreshold(float value)
{
    if (datasetPtr_) {
        if (auto* btpPtr =datasetPtr_->getBottomTrackParamPtr(); btpPtr) {
            btpPtr->threshold = value;
        }
    }
}

void qPlot2D::setOffsetX(float value)
{
    if (datasetPtr_) {
        if (auto* btpPtr =datasetPtr_->getBottomTrackParamPtr(); btpPtr) {
            btpPtr->offset.x = value;
        }
    }
}

void qPlot2D::setOffsetY(float value)
{
    if (datasetPtr_) {
        if (auto* btpPtr =datasetPtr_->getBottomTrackParamPtr(); btpPtr) {
            btpPtr->offset.y = value;
        }
    }
}

void qPlot2D::setOffsetZ(float value)
{
    if (datasetPtr_) {
        if (auto* btpPtr =datasetPtr_->getBottomTrackParamPtr(); btpPtr) {
            btpPtr->offset.z = value;
        }
    }
}

void qPlot2D::scaleYZoomEvent(int delta)
{
    if(delta < 0) {
        currentLoRng_ /= 2;
    }
    else if(delta > 0) {
        currentLoRng_ *= 2;
    }

    if(currentLoRng_ < 100) {
        currentLoRng_ = 100;
    }
    else if(currentLoRng_ > 51200) {
        currentLoRng_ = 51200;
    }

    echogram_.setLowerRng(currentLoRng_);
    setMaxLoRng(currentLoRng_);
}

void qPlot2D::plotMousePosition(int x, int y, bool isSync)
{
    setAimEpochEventState(false);
    if(_isHorizontal) {
        setMousePosition(x, y, isSync);
    }
    else {
        if(x >= 0 && y >= 0) {
            setMousePosition(height() - y, x, isSync);
        } else {
            setMousePosition(-1, -1, isSync);
        }

    }
}

void qPlot2D::simplePlotMousePosition(int x, int y)
{
    Plot2D::setAimEpochEventState(false);

    if(_isHorizontal) {
        Plot2D::simpleSetMousePosition(x, y);
    }
    else {
        if(x >=0 && y >= 0) {
            Plot2D::simpleSetMousePosition(height() - y, x);
        }
        else {
            Plot2D::simpleSetMousePosition(-1, -1);
        }
    }
}

void qPlot2D::onCursorMoved(int x, int y)
{
    Plot2D::onCursorMoved(x, y);
}

void qPlot2D::timerUpdater()
{
    if(m_needUpdate) {
        m_needUpdate = false;
       update();
    }
}

void qPlot2D::dataUpdate()
{
    setMinUpRng(0);
    setMaxLoRng(3200);
    // plotUpdate();
}

void qPlot2D::updater()
{
    m_needUpdate = true;
}


