#ifndef GRAPHICSSCENE3DVIEW_H
#define GRAPHICSSCENE3DVIEW_H

#include <QQuickFramebufferObject>
#include <QtMath>
#include "coordinate_axes.h"
#include "plane_grid.h"
#include "ray_caster.h"
#include "surface_view.h"
#include "image_view.h"
#include "map_view.h"
#include "contacts.h"
#include "boat_track.h"
#include "bottom_track.h"
#include "polygon_group.h"
#include "point_group.h"
#include "ray.h"
#include "navigation_arrow.h"
#include "usbl_view.h"
#include "isobaths_view.h"
#include "data_processor.h"
#include "screetShot.h"




class Dataset;
class GraphicsScene3dRenderer;
class GraphicsScene3dView : public QQuickFramebufferObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(GraphicsScene3dView)
    Q_PROPERTY(double currLat READ getCurrLat NOTIFY currentLatChanged)
    Q_PROPERTY(double currLon READ getCurrLon NOTIFY currentLonChanged)

public:
    //Camera
    class Camera
    {
    public:
        explicit Camera(GraphicsScene3dView* viewPtr = nullptr);
        Camera(qreal pitch, qreal yaw, qreal distToFocusPoint, qreal fov, qreal sensivity);

        float distForMapView() const;
        qreal distToFocusPoint() const;
        qreal fov() const;
        qreal pitch() const;
        qreal yaw() const;
        QMatrix4x4 viewMatrix() const;

        void setCameraListener(Camera* cameraListener) {
            cameraListener_ = cameraListener;
        };

        //void rotate(qreal yaw, qreal pitch); //TODO! Process this method later
        void rotate(const QVector2D& lastMouse, const QVector2D& mousePos);
        void rotate(const QPointF& prevCenter, const QPointF& currCenter, qreal angleDelta, qreal widgetHeight);
        void move(const QVector2D &lastMouse, const QVector2D &mousePos);
        void resetZAxis();
        void moveZAxis(float z);
        void zoom(qreal delta);
        void commitMovement();
        void focusOnObject(std::weak_ptr<SceneObject> object);
        void focusOnPosition(const QVector3D& pos);
        void setDistance(qreal distance);
        void setIsometricView();
        void setMapView();
        void reset();
        void resetRotationAngle();

        float getHeightAboveGround() const;
        float getAngleToGround() const;
        bool  getIsPerspective() const;
        bool  getIsFarAwayFromOriginLla() const;
        map::CameraTilt getCameraTilt() const;
        QVector3D getEyePosition() const;

    private:
        void updateCameraParams();
        void tryToChangeViewLlaRef();
        void updateViewMatrix();
        void checkRotateAngle();
        void tryResetRotateAngle();

    private:
        friend class GraphicsScene3dView;
        friend class GraphicsScene3dRenderer;

        Camera* cameraListener_ = nullptr;

        QVector3D m_eye = {0.0f, 0.0f, 0.0f};
        QVector3D m_up  = {0.0f, 1.0f, 0.0f};
        QVector3D m_lookAt = {0.0f, 0.0f, 0.0f};
        QVector3D m_lookAtSave = {0.0f, 0.0f, 0.0f};
        QVector3D m_relativeOrbitPos = {0.0f, 0.0f, 0.0f};

        QMatrix4x4 m_view;

        std::weak_ptr<SceneObject> m_focusedObject;
        QVector3D m_offset;
        QVector3D m_deltaOffset;
        QVector3D m_focusPoint;

        qreal m_pitch = 0.f;
        qreal m_yaw = 0.f;
        qreal m_fov = 45.f;
        float m_distToFocusPoint = 50.f;
        float distForMapView_ = m_distToFocusPoint;
        qreal m_sensivity = 4.f;
        float distToGround_ = 0.0f;
        float angleToGround_ = 0.0f;
        bool  isPerspective_ = false;
        float highDistThreshold_ = 5000.0f;
        float lowDistThreshold_ = highDistThreshold_ * 0.9f;
        QVector2D m_rotAngle;
        GraphicsScene3dView* viewPtr_;
        LLARef datasetLlaRef_;
        LLA yerevanLla = LLA(32.262781f, 118.702785f, 0.0f);
        LLARef viewLlaRef_ = LLARef(yerevanLla);

        // 偏航滤波器
        float navYawFilteredRad_        = 0.f;
        bool  navYawInited_             = false;
        QElapsedTimer navYawTmr_;
        float navYawTauSec_             = 1.2;
        float navYawDeadbandRad_        = qDegreesToRadians(2.0f);
        float navYawMaxRateRadPerSec_   = qDegreesToRadians(90.0f);
        float navYawSnapRad_            = qDegreesToRadians(120.0f);
    };

    //Renderer
    class InFboRenderer : public QQuickFramebufferObject::Renderer
    {
    public:
        InFboRenderer();
        virtual ~InFboRenderer();

    protected:
        virtual void render() override;
        virtual void synchronize(QQuickFramebufferObject * fbo) override;
        virtual QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;

    private:
        friend class GraphicsScene3dView;

        // maps
        void processMapTextures(GraphicsScene3dView* viewPtr) const;
        // mosaic on surface
        void processMosaicColorTableTexture(GraphicsScene3dView* viewPtr) const;
        void processMosaicTileTexture(GraphicsScene3dView* viewPtr) const;
        // image
        void processImageTexture(GraphicsScene3dView* viewPtr) const;
        // surface
        void processSurfaceTexture(GraphicsScene3dView* viewPtr) const;

        QString checkOpenGLError() const;

        std::unique_ptr <GraphicsScene3dRenderer> m_renderer;

        GraphicsScene3dView* view_ = nullptr;
    };

    enum ActiveMode{
        Idle                                = 0, // not used
        BottomTrackVertexSelectionMode      = 1,
        BottomTrackVertexComboSelectionMode = 2,
        PolygonCreationMode                 = 3,
        MarkCreationMode                    = 4,
        PolygonEditingMode                  = 5,
        BottomTrackSyncPointCreationMode    = 6,
        ShoreCreationMode                   = 7,
        MeasuringRouteCreationMode          = 8
    };

    /**
     * @brief Constructor
     */
    GraphicsScene3dView();

    /**
     * @brief Destructor
     */
    virtual ~GraphicsScene3dView();

    /**
     * @brief Creates renderer
     * @return renderer
     */
    Renderer *createRenderer() const override;
    std::shared_ptr<BoatTrack>       getBoatTrackPtr() const;
    std::shared_ptr<BottomTrack>     bottomTrack() const;
    std::shared_ptr<IsobathsView>    getIsobathsViewPtr() const;
    std::shared_ptr<SurfaceView>     getSurfaceViewPtr() const;
    std::shared_ptr<ImageView>       getImageViewPtr() const;
    std::shared_ptr<MapView>         getMapViewPtr() const;
    std::shared_ptr<Contacts>        getContactsPtr() const;
    std::shared_ptr<PointGroup>      pointGroup() const;
    std::shared_ptr<PolygonGroup>    polygonGroup() const;
    std::shared_ptr<UsblView>        getUsblViewPtr() const;
    std::shared_ptr<NavigationArrow> getNavigationArrowPtr() const;
    std::weak_ptr <Camera>           camera() const;
    float verticalScale() const;
    bool sceneBoundingBoxVisible() const;
    Dataset* dataset() const;
    double getCurrLat() const;
    double getCurrLon() const;
    void clear(bool cleanMap = false);
    QVector3D calculateIntersectionPoint(const QVector3D &rayOrigin, const QVector3D &rayDirection, float planeZ);
    void updateProjection();
    void setNeedToResetStartPos(bool state);
    void forceUpdateDatasetLlaRef();

    Q_INVOKABLE void switchToBottomTrackVertexComboSelectionMode(qreal x, qreal y);
    Q_INVOKABLE void mousePressTrigger(Qt::MouseButtons mouseButton, qreal x, qreal y, Qt::Key keyboardKey = Qt::Key::Key_unknown);
    Q_INVOKABLE void mouseMoveTrigger(Qt::MouseButtons mouseButton, qreal x, qreal y, Qt::Key keyboardKey = Qt::Key::Key_unknown);
    Q_INVOKABLE void mouseReleaseTrigger(Qt::MouseButtons mouseButton, qreal x, qreal y, Qt::Key keyboardKey = Qt::Key::Key_unknown);
    Q_INVOKABLE void mouseWheelTrigger(Qt::MouseButtons mouseButton, qreal x, qreal y, QPointF angleDelta, Qt::Key keyboardKey = Qt::Key::Key_unknown);
    Q_INVOKABLE void pinchTrigger(const QPointF& prevCenter, const QPointF& currCenter, qreal scaleDelta, qreal angleDelta);
    Q_INVOKABLE void keyPressTrigger(Qt::Key key);
    Q_INVOKABLE void bottomTrackActionEvent(BottomTrack::ActionEvent actionEvent);

    void setTrackLastData(bool state);
    void setTextureIdByTileIndx(const map::TileIndex& tileIndx, GLuint textureId);
    void setGridVisibility(bool state);
    void setUseAngleLocation(bool state);
    void setNavigatorViewLocation(bool state);


