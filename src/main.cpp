#include <QGuiApplication>
#include <QApplication>
#include <QQmlContext>
#include <QQmlApplicationEngine>
#include <QLocale>
#include <QResource>
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

void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    Q_UNUSED(type);
    Q_UNUSED(context);
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
    qmlRegisterType<GraphicsScene3dView>("SceneGraphRendering", 1, 0, "GraphicsScene3dView");
    qmlRegisterType<qPlot2D>("WaterFall", 1, 0, "WaterFall");
    qmlRegisterType<BottomTrack>("BottomTrack", 1, 0, "BottomTrack");
    qRegisterMetaType<BottomTrack::ActionEvent>("BottomTrack::ActionEvent");
    // qRegisterMetaType<LinkAttribute>("LinkAttribute");
}


int main(int argc, char *argv[])
{
#ifdef Q_OS_ANDROID
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "0");  // TODO: use qt scaling!
    qputenv("QT_SCALE_FACTOR", "0.5");
#endif

#if defined(Q_OS_LINUX)
    QCoreApplication::setAttribute(Qt::AA_ForceRasterWidgets, false);
    ::qputenv("QT_SUPPORT_GL_CHILD_WIDGETS", "1");
#ifdef LINUX_ES
    ::qputenv("QT_OPENGL", "es2");
#endif
#endif

    QCoreApplication::setOrganizationName("Toslon");
    QCoreApplication::setOrganizationDomain("Toslon.tech");
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
    QCoreApplication::addLibraryPath(QStringLiteral("assets:/qt/plugins"));
    QCoreApplication::addLibraryPath(QStringLiteral(":/android_rcc_bundle/plugins"));

    corePtr = new Core();   // 在QApplication创建后初始化，避免QEventLoop错误
    corePtr->initStreamList();

    // setApplicationDisplayName(app);
    QQmlApplicationEngine engine;
    engine.addImportPath("qrc:/");
    theme.setQmlEngine(&engine);

    SceneObject::qmlDeclare();

    // qInstallMessageHandler(messageHandler); // TODO: comment this
    theme.setTheme();

    registerQmlMetaTypes();

    engine.rootContext()->setContextProperty("dataset", corePtr->getDatasetPtr());
    engine.rootContext()->setContextProperty("core", corePtr);
    engine.rootContext()->setContextProperty("theme", &theme);
    // engine.rootContext()->setContextProperty("linkManagerWrapper", corePtr->getLinkManagerWrapperPtr());
    engine.rootContext()->setContextProperty("deviceManagerWrapper", corePtr->getDeviceManagerWrapperPtr());
    engine.rootContext()->setContextProperty("logViewer", corePtr->getConsolePtr());
    engine.rootContext()->setContextProperty("GetInterface", GetInterface::getInterface());

    corePtr->consoleInfo("Run...");
    corePtr->setEngine(&engine);

    theme.bootConfig();

    //qDebug() << "SQL drivers =" << QSqlDatabase::drivers(); // тут должен появиться QSQLITE
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,[url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            QCoreApplication::exit(-1);
            return;
        }

        QWindow* window = qobject_cast<QWindow*>(obj);
        if(window){
            window->showMaximized();
        }

        theme.refreshLanguage();
        SoftwareParametersStru softPar = theme.getSoftwareParameters();
        LLA lla(softPar.currentLati, softPar.currentLon, 0.0f);
        corePtr->refreshMap(lla);
        corePtr->switchMapType(softPar.mapSourceType);
    }, Qt::QueuedConnection);


// file opening on startup
#ifndef Q_OS_ANDROID
    if (argc > 1) {
        QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, corePtr, [&argv]() {
            corePtr->openLogFile(argv[1], false, true);
        }, Qt::QueuedConnection);
    }
#endif

    QObject::connect(&app,  &QGuiApplication::aboutToQuit, corePtr, [&]() {
        // corePtr->saveLLARefToSettings();
        // corePtr->removeLinkManagerConnections();
        // corePtr->stopLinkManagerTimer();
        corePtr->saveCurrentMapState([](double lat, double lon) {
            theme.setCurrentMapLocation(lat, lon);
        });
        theme.saveSoftwareParameters();
    });

    engine.load(url);
    qCritical() << "App is created";

    return app.exec();
}
