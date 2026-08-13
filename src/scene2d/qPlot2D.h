#pragma once

#include <QImage>
#include <QQuickPaintedItem>
#include <QObject>
#include <dataset.h>
#include <QTimer>
#include "plot2D.h"


class qPlot2D : public QQuickPaintedItem, public Plot2D
{
    Q_OBJECT
public:
    Q_PROPERTY(bool horizontal          READ isHorizontal        WRITE setHorizontal)
    Q_PROPERTY(float timelinePosition   READ timelinePosition    WRITE setTimelinePosition NOTIFY timelinePositionChanged)
    Q_PROPERTY(bool isEnabled           READ getPlotEnabled      WRITE setPlotEnabled)
    Q_PROPERTY(QString contactInfo      READ getContactInfo      WRITE setContactInfo      NOTIFY contactChanged)
    Q_PROPERTY(bool    contactVisible   READ getContactVisible   WRITE setContactVisible   NOTIFY contactChanged)
    Q_PROPERTY(int     contactPositionX READ getContactPositionX                           NOTIFY contactChanged)
    Q_PROPERTY(int     contactPositionY READ getContactPositionY                           NOTIFY contactChanged)
    Q_PROPERTY(int     contactIndx      READ getContactIndx                                NOTIFY contactChanged)
    Q_PROPERTY(double  contactLat       READ getContactLat                                 NOTIFY contactChanged)
    Q_PROPERTY(double  contactDepth     READ getContactDepth                               NOTIFY contactChanged)
    Q_PROPERTY(bool batchCorrect   READ batchCorrect    WRITE setBatchCorrect    NOTIFY drawBatchCorrectChanged)
    Q_PROPERTY(bool depthCorrect   READ depthCorrect    WRITE setDepthCorrect    NOTIFY drawDepthCorrectChanged)

    Q_PROPERTY(int  minUpRng  READ  getMinUpRng   WRITE  setMinUpRng   NOTIFY minUpRngChanged)
    Q_PROPERTY(int  maxLoRng  READ  getMaxLoRng   WRITE  setMaxLoRng   NOTIFY maxLoRngChanged)
    Q_PROPERTY(QString fromLonStr   READ fromLonStr  WRITE setFromLonStr   NOTIFY fromToLonLatiChanged)
    Q_PROPERTY(QString fromLatiStr  READ fromLatiStr WRITE setFromLatiStr  NOTIFY fromToLonLatiChanged)
    Q_PROPERTY(QString toLonStr     READ toLonStr    WRITE setToLonStr     NOTIFY fromToLonLatiChanged)
    Q_PROPERTY(QString toLatiStr    READ toLatiStr   WRITE setToLatiStr    NOTIFY fromToLonLatiChanged)

    qPlot2D(QQuickItem* parent = nullptr);
    void paint(QPainter *painter) override;
//    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) override;

    void setPlot(Dataset* plot);
    void setDataProcessor(DataProcessor* dataProcessorPtr);
    void clearPlotData();
    void setTimelinePositionToStart();

    bool isHorizontal() { return _isHorizontal; }
    void setHorizontal(bool is_horizontal) { _isHorizontal = is_horizontal; Plot2D::setHorizontal(_isHorizontal); update(); }

    void plotUpdate() override;

    bool eventFilter(QObject *watched, QEvent *event) override final;
    void sendSyncEvent(int epoch_index, QEvent::Type eventType) override final;

    int  getMinUpRng();
    void setMinUpRng(int minUpRng);
    int  getMaxLoRng();
    void setMaxLoRng(int maxLoRng);
    Q_INVOKABLE void setSoundVelocity(int soundVelocity, int draftOffset);
    Q_INVOKABLE void setDepthFilterVisible(bool visible, int value);
    Q_INVOKABLE void setKeelOffsetValue(int value);
    bool batchCorrect();
    bool depthCorrect();
    void setBatchCorrect(bool batchCorrect);
    void setDepthCorrect(bool depthCorrect);

    QString fromLonStr();
    void setFromLonStr(QString fromLon);
    QString fromLatiStr();
    void setFromLatiStr(QString fromLati);
    QString toLonStr();
    void setToLonStr(QString toLon);
    QString toLatiStr();
    void setToLatiStr(QString toLati);

    Q_INVOKABLE void drawDepthCorrect(int x, int y);
    Q_INVOKABLE float cursorFrom() const { return Plot2D::cursor_.distance.from; }
    Q_INVOKABLE float cursorTo() const { return Plot2D::cursor_.distance.to; }
    Q_INVOKABLE void setCursorFromTo(float from, float to);
    Q_INVOKABLE void setIndx(int indx);
    Q_INVOKABLE void resetUpLoRng(int upper, int lower);
    Q_INVOKABLE void setSensitivity(int sensitive);

    Q_INVOKABLE void setBottomLineVisible(bool isVisible);
    Q_INVOKABLE void drawBatchCorrect(int x, int y);
    Q_INVOKABLE void clearBatchCorrect();
    Q_INVOKABLE void updateBatchCorrect();
    Q_INVOKABLE void setMarkDistTimeVisible(bool visible, int dist0time1, int distInterval, int timeInterval,
                                    bool isFrame, bool isTime, bool isDepth, bool isCoordinate); //dist:0, time:1

