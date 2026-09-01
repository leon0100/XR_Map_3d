#pragma once

#include <QObject>
#include <QVector>
#include <QImage>
#include <QPoint>
#include <QPixmap>
#include <QPainter>
#include <QEvent>


#include "plot2D_aim.h"
#include "plot2D_defs.h"
#include "plot2D_echogram.h"
#include "plot2D_grid.h"
#include "dataset.h"
#include "data_processor.h"


class Plot2D
{
public:
    Plot2D();

    void setDataset(Dataset* dataset);
    void setDataProcessorPtr(DataProcessor* dataProcessorPtr);

    float getDepthByMousePos(int mouseX, int mouseY, bool isHorizontal) const;
    int getEpochIndxByMousePos(int mouseX, int mouseY, bool isHorizontal) const;
    QPoint getMousePosByDepthAndEpochIndx(float depth, int epochIndx, bool isHorizontal) const;

    bool getPlotEnabled() const;
    void setPlotEnabled(bool state);

    bool plotEnabled() const;

    bool isHorizontal();
    void setHorizontal(bool is_horizontal);

    void setAimEpochEventState(bool state);
    void setTimelinePosition(float position);
    void resetAim();

    void setTimelinePositionSec(float position);
    void setTimelinePositionByEpoch(int epochIndx);

    float timelinePosition();
    void scrollPosition(int columns);

    void setDataChannel(bool fromGui, const ChannelId& channel, uint8_t subChannel1, const QString& portName1, const ChannelId& channel2 = CHANNEL_NONE, uint8_t subChannel2 = 0, const QString& portName2 = QString());

    bool getImage(int width, int height, QPainter* painter, bool is_horizontal);
    void draw(QPainter* painterPtr);

    float getCursorDistance() const;
    std::tuple<ChannelId, uint8_t, QString> getSelectedChannelId(float cursorDistance = 0.0f) const;

    float getEchogramLowLevel() const;
    float getEchogramHighLevel() const;
    void setEchogramLowLevel(float low);
    void setEchogramHightLevel(float high);
    void setEchogramVisible(bool visible);

    void setBottomTrackVisible(bool visible);
    void setBottomTrackTheme(int theme_id);

    void setDistance(float from, float to);
    void scrollDistance(float ratio);

    void setMousePosition(int x, int y, bool isSync = false);
    void simpleSetMousePosition(int x, int y);
    void setMouseTool(MouseTool tool);

    Canvas& canvas();
    DatasetCursor& cursor();

    void resetCash();
    Canvas image(int width, int height);
    void reindexingCursor();

    void setRealtimeChannel(const ChannelId& channelId, uint8_t subChannelId);

    virtual void plotUpdate();
    virtual void sendSyncEvent(int epoch_index, QEvent::Type eventType);


public:
    float currentViewMaxLoRng_ = 1.0f;
    QString fromLonStr_, fromLatiStr_, toLonStr_, toLatiStr_;

protected:
    Canvas        canvas_;
    DatasetCursor cursor_;

    Plot2DAim               aim_;
    Plot2DEchogram          echogram_;
    Plot2DGrid              grid_;
    Dataset*                datasetPtr_ = nullptr;
    DataProcessor*          dataProcessorPtr_ = nullptr;
    bool isHorizontal_;

private:
    bool isEnabled_;
};
