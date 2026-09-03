#pragma once

#include <math.h>
#include <stdint.h>
#include <time.h>
#include <QObject>
#include <QVector>
#include <QVector3D>
#include <QReadWriteLock>

#include "epoch.h"
#include "data_processor_defs.h"

class GraphicsScene3dView;
class Dataset : public QObject
{
    Q_OBJECT

public:
    enum class DatasetState {
        kUndefined = 0,
        kFile,
        kConnection
    };
    enum class LlaRefState {
        kUndefined = 0,
        kSettings,
        kFile,
        kConnection
    };

    Dataset();
    ~Dataset();

    void setState(DatasetState state);
    void setDataProcessorState(DataProcessorType dataProcessorState);
    DataProcessorType getDataProcessorState();

    DatasetState getState() const;
    LLARef getLlaRef() const;
    void   setLlaRef(const LLARef& val, LlaRefState state);

    QVector<Epoch>& getPool() {
        return pool_;
    }

    void removeFrames(int startIndex, int endIndex);

    QVector<Epoch>& getPolygonOutline() {
        return polygonOutline_;
    }

    inline int size() const {
        return pool_.size();
    }

    inline int polygonOutlineSize() const {
        return polygonOutline_.size();
    }

    Epoch* fromIndex(int index_offset = 0) {
        int index = validIndex(index_offset);
        if(index >= 0) {
            return &pool_[index];
        }

        return NULL;
    }

    Epoch* fromPolygonOutlineIndex(int index_offset = 0) {
        int index = validPolygonOutlineIndex(index_offset);
        if(index >= 0) {
            return &polygonOutline_[index];
        }

        return NULL;
    }

    void modifyPolygonOutline(int index,const North_East_Down& ned) {
        if(index >= 0 && index < polygonOutlineNED_.size()) {
            polygonOutlineNED_[index] = ned;
        }
    }

    Epoch fromIndexCopy(int index_offset = 0) {
        QReadLocker rl(&poolMtx_);

        const int index = validIndex(index_offset);
        if (channelsSetup_.empty() || index < 0) {
            return Epoch{};
        }

        const Epoch &src = pool_.at(index);
        Epoch copy = src;

        return copy;
    }

    Epoch* last() {
        if(size() > 0) {
            return fromIndex(endIndex());
        }
        return addNewEpoch();
    }

    Epoch* lastPolygonOutline() {
        if(polygonOutlineSize() > 0) {
            return fromPolygonOutlineIndex(endPolygonOutlineindex());
        }
        return addNewEpochPolygonOutline();
    }

    Epoch* lastlast() {
        if(size() > 1) {
            return fromIndex(endIndex()-1);
        }
        return NULL;
    }

    int endIndex() const {
        return size() - 1;
    }
    int endPolygonOutlineindex() const {
        return polygonOutlineSize() - 1;
    }

    int validIndex(int index_offset = 0) {
        int index = index_offset;
        if(index >= size()) { index = endIndex(); }
        else if(index < 0) { index = -1; }
        return index;
    }

    int validPolygonOutlineIndex(int index_offset = 0) {
        int index = index_offset;
        if(index >= polygonOutlineSize()) { index = endPolygonOutlineindex(); }
        else if(index < 0) { index = -1; }
        return index;
    }

    void getMaxDistanceRange(float* from, float* to, const ChannelId& channel, uint8_t subAddressCh1,
                            const ChannelId& channel2 = CHANNEL_NONE, uint8_t subAddressCh2 = 0);

    bool channelsListIsEmpty() const {
        QReadLocker locker(&lock_);
        return channelsSetup_.isEmpty();
    }

    QVector<DatasetChannel> channelsList() const {
        QReadLocker locker(&lock_);
        return channelsSetup_;
    }

    bool isContainsChannelInChannelSetup(const ChannelId& channelId) const {
        QReadLocker locker(&lock_);
        for (int16_t i = 0; i < channelsSetup_.size(); ++i) {
            if (channelsSetup_.at(i).channelId_ == channelId) {
                return true;
            }
        }
        return false;
    }

    int getLastBottomTrackEpoch() const;

    // float getLastYaw() {
    //     return _lastYaw;
    // }

    BottomTrackParam getBottomTrackParam() {
        QReadLocker rl(&lock_);

        return bottomTrackParam_;
    }

    BottomTrackParam* getBottomTrackParamPtr() {
        return &bottomTrackParam_;
    }

    BottomTrackParam& getBottomTrackParamRef() {
        return bottomTrackParam_;
    }

    std::tuple<ChannelId, uint8_t, QString> channelIdFromName(const QString& name) const;

    void addPolygonOutlineNED(const North_East_Down& ned) {
        polygonOutlineNED_.append(ned);
    }
    QVector<North_East_Down>& getPolygonOutlineNED() {
        return polygonOutlineNED_;
    }
    bool polygonNEDEmpty() {
        return polygonOutlineNED_.isEmpty();
    }

