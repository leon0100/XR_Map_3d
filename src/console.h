#ifndef CONSOLE_H
#define CONSOLE_H

#include <QObject>
#include <console_list_model.h>
#include <QLoggingCategory>
#include <QQuickTextDocument>

enum DialogType {
    Dialog_OK   = 0,
    Dialog_YesNo,
    Dialog_Check
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
        if(type == 0) {
            dialogOkShow_ = true;
        } else if(type == 1) {
            dialogYesNoShow_ = true;
        } else if(type == 2) {
            dialogCheck_ = false;
            dialogCheckShow_ = true;
        }
        emit showDialogInfo(type, msg);
    }

    Q_INVOKABLE void finish() {
        dialogOkShow_ = false;
        dialogYesNoShow_ = false;
        dialogCheckShow_ = false;

        if(checkDialogCallback_) {
            checkDialogCallback_(dialogCheck_);//这里调用回调
            checkDialogCallback_ = nullptr;
        }

        emit finished();
    }

    Q_INVOKABLE void dialogYesNoBtn(bool flag) {
        finish();
        dialogYesNoBtn_ = flag;
    }

    Q_INVOKABLE void setDialogCheck(bool checked) {
        dialogCheck_ = checked;
    }

    bool checkDialogShow() {
        if(dialogOkShow_ ) {
            emit flashDialog(0);
            return true;
        } else if(dialogYesNoShow_) {
            emit flashDialog(1);
            return true;
        } else if(dialogCheckShow_) {
            emit flashDialog(2);
            return true;
        }
        return false;
    }

    std::function<void(bool)> checkDialogCallback_;
    Q_INVOKABLE void dialogCheck(const QString &msg, std::function<void(bool)> cb) {
        checkDialogCallback_ = cb;
        dialogCheckShow_ = true;
        dialogCheck_ = false;
        emit showDialogInfo(Dialog_Check, msg);
    }


public:
    bool getDialogYesNoBtn() {
        return dialogYesNoBtn_;
    }


signals:
    void showDialogInfo(int type, const QString &msg);
    void flashDialog(int type);
    void finished();


private:
    bool dialogOkShow_ = false, dialogYesNoShow_ = false, dialogCheckShow_ = false;
    bool dialogYesNoBtn_ = false, dialogCheck_ = false;


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
