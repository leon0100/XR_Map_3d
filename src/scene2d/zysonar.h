#ifndef ZYSONAR_H
#define ZYSONAR_H

#include <QObject>

#include <QWidget>
#include <QtGui>
#include <QLine>
#include <QDesktopWidget>

#include <QPainter>
#include <QMouseEvent>
#include <QCursor>
#include <QPushButton>
#include <QStringList>

#include <QMessageBox>


#define     GET_YEAR(x)         (x&0b11111100000000000000000000000000)>>26
#define     GET_MONTH(x)        (x&0b00000011110000000000000000000000)>>22
#define     GET_DAY(x)          (x&0b00000000001111100000000000000000)>>17
#define     GET_HOUR(x)         (x&0b00000000000000011111000000000000)>>12
#define     GET_MINUTE(x)       (x&0b00000000000000000000111111000000)>>6
#define     GET_SECOND(x)       (x&0b00000000000000000000000000111111)

typedef struct
{
    int draft;   //吃水深度
    int sfEnd;   //surfaceEnd
    int btStart;  //bottomStart
    int startIdx;
    int endIdx;
    QList<quint8> rawData; //像素点
}StructSonarInfo;

typedef struct
{
    int btStart;
    float scale;
}StructBottomLine;

#define SONAR_BMP_WIDTH 512      /*! @brief 声呐图片宽度，也就是原始数据数组列数 */  //该值没用到
#define SONAR_IMAGE_HEIGHT 2048  /*! @brief 声呐图片高度 */
/*-----------------------------------------------------------------------------*/
/*! @brief 声呐图像类*/
// class ZySonarImage : public QWidget
// {
//     Q_OBJECT
// public:
//     ZySonarImage(QWidget *parent = 0, int imageHeight = 1024, QString mode="offline");
//     ~ZySonarImage();

//     void initImageBuffer();
//     void reInitImageBuffer(int width);
//     void reFillBackgroundColor();

//     int getSonarImageWidth() const;
//     int getSonarImageHeight() const;

//     void drawImagePixel(int column, StructSonarInfo sonarInfo, float scale, int colorNum, bool is2nd=false,int flagViewMode=0);
//     void stretchImagePixel(int column, StructSonarInfo sonarInfo, float scale,int colorNum, bool is2nd=false,int flagViewMode=0);
//     void compressImagePixel(int column, StructSonarInfo sonarInfo, float scale,int colorNum, bool is2nd=false,int flagViewMode=0);

//     void drawImagePixelOnline(int column, StructSonarInfo sonarInfo, float scale);
//     void stretchImagePixelOnline(int column, StructSonarInfo sonarInfo, float scale);
//     void compressImagePixelOnline(int column, StructSonarInfo sonarInfo, float scale);
//     void setUseBottomLine(bool value);

//     void refreshTextInfo_AboveImage(float depth, float hdg, float spd, double lon, double lat, float temp);
//     void setSonarFreqString(QString freq);

//     void showMarkLine(int x_pos, float depth, double lon, double lat, QString dateTime);
//     void hideMarkLine();

//     void setRemoveIndex(int indexFrom, int indexTo);
//     void doRemoveModeStep_1(int numFrom);
//     void doRemoveModeStep_2(int numTo);
//     void getInRemoveMode();
//     void getOutRemoveMode();
//     int  getRemoveModeStep() const;

//     void setChartSpeedLevel(int value);

//     void markingClear();
//     void markingAppend(int pos, QString string);

//     void bottomLineClear();
//     void bottomLineAppend(int pos, int btStart);
//     void bottomLineAppend_2(int pos, int btStart);
//     void bottomLineAppend_channel1(int pos, int btStart);
//     void bottomLineAppend_channel2(int pos, int btStart);

//     void setMultiCorrect(bool value, bool small2s);

//     QString getMarkLineInfo();
//     void showMarkLine(int x_pos, float depth, double lon, double lat, quint32 time, float alt1, float alt2);
//     bool markLineIsShow();

//     void showAlarm();
//     void hideAlarm();