    void setAutoBounadry() {
        autoBoundary_.clear();
        autoBoundary_.append(QVector3D(minY_, minX_, 0));
        autoBoundary_.append(QVector3D(maxY_, minX_, 0));
        autoBoundary_.append(QVector3D(maxY_, maxX_, 0));
        autoBoundary_.append(QVector3D(minY_, maxX_, 0));
        autoBoundary_.append(QVector3D(minY_, minX_, 0));
    }
    void setAutoBounadry(QVector<QVector3D>& autoBoundary) {
        autoBoundary_ = autoBoundary;
    }
    QVector<QVector3D>& getAutoBounadry() {
        return autoBoundary_;
    }

public slots:
    friend class DataProcessor;
    void  onSonarPosCanCalc(uint64_t indx);
    void  setSonarOffset(float x, float y, float z);
    void  addChart(const ChannelId& channelId, const ChartParameters& chartParams, const QVector<QVector<uint8_t>>& data, bool enableRender);
    void  addChartMeta(const ChannelId& channelId, const ChartParameters& chartParams, bool enableRender);
    void  addPosition_realTime(double lat, double lon, double depth, bool isRead);
    void  addPosition_file(double lat, double lon, int depth, bool enableRender);

    void resetDataset();
    void resetPolygonOutline();
    void clearBoundary();

    void triggerRenderUpdate();
    void preallocatePool(int capacity);
    void setDiskSonarCache(class DiskSonarCache* cache) { diskSonarCache_ = cache; }
    DiskSonarCache* getDiskSonarCache() const { return diskSonarCache_; }

    void setRefPosition(int epoch_index);
    void setRefPosition(Epoch* ref_epoch);
    void setRefPosition(Position position);

    QStringList channelsNameList();

    void onLastBottomTrackEpochChanged(const ChannelId& channelId, int val, const BottomTrackParam& btP, bool manual, bool redrawAll);

signals:
    void epochAdded(uint64_t indx);
    void positionAdded(uint64_t indx);
    void chartAdded(uint64_t indx);
    void attitudeAdded(uint64_t indx);
    void dataUpdate();
    void bottomTrackUpdated(const ChannelId& channelId, int lEpoch, int rEpoch, bool manual, bool redrawAll);
    void updatedLlaRef();
    void locationToDest(LLA targetLla);
    void channelsUpdated();

    void signalDrawOutline(bool drawOutlineMode);


protected:
    int lastEventTimestamp = 0;
    int lastEventId = 0;
    float _lastEncoder = 0;

    DatasetChannel firstChannelId_ = DatasetChannel();
    QVector<DatasetChannel> channelsSetup_;

    void validateChannelList(const ChannelId& channelId, uint8_t subChannelId);

    QVector<Epoch> pool_;
    QVector<Epoch> polygonOutline_;
    QVector<North_East_Down> polygonOutlineNED_;

    // float _lastYaw = 0.0f, _lastPitch = 0.0f, _lastRoll = 0.0f;

public:
    Epoch* addNewEpoch();
    Epoch* addNewEpochPolygonOutline();
    void modifyPolygonOutlineEpoch(int index, const Epoch& epoch);
    bool isEmptyPolygon() {
        return polygonOutline_.isEmpty();
    }

    void location(double lat, double lon);

private:
    LlaRefState getCurrentLlaRefState() const;
    bool shouldAddNewEpoch(const ChannelId& channelId, uint8_t numSubChannels) const;
    void updateEpochWithChart(const ChannelId& channelId, const ChartParameters& chartParams,
                            const QVector<QVector<uint8_t>>& data, float resolution, float offset);

    mutable QReadWriteLock lock_;
    mutable QReadWriteLock poolMtx_ = QReadWriteLock(QReadWriteLock::Recursive);
    mutable QReadWriteLock polygonOutlineMtx_;

    LLARef _llaRef;
    LlaRefState llaRefState_ = LlaRefState::kUndefined;
    DatasetState state_ = DatasetState::kUndefined;
    int lastBottomTrackEpoch_;
    BottomTrackParam bottomTrackParam_;
    QMap<ChannelId, int> lastAddChartEpochIndx_;

    QList<QString> channelsNames_;
    QList<ChannelId> channelsIds_;
    QList<uint8_t> subChannelIds_;
    QVector3D sonarOffset_;
    uint64_t sonarPosIndx_;

public:
    QVector<float> vec_CSV_;
    void setDistProcesing_CSV(float depth) {
        vec_CSV_.append(-depth);
    }
    double getDistProccesing_CSV(int index) {
        if(index >= vec_CSV_.size()) {
            return 0.0;
        }
        return vec_CSV_.at(index);
    }
    double minDepth_, maxDepth_;
    QVector<QVector3D> autoBoundary_;
    DataProcessorType dataProcessorState_;

    float minX_ = std::numeric_limits<float>::max();
    float maxX_ = std::numeric_limits<float>::lowest();
    float minY_ = std::numeric_limits<float>::max();
    float maxY_ = std::numeric_limits<float>::lowest();

    DiskSonarCache* diskSonarCache_ = nullptr;
};
