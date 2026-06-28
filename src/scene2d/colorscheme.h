#ifndef COLORSCHEME_H
#define COLORSCHEME_H


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
/*----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
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

//     static void loadColorScheme(QString fileName);
//     static void saveColorScheme();
//     static void readColorToColorList(QString fileName);
//     static void getColorFromColorList();

//     static void loadColorScheme_2(QString fileName);
//     static void saveColorScheme_2();
//     static void readColorToColorList_2(QString fileName);
//     static void getColorFromColorList_2();

//     static void loadColorScheme_channel1(QString fileName);
//     static void saveColorScheme_channel1();
//     static void readColorToColorList_channel1(QString fileName);
//     static void getColorFromColorList_channel1();

//     static void loadColorScheme_channel2(QString fileName);
//     static void saveColorScheme_channel2();
//     static void readColorToColorList_channel2(QString fileName);
//     static void getColorFromColorList_channel2();


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
//     static QList<StructColorList> colorList_surface;
//     static QList<StructColorList> colorList_fish;
//     static QList<StructColorList> colorList_bottom;

//     static QList<StructColorList> colorList_surface_2;
//     static QList<StructColorList> colorList_fish_2;
//     static QList<StructColorList> colorList_bottom_2;

//     static QList<StructColorList> colorList_surface_channel1;
//     static QList<StructColorList> colorList_fish_channel1;
//     static QList<StructColorList> colorList_bottom_channel1;

//     static QList<StructColorList> colorList_surface_channel2;
//     static QList<StructColorList> colorList_fish_channel2;
//     static QList<StructColorList> colorList_bottom_channel2;

// };



typedef struct
{
    float colorPosition;
    uint colorValue;
}StructColorList;

class ZyColorScheme : public QWidget
{
public:
    ZyColorScheme();
    ~ZyColorScheme();

    void loadColorScheme(QString fileName);
    void saveColorScheme();
    void readColorToColorList(QString fileName);
    void getColorFromColorList();

    void loadColorScheme_2(QString fileName);
    void saveColorScheme_2();
    void readColorToColorList_2(QString fileName);
    void getColorFromColorList_2();


    /*---------------------------------*色表*---------------------------------*/
    static int colorScheme_surface[];
    static int colorScheme_fish[];
    static int colorScheme_bottom[];

    static int colorScheme_surface_2[];
    static int colorScheme_fish_2[];
    static int colorScheme_bottom_2[];

    static int colorLine;

    static int background[];
    static int backgroundIndex;
    static int getBackgroundIndex();
    static void setBackgroundIndex(int value);

    static int onlineColor;
    static int getOnlineColor();
    static void setOnlineColor(int value);

private:
    QList<StructColorList> colorList_surface;
    QList<StructColorList> colorList_fish;
    QList<StructColorList> colorList_bottom;

    QList<StructColorList> colorList_surface_2;
    QList<StructColorList> colorList_fish_2;
    QList<StructColorList> colorList_bottom_2;

};

#endif // COLORSCHEME_H