protected:
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;


public Q_SLOTS:
    void setSceneBoundingBoxVisible(bool visible);
    void fitAllInView();
    void setIsometricView();
    void setCancelZoomView();
    void setMapView();//将3D视图切换到地图视图模式，也就是正交投影、俯视角度
    void setLastEpochFocusView(bool useAngle, bool useNavigatorView);
    void setIdleMode();
    void setVerticalScale(float scale);
    void shiftCameraZAxis(float shift);
    void setBottomTrackVertexSelectionMode();
    void setPolygonCreationMode();
    void setPolygonEditingMode();
    void onCursorChanged(Qt::CursorShape cursorShape);
    void setDataset(Dataset* dataset);
    void setDataProcessorPtr(DataProcessor* dataProcessorPtr);
    void addPoints(QVector<QVector3D>, QColor color, float width = 1);
    void setQmlRootObject(QObject* object);
    void setQmlAppEngine(QQmlApplicationEngine* engine);
    void updateMapView();
    void updateViews();

    // from DataHorizon
    void onPositionAdded(uint64_t indx);
    void setIsNorth(bool state);

    void slotScreetGraphics();

signals:
    void sendRectRequest(QVector<LLA> rect, bool isPerspective, LLARef viewLlaRef, bool moveUp, map::CameraTilt tiltCam);
    void sendLlaRef(LLARef viewLlaRef);
    void cameraIsMoved();
    void sendMapTextureIdByTileIndx(const map::TileIndex& tileIndx, GLuint textureId);

    void currentLatChanged();
    void currentLonChanged();