//     void setDualFreqMode(bool isDual);
//     void setCurrentCorrectColor(QString color);

//     void filterLineClear();
//     void filterLineAppend(int pos, int btStart);
//     void filterLineAppend_2(int pos, int btStart);
//     void filterLineAppend_channel1(int pos, int btStart);
//     void filterLineAppend_channel2(int pos, int btStart);

//     void showChannelSwitch();
//     void setCurrentSonarImage(int num);
//     int getCurrentSonarImage();

// signals:
//     void signal_MouseMove(int,int);
//     void signal_MouseDoubleClickedLeft(int);
//     void signal_MouseClickedRight();
//     void signal_SonarImageResized();
//     void signal_multiCorrect(QList<QPoint>);
//     void signal_SonarImageDrag(int);
//     void signal_SonarImageWheelUp();
//     void signal_SonarImageWheelDown();
//     void signal_NaviLineMove(int,bool);
//     void signal_chBox_click();

// protected:
//     void mouseMoveEvent(QMouseEvent *event);
//     void mouseDoubleClickEvent(QMouseEvent *event);
//     void resizeEvent(QResizeEvent *size);
//     void mouseReleaseEvent(QMouseEvent *event);
//     void mousePressEvent(QMouseEvent *event);
//     void leaveEvent(QMouseEvent *event);
//     void wheelEvent(QWheelEvent *event);

// private:
//     void paintEvent(QPaintEvent *event);

//     int sonarImageWidth;
//     int sonarImageHeight = 1024;
//     int *sonarImageBuffer,*sonarImageBuffer_2,*sonarImageBuffer_channel1,*sonarImageBuffer_channel2;
//     QImage *sonarImage,*sonarImage_2,*sonarImage_channel1,*sonarImage_channel2;
//     bool isDualFreqMode = false;

//     QString info_depth="", info_hdg="", info_spd="", info_lon="", info_lat="", info_temp="", info_sat="", info_hdop="", info_status="", info_wridx="";
//     QString currentSonarFreq = "";
//     QString currentCorrectColor = "red";

//     int lastMousePos=0,nowMousePos=0;

//     QWindow *window;

//     int markLineIndex=-1;
//     QString markInfo_depth="-.-", markInfo_lon="-.-", markInfo_lat="-.-", markInfo_date="-.-", markInfo_time="-.-", markInfo_alt1="-.-", markInfo_alt2="-.-";

//     QList<int> list_markingPos;
//     QList<QString> list_markingString;

//     QList<int> list_bottomLineIdx;
//     QList<int> list_bottomLinePos;
//     QList<int> list_filterLineIdx;
//     QList<int> list_filterLinePos;

//     QList<int> list_bottomLineIdx_2;
//     QList<int> list_bottomLinePos_2;
//     QList<int> list_filterLineIdx_2;
//     QList<int> list_filterLinePos_2;

//     QList<int> list_bottomLineIdx_channel1;
//     QList<int> list_bottomLinePos_channel1;
//     QList<int> list_filterLineIdx_channel1;
//     QList<int> list_filterLinePos_channel1;

//     QList<int> list_bottomLineIdx_channel2;
//     QList<int> list_bottomLinePos_channel2;
//     QList<int> list_filterLineIdx_channel2;
//     QList<int> list_filterLinePos_channel2;

//     QPoint toolTipPos = QPoint(-1,-1);

//     int lastBtStart = 0;
//     float lastScale = 1;
//     QString lastOp = "";
//     bool useBottomLine = true;

//     int removeFromIndex = -1;
//     int removeToIndex = -1;
//     int removeModeStep = -1;
//     QString removeFromNum = "";
//     QString removeToNum = "";

//     int chartSpeedLevel = 1;

//     bool flag_showExtraInfo = false;
//     QString currentViewerMode = "offline";

//     bool flag_inMultiCorrect = false;
//     bool flag_useSmall = false;
//     QList<QPoint> list_multiCorrect;
//     QPoint mousePos = QPoint(0,0);

