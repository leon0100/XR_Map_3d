#pragma once

#include "plot2D_plot_layer.h"
#include "plot2D_defs.h"
#include "colorScheme.h"



typedef struct
{
    int draft;    //吃水深度
    int sfEnd;    //surfaceEnd
    int btStart;  //bottomStart
    int startIdx;
    int endIdx;
    QList<quint8> rawData; //像素点
}StructSonarInfo;

#define COLOR_LINE 7   //偏移量
#define WAVE_PIXEL_WIDTH 36

class Plot2DEchogram : public PlotLayer {
public:
    enum ThemeId {
        ClassicTheme,
        SepiaTheme,
        WRGBDTheme,
        WBTheme,
        BWTheme,
        CustomTheme
    };

    Plot2DEchogram();
    ~Plot2DEchogram();
    bool draw(Plot2D* parent, Dataset* dataset);

    float getLowLevel() const;
    float getHighLevel() const;
    void setLowLevel(float low);
    void setHightLevel(float high);
    void setLevels(float low, float hight);

    void setColorScheme(QVector<QColor> coloros, QVector<int> levels);
    int getThemeId() const;
    void setThemeId(int theme_id);
    void setCompensation(int compensation_id);


    // int updateCash(Plot2D* parent, Dataset* dataset, int width, int height);
    int updateCache(Plot2D* parent, Dataset* dataset, int width, int height);
    void resetCash();
    void setUpperRng(int maxUpRng);
    void setLowerRng(int minLoRng);

    void addReRenderPlotIndxs(const QSet<int>& indxs);

protected:
    struct CashLine
    {
        enum class CashState
        {
            CashStateNotValid = 0,
            CashStateValid,
            CashStateEraced
        };

        int poolIndex = -1;
        CashState state = CashState::CashStateNotValid;
        bool isNeedUpdate = true;

        QVector<uint8_t> waveData;
        int sfEnd = 0;
        int btStart = 0;
        int bottomLineIdx = 0;
    };

    uint16_t _colorHashMap[256];
    QVector<CashLine> _cash;

    QVector<QRgb> _colorTable;
    QVector<QRgb> _colorLevels;
    QImage _image;
    QPixmap _pixmap;
    bool _flagColorChanged = true;

    int _compensation_id = 0;

    struct {
        bool resetCash = true;
    } _cashFlags;

    struct {
       float low = 100, high = 10;
    } _levels;

    struct {
       float low = NAN, high = NAN;
    } _lastLevels;

    DatasetCursor _lastCursor;

    bool getTriggerCashReset() {
        bool reset_cash = _cashFlags.resetCash;
        _cashFlags.resetCash = false;
        return reset_cash;
    }


private:
    void stretchCompressPixel(QVector<uint8_t> &rawDataVec, uint8_t* dist, int distLen, float scale, int startIndx);
    void drawLatestWavePixel(Plot2D* parent, int panelX, int panelY, int height);



private:
    ThemeId themeId_;
    QSet<int> reRenderPlotIndxs_;

    ZyColorScheme *zyColorScheme_;

    int currentUpRng_ = 0, currentLoRng_ = 3200;

    QVector<uint8_t> latestWave_;
    int latestWaveSfEnd_ = 0;
    int latestWaveBtStart_ = 0;
    int bottomLineIdx_ = 0;

};