    Q_INVOKABLE void setColorScheme(int index);


protected:
    Dataset* dataset_ = nullptr;
    QTimer* m_updateTimer;
    bool _isHorizontal = true;


signals:
    void timelinePositionChanged();
    void plotEnableChanged();
    void contactChanged();
    void outlineModeChanged();
    void minUpRngChanged();
    void maxLoRngChanged();
    void fromToLonLatiChanged();
    void drawBatchCorrectChanged();
    void drawDepthCorrectChanged();


protected slots:
    void dataUpdate();


public slots:
    void horScrollEvent(int delta);
    // void verZoomEvent(int delta);
    void verScrollEvent(int delta);
    Q_INVOKABLE void scaleYZoomEvent(int delta);
    Q_INVOKABLE void plotMousePosition(int x, int y, bool isSync = false);
    Q_INVOKABLE void simplePlotMousePosition(int x, int y);
    Q_INVOKABLE void onCursorMoved(int x, int y);
    Q_INVOKABLE void plotMouseTool(int mode);
    // Q_INVOKABLE bool setContact(int indx, const QString& text);
    // Q_INVOKABLE bool setActiveContact(int indx);
    // Q_INVOKABLE bool deleteContact(int indx);
    // Q_INVOKABLE void updateContact();


    void plotDatasetChannelFromStrings(const QString& ch1Str, const QString& ch2Str)
    {
        if (!datasetPtr_) {
           return;
        }

        auto [ch1, sub1, name1] = datasetPtr_->channelIdFromName(ch1Str);
        auto [ch2, sub2, name2] = datasetPtr_->channelIdFromName(ch2Str);

        setDataChannel(true, ch1, sub1, name1, ch2, sub2, name2);

        plotUpdate();
    }

    ChannelId plotDatasetChannel()     { return cursor_.channel1; }
    uint8_t   plotDatasetSubChannel()  { return cursor_.subChannel1; }
    ChannelId plotDatasetChannel2()    { return cursor_.channel2; }
    uint8_t   plotDatasetSubChannel2() { return cursor_.subChannel2; }

    void plotEchogramVisible(bool visible) { setEchogramVisible(visible); }
    void plotBottomTrackVisible(bool visible) { setBottomTrackVisible(visible); }
    void plotBottomTrackTheme(int theme_id) { setBottomTrackTheme(theme_id); }

    void plotRangefinderVisible(bool visible) { setRangefinderVisible(visible); }
    void plotRangefinderTheme(int theme_id) { setRangefinderTheme(theme_id); }
    void plotAttitudeVisible(bool visible) { setAttitudeVisible(visible); }
    void plotTemperatureVisible(bool visible) { setTemperatureVisible(visible); }
    void plotDopplerBeamVisible(bool visible, int beam_filter) { setDopplerBeamVisible(visible, beam_filter); }
    void plotDopplerInstrumentVisible(bool visible) { setDopplerInstrumentVisible(visible); }

    void plotGNSSVisible(bool visible, int flags) { setGNSSVisible(visible, flags);}

    void plotAcousticAngleVisible(bool visible) { setAcousticAngleVisible(visible); }

    void plotGridInvert(bool state) { setGridInvert(state); };
    void plotAngleVisibility(bool state)   { setAngleVisibility(state); }
    void plotAngleRange(int angleRange) { setAngleRange(angleRange); }
    void plotVelocityRange(float velocity) { setVelocityRange(velocity); }

    void plotDistanceAutoRange(int auto_range_type) { setDistanceAutoRange(auto_range_type); }

    void plotEchogramSetLevels(float low, float hight) {
        setEchogramLowLevel(low);
        setEchogramHightLevel(hight);
    }

    Q_INVOKABLE float getLowEchogramLevel() const;
    Q_INVOKABLE float getHighEchogramLevel() const;

    void drawPolygonOutline(bool outlineMode);
    void doDistProcessing(int preset, int window_size, float vertical_gap, float range_min, float range_max,
                    float gain_slope, float threshold, float offsetx, float offsety, float offsetz, bool manual);
    void refreshDistParams(int preset, int windowSize, float verticalGap, float rangeMin, float rangeMax,
                    float gainSlope, float threshold, float offsetX, float offsetY, float offsetZ);

    void setPreset(int value);
    void setWindowSize(int value);
    void setVerticalGap(float value);
    void setRangeMin(float value);
    void setRangeMax(float value);
    void setGainSlope(float value);
    void setThreshold(float value);
    void setOffsetX(float value);
    void setOffsetY(float value);
    void setOffsetZ(float value);

    Q_INVOKABLE void setDeleteFrameMode(bool mode);
    Q_INVOKABLE bool onDoubleClick(int x, int y);
    Q_INVOKABLE void updateDeleteFrameMousePos(int x, int y);
    Q_INVOKABLE void clearDeleteFrame();
    Q_INVOKABLE int  getDeleteStartIdx();
    Q_INVOKABLE int  getDeleteEndIdx();
    Q_INVOKABLE bool deleteSelectedFrames();

private:
    int indx_ = -1;
    int currentUpRng_ = 0, currentLoRng_ = 1500;
    bool prompt_ = true;
    bool isBatchCorrect_ = false;
    bool isDepthCorrect_ = false;
};
