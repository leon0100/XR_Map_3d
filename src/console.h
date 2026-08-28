#ifndef CONSOLE_H
#define CONSOLE_H



#include <QObject>
// #include <console_list_model.h>
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



/*-----------------------------------GetInterface单例---------------------------------------------*/
#define GIF  GetInterface::getInterface()
class GetInterface : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
public:
    GetInterface(const GetInterface&) = delete;
    GetInterface& operator=(const GetInterface&) = delete;

    static GetInterface* getInterface() {
        static GetInterface instance;
        return &instance;
    }



private:
    explicit GetInterface(QObject* parent = nullptr) : QObject(parent)
    {
    }



public:
    Q_INVOKABLE void dialogInfo(int type, const QString &msg)
    {
        emit showDialogInfo(type, msg);
    }

    Q_INVOKABLE void dialogYesNoBtn(bool flag)
    {
        if(yesNoCallback_) {
            yesNoCallback_(flag);
            yesNoCallback_ = nullptr;
        }
    }

    Q_INVOKABLE void checkDialogBtn(bool flag, bool isCheck)
    {
        if(checkDialogCallback_) {
            checkDialogCallback_(flag, isCheck);
            checkDialogCallback_ = nullptr;
        }
    }

    std::function<void(bool)> yesNoCallback_;
    void dialogYesNo(const QString &msg, std::function<void(bool)> cb)
    {
        yesNoCallback_ = cb;
        emit showDialogInfo(Dialog_YesNo, msg);
    }

    /*
        // 主循环(永远运行)
        QApplication::exec()  ◄─────────────┐
            └─ 处理事件队列                  │
                                             │
        // 嵌套子循环(临时运行)              │
        loop.exec()  ◄──┐                   │
            └─ 处理事件队列(子级)            │
                └─ loop.quit() → 返回 ───────┘  退出后回到主循环
    */
    bool dialogYesNoSync(const QString &msg)
    {
        bool result = false;

        QEventLoop loop; // 创建嵌套事件循环对象
        yesNoCallback_ = [&result, &loop](bool confirmed) { //注册回调
            result = confirmed;
            loop.quit();
        };
        emit showDialogInfo(Dialog_YesNo, msg);
        loop.exec();//当前线程在此阻塞,但Qt事件分发机制仍在工作,可以处理: QML按钮点击事件、其他排队的信号

        yesNoCallback_ = nullptr;
        return result;
    }


    std::function<void(bool, bool)> checkDialogCallback_;
    void dialogCheck(const QString &msg, std::function<void(bool, bool)> cb,
                     const QString &checkBoxText = QString())
    {
        checkDialogCallback_ = cb;
        emit showDialogInfo(Dialog_Check, msg);
        emit setCheckBoxText(checkBoxText);
    }
    void dialogCheck2(const QString &msg, std::function<void(bool, bool)> cb,
                      const QString &checkBoxText = QString())
    {
        checkDialogCallback_ = cb;
        emit showDialogInfo(Dialog_Check2, msg);
        emit setCheckBoxText(checkBoxText);
    }



signals:
    void showDialogInfo(int type, const QString &msg);
    void flashDialog(int type);
    void setCheckBoxText(const QString &text);

};


#endif // CONSOLE_H