private:
    void updateBounds();
    void updatePlaneGrid();
    void clearComboSelectionRect();
    void initAutoDistTimer();
    void calculateLatLong(qreal x, qreal y, double& latitude, double& longitude);

private:
    friend class BottomTrack;
    friend class BoatTrack;

    std::shared_ptr<Camera>       m_camera;
    std::shared_ptr<Camera>       m_axesThumbnailCamera;
    QPointF m_startMousePos = {0.0f, 0.0f};
    QPointF m_lastMousePos = {0.0f, 0.0f};
    std::shared_ptr<RayCaster>    m_rayCaster;
    std::shared_ptr<IsobathsView> isobathsView_;
    std::shared_ptr<SurfaceView>  surfaceView_;
    std::shared_ptr<ImageView>    imageView_;//管理和渲染多个瓦片组成地图
    std::shared_ptr<MapView>      mapView_;  //渲染单个图片/纹理
    std::shared_ptr<Contacts>     contacts_;
    std::shared_ptr<BoatTrack>    boatTrack_;
    std::shared_ptr<BottomTrack>  m_bottomTrack;
    std::shared_ptr<PolygonGroup> m_polygonGroup;
    std::shared_ptr<PointGroup>   m_pointGroup;
    std::shared_ptr<CoordinateAxes> m_coordAxes;
    std::shared_ptr<PlaneGrid>    m_planeGrid; //辅助网格
    std::shared_ptr<SceneObject>  m_vertexSynchroCursour;
    std::shared_ptr<NavigationArrow> navigationArrow_;
    std::shared_ptr<UsblView>     usblView_;

    QMatrix4x4 m_model;
    QMatrix4x4 m_projection;
    Cube m_bounds;
    ActiveMode m_mode    = ActiveMode::BottomTrackVertexSelectionMode;
    ActiveMode lastMode_ = ActiveMode::BottomTrackVertexSelectionMode;
    QRect m_comboSelectionRect = { 0, 0, 0, 0 };
    Ray m_ray;
    float m_verticalScale = 1.0f;
    bool m_isSceneBoundingBoxVisible = true;
    Dataset* datasetPtr_ = nullptr;
    DataProcessor* dataProcessorPtr_ = nullptr;
