#ifndef COLORSCHEME_H
#define COLORSCHEME_H


#include <QObject>
#include <QtGui>


typedef struct
{
    float colorPosition;
    uint colorValue;
}StructColorList;

class ZyColorScheme
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

    void initDefaultColorList();
    void applyColorScheme(int index);

    static const int COLOR_SCHEME_COUNT = 5;

    static int colorSchemeIndex();


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

    static int colorSchemeIndex_;

private:
    QList<StructColorList> colorList_surface;
    QList<StructColorList> colorList_fish;
    QList<StructColorList> colorList_bottom;

    QList<StructColorList> colorList_surface_2;
    QList<StructColorList> colorList_fish_2;
    QList<StructColorList> colorList_bottom_2;

};

#endif // COLORSCHEME_H
