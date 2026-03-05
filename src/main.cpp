#include <QGuiApplication>
#include <QApplication>


#include <QQmlContext>
#include <QQmlApplicationEngine>
#include <QTranslator>
#include <QLocale>
#include <QSettings>
#include <QVector>
#include <QString>
#include <QThread>
#include <QResource>
#include <QFile>
#include <QByteArray>
#include <QQuickWindow>
#include <QSql>
#include <QSqlDatabase>
#include <QQuickStyle>
#include "qPlot2D.h"
#include "core.h"
#include "themes.h"
#include "scene_object.h"
#include "bottom_track.h"


Core* corePtr = nullptr;  // 改为指针，延迟初始化
Themes theme;
QTranslator translator;
QVector<QString> availableLanguages{"en", "ru", "pl"};

void loadLanguage(QGuiApplication &app)
{
    QSettings settings;
    QString currentLanguage;

    int savedLanguageIndex = settings.value("appLanguage", -1).toInt();

    if (savedLanguageIndex == -1) {
        currentLanguage = QLocale::system().name().split('_').first();
        if (auto indx = availableLanguages.indexOf(currentLanguage); indx == -1) {
            currentLanguage = availableLanguages.front();
        }
        else {
            settings.setValue("appLanguage", indx);
        }
    }
    else {
        if (savedLanguageIndex >= 0 && savedLanguageIndex < availableLanguages.count()) {
            currentLanguage = availableLanguages.at(savedLanguageIndex);
        }
        else {
            currentLanguage = availableLanguages.front();
        }
    }

    QString translationFile = ":/translations/translation_" + currentLanguage + ".qm";

    if (translator.load(translationFile)) {
        app.installTranslator(&translator);
    }
}

void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    Q_UNUSED(type);
    Q_UNUSED(context);
    // core.consoleInfo(msg);
}

void setApplicationDisplayName(QGuiApplication& app)
{
    QResource resource(":/version.txt");
    if (resource.isValid()) {
        QFile file(":/version.txt");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray data = file.readAll();
            app.setApplicationDisplayName(QString::fromUtf8(data));
            file.close();
        }
    }
}

void registerQmlMetaTypes()
{
    qmlRegisterType<GraphicsScene3dView>("SceneGraphRendering", 1, 0,"GraphicsScene3dView");
    qmlRegisterType<qPlot2D>( "WaterFall", 1, 0, "WaterFall");
    qmlRegisterType<BottomTrack>("BottomTrack", 1, 0, "BottomTrack");
    qRegisterMetaType<BottomTrack::ActionEvent>("BottomTrack::ActionEvent");
    qRegisterMetaType<LinkAttribute>("LinkAttribute");
}


int main(int argc, char *argv[])
{
#ifdef Q_OS_ANDROID
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "0");  // TODO: use qt scaling!
    qputenv("QT_SCALE_FACTOR", "0.5");            //
#endif

#if defined(Q_OS_LINUX)
    QCoreApplication::setAttribute(Qt::AA_ForceRasterWidgets, false);
    ::qputenv("QT_SUPPORT_GL_CHILD_WIDGETS", "1");
#ifdef LINUX_ES
    ::qputenv("QT_OPENGL", "es2");
#endif
#endif

    QCoreApplication::setOrganizationName("Toslon");
    QCoreApplication::setOrganizationDomain("toslon.tech");
    QCoreApplication::setApplicationName("ToslonApp");
    QCoreApplication::setApplicationVersion("0.10");

#if defined(Q_OS_WIN)
    //QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);
#endif

    QSurfaceFormat format;
#if defined(Q_OS_ANDROID) || defined(LINUX_ES)
    format.setRenderableType(QSurfaceFormat::OpenGLES);
#else
    format.setRenderableType(QSurfaceFormat::OpenGL);
#endif
    format.setSwapInterval(0);

    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);
    QQuickStyle::setStyle("Basic");
    corePtr = new Core();   // 在QApplication创建后初始化，避免QEventLoop错误

    QCoreApplication::addLibraryPath(QStringLiteral("assets:/qt/plugins"));
    QCoreApplication::addLibraryPath(QStringLiteral(":/android_rcc_bundle/plugins"));

    loadLanguage(app);
    corePtr->initStreamList();

    setApplicationDisplayName(app);
    QQmlApplicationEngine engine;
    engine.addImportPath("qrc:/");

    SceneObject::qmlDeclare();

    //qInstallMessageHandler(messageHandler); // TODO: comment this

    registerQmlMetaTypes();

    engine.rootContext()->setContextProperty("dataset", corePtr->getDatasetPtr());
    engine.rootContext()->setContextProperty("core", corePtr);
    engine.rootContext()->setContextProperty("theme", &theme);
    engine.rootContext()->setContextProperty("linkManagerWrapper", corePtr->getLinkManagerWrapperPtr());
    engine.rootContext()->setContextProperty("deviceManagerWrapper", corePtr->getDeviceManagerWrapperPtr());
    engine.rootContext()->setContextProperty("logViewer", corePtr->getConsolePtr());

    corePtr->consoleInfo("Run...");
    corePtr->setEngine(&engine);


    //qDebug() << "SQL drivers =" << QSqlDatabase::drivers(); // тут должен появиться QSQLITE
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,[url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)  QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);


// file opening on startup
#ifndef Q_OS_ANDROID
    if (argc > 1) {
        QObject::connect(&engine,   &QQmlApplicationEngine::objectCreated, corePtr,     [&argv]() {
            corePtr->openLogFile(argv[1], false, true);
        }, Qt::QueuedConnection);
    }
#endif

    QObject::connect(&app,  &QGuiApplication::aboutToQuit, corePtr, [&]() {
            // corePtr->saveLLARefToSettings();
            corePtr->removeLinkManagerConnections();
            corePtr->stopLinkManagerTimer();
#ifdef SEPARATE_READING
            corePtr->removeDeviceManagerConnections();
            corePtr->stopDeviceManagerThread();
#endif
    });

    engine.load(url);
    qCritical() << "App is created";



    // QScreen *screen = QGuiApplication::primaryScreen();
    // QSize size = screen->size();                // 分辨率（像素）
    // QRect geometry = screen->geometry();        // 屏幕区域
    // qreal dpi = screen->logicalDotsPerInch();   // 逻辑 DPI
    // qreal dpiPhysical = screen->physicalDotsPerInch(); // 物理 DPI
    // qreal ratio = screen->devicePixelRatio();   // 像素比
    // qDebug() << "屏幕分辨率(px):" << size.width() << "x" << size.height();
    // qDebug() << "屏幕geometry:" << geometry;
    // qDebug() << "逻辑DPI:" << dpi;
    // qDebug() << "物理DPI:" << dpiPhysical;
    // qDebug() << "devicePixelRatio:" << ratio;
    return app.exec();
}