#if defined (Q_OS_ANDROID) || defined (LINUX_ES)
    static constexpr double mouseThreshold_{ 15.0 };
#else
    static constexpr double mouseThreshold_{ 10.0 };
#endif

    static constexpr float perspectiveEdge_{ 5000.0f };
    static constexpr float nearPlanePersp_{ 1.0f };
    static constexpr float farPlanePersp_{ 20000.0f };
    static constexpr float nearPlaneOrthoCoeff_{ 0.05f };
    static constexpr float farPlaneOrthoCoeff_{ 1.2f };

    bool wasMoved_;
    Qt::MouseButtons wasMovedMouseButton_;
    QObject* qmlRootObject_;
    bool switchedToBottomTrackVertexComboSelectionMode_;
    bool needToResetStartPos_;
    float lastCameraDist_;
    bool trackLastData_;
    bool gridVisibility_;
    bool useAngleLocation_;
    bool navigatorViewLocation_;
    bool isNorth_;
    QTimer* testingTimer_;


    double currentLat_ = 0.0;
    double currentLon_ = 0.0;

    int currentMapLevel_,screetCurrentMapLevel_;



public:
    Q_INVOKABLE void setScreenMode(bool isScreen);
    void setCurrentMapLevel(int mapLevel);
    Q_PROPERTY(QWidget* screetShot READ screetShot CONSTANT)
    QWidget* screetShot() { return &screetShot_; }


    ScreetShot screetShot_;
    bool screenshotPending_ = false;
    QString screenshotPath_ = ".20260211.png";
    QMutex screenshotMutex_;
    int mapLevel_;
    QRect targetRect_;
    int pixel300m_;
    QString rowStr_,colStr_;


signals:
    void requestRenderUpdate();


public slots:

    // 截图任务处理方法
    void processNextScreenshotTask();

private:
    // 截图任务队列
    QQueue<ScreenshotTask> screenshotQueue_;
    QMutex screenshotQueueMutex_;//要定义为成员变量，而不是局部变量（会导致每个线程都有自己的mutex，没有互斥效果）
    bool isProcessingScreenshot_ = false;

    // 当前处理的截图任务
    ScreenshotTask currentScreenshotTask_;

    void startScreenshotTask(const ScreenshotTask& task);
    bool tilesRenderComplete_ = false; //当前瓦片渲染完成标志

    int screenshotRetryCount_ = 0;
    static const int MAX_RETRY_COUNT = 100;  // 最大重试次数


private:
    QOpenGLFramebufferObject* offscreenFbo_ = nullptr;
    GLuint offscreenVboPos_ = 0;
    GLuint offscreenVboUV_ = 0;
    int offscreenFboWidth_ = 0;
    int offscreenFboHeight_ = 0;

    bool initOffscreenFbo(int width, int height);
    void cleanupOffscreenFbo();
    QImage renderTilesToOffscreenFbo(const QVector<map::TileIndex>& tileIndices,
        const std::unordered_map<map::TileIndex, QImage>& tileImages,
        float pixelMinX, float pixelMinY,float pixelMaxX, float pixelMaxY);

    // 定时器用于定期触发updateMapView
    QTimer* updateTimer_ = nullptr;


public:
    void saveTilesToOffscreenPng(const QString& filePath,
        double minLat, double maxLat, double minLon, double maxLon,
        int mapLevel, int outputWidth = 2048, int outputHeight = 2048);


    float minZ_, maxZ_;

};

#endif // GRAPHICSSCENE3DVIEW_H
