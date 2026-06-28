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

#define COLOR_LINE 0   //偏移量

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

    void updateColors();

    // 设置配色类型（surface/fish/bottom）
    void setColorSchemeType(int type) { _colorSchemeType = type; }
    int getColorSchemeType() const { return _colorSchemeType; }

    // int updateCash(Plot2D* parent, Dataset* dataset, int width, int height);
    int updateCash(Plot2D* parent, Dataset* dataset, int width, int height, int sfEnd = -1, int btStart = -1);
    void resetCash();

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

        QVector<int16_t> data;
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
    int _lastWidth = -1;
    int _lastHeight = -1;

    bool getTriggerCashReset() {
        bool reset_cash = _cashFlags.resetCash;
        _cashFlags.resetCash = false;
        return reset_cash;
    }
private:
    ThemeId themeId_;
    QSet<int> reRenderPlotIndxs_;

    // 自定义配色相关
    ZyColorScheme _colorScheme;
    ZyColorScheme *zyColorScheme_;
    QString _customSchemePath;  // 自定义配色文件路径
    int _colorSchemeType;       // 配色类型：0=surface, 1=fish, 2=bottom
    bool _useCustomScheme;      // 是否使用自定义配色


    static int colorScheme_surface[];
    static int colorScheme_fish[];
    static int colorScheme_bottom[];

    QList<StructColorList> colorList_surface;
    QList<StructColorList> colorList_fish;
    QList<StructColorList> colorList_bottom;

    int sonarImageHeight = 1024;


    // 深度分区参数
    float _currentDepth;        // 当前深度
    float _currentLoRng;        // 低量程
    float _currentUpRng;        // 高量程
    float _currentSspd;         // 声速
    int _currentPingSize;       // 脉冲大小
    int _sfEnd;                 // 水表结束位置（像素）
    int _btStart;               // 水底开始位置（像素）
    int cntt_ = 0;
};