//     bool flag_inAlarm = false;
//     bool flag_notAgain = false;
//     bool flag_chBox_check = false;
//     int x_chBox=0,y_chbox = 0;

//     bool flag_inNaviLine = false;
//     int nowX = 0;

//     int currChannelValue = -1;  //0：正扫     1、2：1、2通道的侧扫
//     QPixmap slideScanBoat_;
//     unsigned int boatWidth_;
//     unsigned int boatHeight_;
// };


// /*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
// /*! @brief 声呐波形类*/
// class ZySonarWave : public QWidget
// {
//     Q_OBJECT
// public:
//     ZySonarWave(QWidget *parent = 0, int waveHeight = 1024);
//     ~ZySonarWave();

//     void initWaveBuffer();
//     void reFillBackgroundColor();

//     void drawWavePixel(StructSonarInfo sonarInfo, float scale, int colorNum);
//     void stretchWavePixel(StructSonarInfo sonarInfo, float scale, int colorNum);
//     void compressWavePixel(StructSonarInfo sonarInfo, float scale, int colorNum);

//     void drawWavePixelOnline(StructSonarInfo sonarInfo, float scale);
//     void stretchWavePixelOnline(StructSonarInfo sonarInfo, float scale);
//     void compressWavePixelOnline(StructSonarInfo sonarInfo, float scale);

//     int getBottomLineIndex() const;
//     void setBottomLineIndex(int value);
//     void setBottomLineIndex_hFreq(int value);
//     void setBottomLineIndex_channel1(int value);
//     void setBottomLineIndex_channel2(int value);

// signals:
//     void signal_MouseMoveY(int);
//     void signal_MousePressed();
//     void signal_MouseReleased();
//     void signal_MouseLeaved();
//     void signal_MouseDoubleClickedRight();

// protected:
//     void mouseMoveEvent(QMouseEvent *event);
//     void mousePressEvent(QMouseEvent *event);
//     void mouseReleaseEvent(QMouseEvent *event);
//     void leaveEvent(QEvent *event);
//     void mouseDoubleClickEvent(QMouseEvent *event);
// private:
//     void paintEvent(QPaintEvent *event);

//     int sonarWaveHeight = 1024;
//     int *sonarWaveBuffer;   //<声呐实时波形缓存数组
//     QImage *sonarWave;      //<要显示出来的声呐实时波形图像

//     bool showBottomLine = true;

//     int bottomLineIndex = 0;
//     int bottomLineIndex_hFreq = -1;
//     int bottomLineIndex_channel1 = -1;
//     int bottomLineIndex_channel2 = -1;
//     int ratio = 10;

//     int currentColor =1 ;
// };



// /*! @brief 声呐波形类      侧扫声纳显示*/
// class ZySonarWave_SlideScan : public QWidget
// {
//     Q_OBJECT
// public:
//     ZySonarWave_SlideScan(QWidget *parent = 0, int waveWidth = 512);
//     ~ZySonarWave_SlideScan();

//     void initWaveBuffer();
//     void reFillBackgroundColor();

//     void drawWavePixel(StructSonarInfo sonarInfo, float scale, int colorNum);
//     void stretchWavePixel(StructSonarInfo sonarInfo, float scale, int colorNum);
//     void compressWavePixel(StructSonarInfo sonarInfo, float scale, int colorNum);

//     int getBottomLineIndex() const;
//     void setBottomLineIndex(int value);

// signals:
//     void signal_MouseMoveY(int);
//     void signal_MousePressed();
//     void signal_MouseReleased();
//     void signal_MouseLeaved();
//     void signal_MouseDoubleClickedRight();

// protected:
//     void mouseMoveEvent(QMouseEvent *event);
//     void mousePressEvent(QMouseEvent *event);
//     void mouseReleaseEvent(QMouseEvent *event);
//     void leaveEvent(QEvent *event);
//     void mouseDoubleClickEvent(QMouseEvent *event);
// private:
//     void paintEvent(QPaintEvent *event);

