#ifndef CONSOLE_H
#define CONSOLE_H

#include <QObject>
#include <console_list_model.h>
#include <QLoggingCategory>
#include <QQuickTextDocument>

enum DialogType {
    Dialog_OK   = 0,
    Dialog_YesNo,
    Dialog_Check,
    Dialog_Check2,
    Dialog_Loading
};

struct ImageInfo
{
    int x = 0;
    int y = 0;
    int z = 0;
    QString url;       // 下载瓦片的地址
    QString format;    // 图片格式
    // QPixmap img;       // 保存下载后的瓦片
    short count = 0;   // 失败下载次数，初始为0，下载失败一次+1
    bool isValidTile = false;
};



#define _180_PI (57.2957795131f)
#define _PI_180 (0.01745329252f)
#define EARTH_RADIUS 6378137   // 赤道半径
#define POINT_REPEATITIVE 361.0f


#ifndef PI
#define PI (3.1415926535898)
#endif


#define  MAP_TIlE_SIZE   (256)



/*---------------------------------------GetInterface单例------------------------------------------------*/
#define GIF  GetInterface::getInterface()
class GetInterface : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
public:
    GetInterface(const GetInterface&) = delete;
    GetInterface& operator=(const GetInterface&) = delete;

    static GetInterface* getInterface();

    void setCurrentMap(QString currTileUrl) {
        currTileUrl_ = currTileUrl;
    }
    QString getCurrentMap() {
        return currTileUrl_;
    }

    double getDistance_Haversine(double currLon, double currLati, double goalLon, double goalLati);


private:
    explicit GetInterface(QObject* parent = nullptr);


private:
    QString currTileUrl_;


signals:
    void update(ImageInfo info);             // 传出下载的瓦片图信息
    void updateTitle(int x, int y, int z);   // 传出下载的瓦片编号

    void showRect(QRect rect);   // 设置显示像素范围
    void setLevel(int level);    // 设置瓦片层级
    void setLevelDirection(int level);

public:
    Q_INVOKABLE void dialogInfo(int type, const QString &msg) {
        emit showDialogInfo(type, msg);
    }

    Q_INVOKABLE void dialogYesNoBtn(bool flag) {
        if(yesNoCallback_) {
            yesNoCallback_(flag);
            yesNoCallback_ = nullptr;
        }
    }

    Q_INVOKABLE void checkDialogBtn(bool flag, bool isCheck) {
        if(checkDialogCallback_) {
            checkDialogCallback_(flag, isCheck);
            checkDialogCallback_ = nullptr;
        }
    }

    std::function<void(bool)> yesNoCallback_;
    void dialogYesNo(const QString &msg, std::function<void(bool)> cb) {
        yesNoCallback_ = cb;
        emit showDialogInfo(Dialog_YesNo, msg);
    }

    std::function<void(bool, bool)> checkDialogCallback_;
    void dialogCheck(const QString &msg, std::function<void(bool, bool)> cb, const QString &checkBoxText = QString()) {
        checkDialogCallback_ = cb;
        emit showDialogInfo(Dialog_Check, msg);
        emit setCheckBoxText(checkBoxText);
    }
    void dialogCheck2(const QString &msg, std::function<void(bool, bool)> cb, const QString &checkBoxText = QString()) {
        checkDialogCallback_ = cb;
        emit showDialogInfo(Dialog_Check2, msg);
        emit setCheckBoxText(checkBoxText);
    }



signals:
    void showDialogInfo(int type, const QString &msg);
    void flashDialog(int type);
    void setCheckBoxText(const QString &text);

};




/*----------------------------------------Console----------------------------------------------*/
class Console : public QObject
{
    Q_OBJECT
public:
    Console();
    ConsoleListModel* listModel() const;

    void put(QtMsgType type, const QString &msg);

public slots:


private:
    ConsoleListModel *m_list;
};

#endif // CONSOLE_H