//     int sonarWaveWidth = 512;
//     int *sonarWaveBuffer;   //<声呐实时波形缓存数组
//     QImage *sonarWave;      //<要显示出来的声呐实时波形图像

//     bool showBottomLine = true;

//     int whichChannel = 0; //1、1通道；   2、2通道
//     int bottomLineIndex = 0;

//     int ratio = 10;
// };





// /*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
// /*! @brief 声呐刻度尺类*/
// class ZySonarAxis : public QWidget
// {
//     Q_OBJECT
// public:
//     ZySonarAxis(QWidget *parent = 0);
//     ~ZySonarAxis();

//     void setLoRng(float value);
//     void setUpRng(float value);

// signals:
//     void signal_MouseDoubleClickedRight();

// private:
//     void paintEvent(QPaintEvent *event);
//     void mouseDoubleClickEvent(QMouseEvent *event);

//     float loRng = 10;
//     float upRng = 0;
// };


// class ZySonarAxis_slideScan : public QWidget
// {
//     Q_OBJECT
// public:
//     ZySonarAxis_slideScan(QWidget *parent = 0);
//     ~ZySonarAxis_slideScan();

//     void setLoRng(float value);
//     void setUpRng(float value);

//     void setSlideScanChannelNum(int value);

// signals:
//     void signal_MouseDoubleClickedRight();

// private:
//     void paintEvent(QPaintEvent *event);
//     void mouseDoubleClickEvent(QMouseEvent *event);

//     float loRng = 10;
//     float upRng = 0;

//     int slideScanChanNum = 0;
// };



// /*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
// typedef struct
// {
//     float colorPosition;
//     uint colorValue;
// }StructColorList;

// class ZyColorScheme : public QWidget
// {
// public:
//     ZyColorScheme();
//     ~ZyColorScheme();

//     void loadColorScheme(QString fileName);
//     void saveColorScheme();
//     void readColorToColorList(QString fileName);
//     void getColorFromColorList();

//     void loadColorScheme_2(QString fileName);
//     void saveColorScheme_2();
//     void readColorToColorList_2(QString fileName);
//     void getColorFromColorList_2();

//     void loadColorScheme_channel1(QString fileName);
//     void saveColorScheme_channel1();
//     void readColorToColorList_channel1(QString fileName);
//     void getColorFromColorList_channel1();

//     void loadColorScheme_channel2(QString fileName);
//     void saveColorScheme_channel2();
//     void readColorToColorList_channel2(QString fileName);
//     void getColorFromColorList_channel2();


//     /*---------------------------------*色表*---------------------------------*/
//     static int colorScheme_surface[];
//     static int colorScheme_fish[];
//     static int colorScheme_bottom[];

//     static int colorScheme_surface_2[];
//     static int colorScheme_fish_2[];
//     static int colorScheme_bottom_2[];

//     static int colorScheme_surface_channel1[];
//     static int colorScheme_fish_channel1[];
//     static int colorScheme_bottom_channel1[];

//     static int colorScheme_surface_channel2[];
//     static int colorScheme_fish_channel2[];
//     static int colorScheme_bottom_channel2[];

//     static int colorLine;

//     static int background[];
//     static int backgroundIndex;
//     static int getBackgroundIndex();
//     static void setBackgroundIndex(int value);

//     static int onlineColor;
//     static int getOnlineColor();
//     static void setOnlineColor(int value);

// private:
//     QList<StructColorList> colorList_surface;
//     QList<StructColorList> colorList_fish;
//     QList<StructColorList> colorList_bottom;

//     QList<StructColorList> colorList_surface_2;
//     QList<StructColorList> colorList_fish_2;
//     QList<StructColorList> colorList_bottom_2;

//     QList<StructColorList> colorList_surface_channel1;
//     QList<StructColorList> colorList_fish_channel1;
//     QList<StructColorList> colorList_bottom_channel1;

//     QList<StructColorList> colorList_surface_channel2;
//     QList<StructColorList> colorList_fish_channel2;
//     QList<StructColorList> colorList_bottom_channel2;

// };


#endif // ZYSONAR_H
