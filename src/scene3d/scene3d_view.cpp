#include "scene3d_view.h"
#include <cmath>
#include <memory.h>
#include <math.h>
#include <QOpenGLFramebufferObject>
#include <QVector3D>


#include "scene3d_renderer.h"
#include "dataset.h"
#include "map_defs.h"


/*---------------------------------------GraphicsScene3dView---------------------------------------*/
GraphicsScene3dView::GraphicsScene3dView() :
    QQuickFramebufferObject(),
    m_camera(std::make_shared<Camera>(this)),
    m_axesThumbnailCamera(std::make_shared<Camera>()),
    m_rayCaster(std::make_shared<RayCaster>()),
    isobathsView_(std::make_shared<IsobathsView>()),
    surfaceView_(std::make_shared<SurfaceView>()),
    imageView_(std::make_shared<ImageView>()),
    mapView_(std::make_shared<MapView>(this)),
    boatTrack_(std::make_shared<BoatTrack>(this, this)),
    polygonOutline_(std::make_shared<PolygonOutline>(this, this)),
    m_bottomTrack(std::make_shared<BottomTrack>(this, this)),
    m_polygonGroup(std::make_shared<PolygonGroup>()),
    m_pointGroup(std::make_shared<PointGroup>()),
    m_coordAxes(std::make_shared<CoordinateAxes>()),
    m_planeGrid(std::make_shared<PlaneGrid>()),
    navigationArrow_(std::make_shared<NavigationArrow>()),
    wasMoved_(false),
    wasMovedMouseButton_(Qt::MouseButton::NoButton),
    qmlRootObject_(nullptr),
    needToResetStartPos_(false),
    lastCameraDist_(m_camera->distForMapView()),
    gridVisibility_(true),
    isNorth_(false)
{
    setObjectName("GraphicsScene3dView");
    setMirrorVertically(true);
    setAcceptedMouseButtons(Qt::AllButtons);

    m_camera->setCameraListener(m_axesThumbnailCamera.get());

    imageView_->setView(this);

    QObject::connect(isobathsView_.get(), &IsobathsView::changed, this, &QQuickFramebufferObject::update);
    QObject::connect(surfaceView_.get(),  &SurfaceView::changed,  this, &QQuickFramebufferObject::update);
    QObject::connect(imageView_.get(),    &ImageView::changed,    this, &QQuickFramebufferObject::update);
    QObject::connect(mapView_.get(),      &MapView::changed,      this, &QQuickFramebufferObject::update);
    QObject::connect(boatTrack_.get(),    &BoatTrack::changed,    this, &QQuickFramebufferObject::update);
    QObject::connect(polygonOutline_.get(), &PolygonOutline::changed, this, &QQuickFramebufferObject::update);
    QObject::connect(m_bottomTrack.get(), &BottomTrack::changed,  this, &QQuickFramebufferObject::update);
    QObject::connect(m_polygonGroup.get(), &PolygonGroup::changed, this, &QQuickFramebufferObject::update);
    QObject::connect(m_coordAxes.get(),   &CoordinateAxes::changed, this, &QQuickFramebufferObject::update);
    QObject::connect(m_planeGrid.get(),   &PlaneGrid::changed,    this, &QQuickFramebufferObject::update);
    QObject::connect(navigationArrow_.get(), &NavigationArrow::changed, this, &QQuickFramebufferObject::update);


    QObject::connect(isobathsView_.get(), &IsobathsView::boundsChanged, this, &GraphicsScene3dView::updateBounds);
    QObject::connect(surfaceView_.get(),  &SurfaceView::boundsChanged,  this, &GraphicsScene3dView::updateBounds);
    QObject::connect(imageView_.get(),    &ImageView::boundsChanged,    this, &GraphicsScene3dView::updateBounds);
    QObject::connect(mapView_.get(),      &MapView::boundsChanged,      this, &GraphicsScene3dView::updateBounds);
    QObject::connect(m_bottomTrack.get(), &BottomTrack::boundsChanged,  this, &GraphicsScene3dView::updateBounds);
    QObject::connect(polygonOutline_.get(), &PolygonOutline::boundsChanged,  this, &GraphicsScene3dView::updateBounds);
    QObject::connect(m_polygonGroup.get(), &PolygonGroup::boundsChanged, this, &GraphicsScene3dView::updateBounds);
    QObject::connect(m_coordAxes.get(),   &CoordinateAxes::boundsChanged, this, &GraphicsScene3dView::updateBounds);
    QObject::connect(boatTrack_.get(),    &PlaneGrid::boundsChanged,    this, &GraphicsScene3dView::updateBounds);
    QObject::connect(navigationArrow_.get(), &NavigationArrow::boundsChanged, this, &GraphicsScene3dView::updateBounds);

    QObject::connect(this, &GraphicsScene3dView::cameraIsMoved, this, &GraphicsScene3dView::updateMapView, Qt::DirectConnection);
    QObject::connect(this, &GraphicsScene3dView::cameraIsMoved, this, &GraphicsScene3dView::updateViews, Qt::DirectConnection);

    connect(&screetShot_, &ScreetShot::signalScreetGraphics, this, &GraphicsScene3dView::slotScreetGraphics, Qt::DirectConnection);
}

GraphicsScene3dView::~GraphicsScene3dView()
{
}

QQuickFramebufferObject::Renderer *GraphicsScene3dView::createRenderer() const
{
    return new GraphicsScene3dView::InFboRenderer();
}

std::shared_ptr<BoatTrack> GraphicsScene3dView::getBoatTrackPtr() const
{
    return boatTrack_;
}

std::shared_ptr<BottomTrack> GraphicsScene3dView::bottomTrack() const
{
    return m_bottomTrack;
}

std::shared_ptr<PolygonOutline> GraphicsScene3dView::polygonOutline() const
{
    return polygonOutline_;
}

std::shared_ptr<IsobathsView> GraphicsScene3dView::getIsobathsViewPtr() const
{
    return isobathsView_;
}

std::shared_ptr<SurfaceView> GraphicsScene3dView::getSurfaceViewPtr() const
{
    return surfaceView_;
}

std::shared_ptr<ImageView> GraphicsScene3dView::getImageViewPtr() const
{
    return imageView_;
}

std::shared_ptr<MapView> GraphicsScene3dView::getMapViewPtr() const
{
    return mapView_;
}

std::shared_ptr<PointGroup> GraphicsScene3dView::pointGroup() const
{
    return m_pointGroup;
}

std::shared_ptr<PolygonGroup> GraphicsScene3dView::polygonGroup() const
{
    return m_polygonGroup;
}

std::shared_ptr<NavigationArrow> GraphicsScene3dView::getNavigationArrowPtr() const
{
    return navigationArrow_;
}

std::weak_ptr<GraphicsScene3dView::Camera> GraphicsScene3dView::camera() const
{
    return m_camera;
}

float GraphicsScene3dView::verticalScale() const
{
    return m_verticalScale;
}

bool GraphicsScene3dView::sceneBoundingBoxVisible() const
{
    return m_isSceneBoundingBoxVisible;
}

Dataset *GraphicsScene3dView::dataset() const
{
    return datasetPtr_;
}

double GraphicsScene3dView::getCurrLat() const
{
    return currentLat_;
}

double GraphicsScene3dView::getCurrLon() const
{
    return currentLon_;
}

void GraphicsScene3dView::setCursorShape(Qt::CursorShape shape)
{
    if (cursorShape_ != shape) {
        cursorShape_ = shape;
    }
    emit cursorShapeChanged();
}

void GraphicsScene3dView::clear(bool isClearTrack, bool cleanMap)
{
    if(isClearTrack) {
        boatTrack_->clearData();
        m_bottomTrack->clearData();
        navigationArrow_->clearData();
    }

    isobathsView_->clear();
    surfaceView_->clear();
    imageView_->clear();
    if (cleanMap) {
        mapView_->clear();
    }
    m_polygonGroup->clearData();
    polygonOutline_->clearData();
    m_bounds = Cube();
    updateBounds();

    QQuickFramebufferObject::update();
}

QVector3D GraphicsScene3dView::calculateIntersectionPoint(const QVector3D &rayOrigin, const QVector3D &rayDirection, float planeZ)
{
    QVector3D retVal;

    if (qAbs(rayDirection.z()) < 1e-6) {
        return retVal;
    }
    const float t = (planeZ - rayOrigin.z()) / rayDirection.z();

    if (t < 0) {
        return retVal;
    }
    retVal = rayOrigin + rayDirection * t;

    return retVal;
}

void GraphicsScene3dView::mousePressTrigger(Qt::MouseButtons mouseButton, qreal x, qreal y, Qt::Key keyboardKey)
{
    Q_UNUSED(keyboardKey)

    //当前点x,y的经纬度坐标
    calculateLatLong(x, y, currentLat_, currentLon_);
    emit currentLatChanged();
    emit currentLonChanged();

    QPoint pos = QPoint(x,y);

    if (mouseButton == Qt::LeftButton)
    {
        /*-- 截图模式 --*/
        if(screetShot_.isScreenMode_)
        {
            if(!screetShot_.firstScreenDown_) {
                screetShot_.startPos_ = QPointF(x, y);
                screetShot_.shotRect_ = QRectF(x, y, 0, 0);
                screetShot_.setSelectionRectVisible(true);
                screetShot_.setSelectionRect(screetShot_.shotRect_);
            }
            else {
                screetShot_.judgeResizeMode(screetShot_.shotRect_,pos); //确认调整模式
                if(screetShot_.resizeMode_ == ResizeMode::Move) {
                    screetShot_.endPos_ = pos;
                    screetShot_.dragging_ = true;
                }
            }

            return;
        }

        /*--- 测距模块 ---*/
        if(screetShot_.isDistMeasureMode_)
        {
            if(screetShot_.isDrawMeasure_ == 0) {
                screetShot_.isDrawMeasure_ = 1;
                screetShot_.setP1Visible(true);
                screetShot_.setDistLineP1(QPointF(x, y));
                screetShot_.worldCoorOrigin_ = calculateToWorldCoor(x, y);
                emit screetShot_.signalStartToEndDist(0);
            }
            else if(screetShot_.isDrawMeasure_ == 1) {
                screetShot_.isDrawMeasure_ = 2;
                screetShot_.setP2Visible(true);
                screetShot_.setDistLineP2(QPointF(x, y));
                QVector3D worldCoorEnd = calculateToWorldCoor(x, y);
                const float dx = (worldCoorEnd.x() - screetShot_.worldCoorOrigin_.x());
                const float dy = (worldCoorEnd.y() - screetShot_.worldCoorOrigin_.y());
                double dist = std::sqrt(dx * dx + dy * dy);
                emit screetShot_.signalStartToEndDist(dist);
                QGuiApplication::setOverrideCursor(Qt::ArrowCursor);
            }
        }

        /*- 绘制多边形轮廓模式 -*/
        if(polygonOutline_->getOutlineMode()) {
            polygonOutline_->polygonAddPoint(currentLat_, currentLon_);
        }
        else {
            if(!polygonOutline_->getDraggingPoint() && polygonOutline_->draggingPtIndex_ >= 0) {
                polygonOutline_->setDraggingPoint(true);
                setCursorShape(Qt::CrossCursor);
            }
            else if(polygonOutline_->getDraggingPoint()) {
                polygonOutline_->setDraggingPoint(false);
                polygonOutline_->draggingPtIndex_ = -1;
                setCursorShape(Qt::ArrowCursor);
            }
        }
    }

    wasMoved_ = false;

    if (qmlRootObject_) { // maybe this will be removed
        if (auto selectionToolButton = qmlRootObject_->findChild<QObject*>("selectionToolButton"); selectionToolButton) {
            selectionToolButton->property("checked").toBool() ?
                m_mode = ActiveMode::BottomTrackVertexSelectionMode : m_mode = ActiveMode::Idle;
        }
    }

    m_camera->m_lookAtSave = m_camera->m_lookAt;

    m_startMousePos = {x, y};
    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::mouseDoubleClickTrigger(Qt::MouseButtons mouseButton, qreal x, qreal y, Qt::Key keyboardKey)
{
    Q_UNUSED(keyboardKey)

    completeDrawOutline();
}

void GraphicsScene3dView::mouseMoveTrigger(Qt::MouseButtons mouseButton, qreal x, qreal y, Qt::Key keyboardKey)
{
    bool cameraWasMoved{ false };
    if (needToResetStartPos_) {
        m_camera->m_lookAtSave = m_camera->m_lookAt;
        m_startMousePos = QPointF(x, y);
        needToResetStartPos_ = false;
    }

    QPoint pos = QPoint(x,y);

    if(screetShot_.isScreenMode_)
    {
        if(!screetShot_.firstScreenDown_) {
            QGuiApplication::setOverrideCursor(Qt::CrossCursor);
            if(mouseButton == Qt::LeftButton) {
                QPointF currentPos(x, y);
                qreal width  = currentPos.x() - screetShot_.startPos_.x();
                qreal height = currentPos.y() - screetShot_.startPos_.y();
                QRectF shotRect = QRectF(std::min(screetShot_.startPos_.x(), currentPos.x()),
                std::min(screetShot_.startPos_.y(), currentPos.y()), std::abs(width), std::abs(height));
                calculateLatLong(shotRect.topLeft().x(), shotRect.topLeft().y(),
                                screetShot_.topLeftLati_,screetShot_.topLeftLong_);
                calculateLatLong(shotRect.bottomRight().x(), shotRect.bottomRight().y(),
                                screetShot_.bottomRightLati_, screetShot_.bottomRightLong_);
                screetShot_.setSelectionRect(shotRect);
                // qDebug() << "Screen capture rect:" << screetShot_.shotRect_;
            }
        }
        else {
            screetShot_.judgeResizeMode(screetShot_.shotRect_, pos);
            if(mouseButton == Qt::LeftButton) {
                screetShot_.resizeMode(screetShot_.shotRect_, pos);
                calculateLatLong(screetShot_.shotRect_.topLeft().x(), screetShot_.shotRect_.topLeft().y(),
                                screetShot_.topLeftLati_, screetShot_.topLeftLong_);
                calculateLatLong(screetShot_.shotRect_.bottomRight().x(), screetShot_.shotRect_.bottomRight().y(),
                                screetShot_.bottomRightLati_, screetShot_.bottomRightLong_);
                screetShot_.setSelectionRect(screetShot_.shotRect_);
            }
        }

        return;
    }

    /*-- 测距模块 --*/
    if(screetShot_.isDistMeasureMode_)
    {
        if(screetShot_.isDrawMeasure_ == 1) {
            screetShot_.setDistLineP2(QPointF(x, y));
            QVector3D worldCoorEnd = calculateToWorldCoor(x, y);
            const float dx = (worldCoorEnd.x() - screetShot_.worldCoorOrigin_.x());
            const float dy = (worldCoorEnd.y() - screetShot_.worldCoorOrigin_.y());
            double dist = std::sqrt(dx * dx + dy * dy);
            emit screetShot_.signalStartToEndDist(dist);
        }
    }

    if(screetShot_.getLandMarkMode())
    {
        if(mouseButton == Qt::LeftButton) {
            screetShot_.setLandMarkPtX(x);
            screetShot_.setLandMarkPtY(y);
            double landMarkLat, landMarkLon;
            calculateLatLong(x, y, landMarkLat, landMarkLon);
            screetShot_.setSpotLatitude(QString::number(landMarkLat) + "°");
            screetShot_.setSpotLongitude(QString::number(landMarkLon) + "°");
        }

        return;
    }


    /*- 绘制多边形轮廓模式 -*/
    if(polygonOutline_->getOutlineMode()) {
        setCursorShape(Qt::PointingHandCursor);
    }
    else if (polygonOutline_->getDraggingPoint() && (polygonOutline_->draggingPtIndex_ >= 0))
    {
        calculateLatLong(x, y, currentLat_, currentLon_);
        LLA lla = LLA(currentLat_, currentLon_);
        polygonOutline_->modifyPolygonVertex(polygonOutline_->draggingPtIndex_, lla);
    }
    else if (!polygonOutline_->getOutlineMode() && !datasetPtr_->isEmptyPolygon())
    {
        calculateLatLong(x, y, currentLat_, currentLon_);
        LLA lla = LLA(currentLat_, currentLon_);
        polygonOutline_->draggingPtIndex_ = polygonOutline_->getNearestVertexIndex(lla);
    }

    // movement threshold for sync
    if (!wasMoved_) {
        double dist{ std::sqrt(std::pow(x - m_startMousePos.x(), 2) + std::pow(y - m_startMousePos.y(), 2)) };
        if (dist > mouseThreshold_) {
            wasMoved_ = true;
            if (wasMovedMouseButton_ != mouseButton)
                wasMovedMouseButton_ = mouseButton;
        }
    }

    // ray for marker
    auto toOrig = QVector3D(x, height() - y, -1.0f).unproject(m_camera->m_view * m_model, m_projection, boundingRect().toRect());
    auto toEnd  = QVector3D(x, height() - y, 1.0f).unproject(m_camera->m_view * m_model, m_projection, boundingRect().toRect());
    auto toDir  = (toEnd - toOrig).normalized();
    auto to     = calculateIntersectionPoint(toOrig, toDir, 0);
    m_ray.setOrigin(toOrig);
    m_ray.setDirection(toDir);

    // if (switchedToBottomTrackVertexComboSelectionMode_) {
    //     m_comboSelectionRect.setBottomRight({static_cast<int>(x), static_cast<int>(height() - y)});
    //     m_bottomTrack->mouseMoveEvent(mouseButton, x, y);
    // }
    // else {
#if defined(Q_OS_ANDROID)
        Q_UNUSED(keyboardKey);
        auto fromOrig = QVector3D(m_startMousePos.x(), height() - m_startMousePos.y(), -1.0f).unproject(m_camera->m_view * m_model, m_projection, boundingRect().toRect());
        auto fromEnd = QVector3D(m_startMousePos.x(), height() - m_startMousePos.y(), 1.0f).unproject(m_camera->m_view * m_model, m_projection, boundingRect().toRect());
        auto fromDir = (fromEnd - fromOrig).normalized();
        auto from = calculateIntersectionPoint(fromOrig, fromDir , 0);
        m_camera->move(QVector2D(from.x(), from.y()), QVector2D(to.x() ,to.y()));
        cameraWasMoved = true;
#else
        if (mouseButton.testFlag(Qt::LeftButton) && (keyboardKey == Qt::Key_Control)) {
            if (m_camera->getIsPerspective() && !isNorth_) {
                m_camera->rotate(QVector2D(m_lastMousePos), QVector2D(x, y));
                m_axesThumbnailCamera->rotate(QVector2D(m_lastMousePos), QVector2D(x, y));
                m_startMousePos = {x, y};
                cameraWasMoved = true;
            }
        }
        else if (mouseButton.testFlag(Qt::LeftButton)) {
            auto fromOrig = QVector3D(m_startMousePos.x(), height() - m_startMousePos.y(), -1.0f).unproject(m_camera->m_view * m_model, m_projection, boundingRect().toRect());
            auto fromEnd = QVector3D(m_startMousePos.x(), height() - m_startMousePos.y(), 1.0f).unproject(m_camera->m_view * m_model, m_projection, boundingRect().toRect());
            auto fromDir = (fromEnd - fromOrig).normalized();
            auto from = calculateIntersectionPoint(fromOrig, fromDir , 0);
            m_camera->move(QVector2D(from.x(), from.y()), QVector2D(to.x() ,to.y()));
            cameraWasMoved = true;
        }
#endif
    // }

    m_lastMousePos = { x, y };
    QQuickFramebufferObject::update();

    if (cameraWasMoved) {
        emit cameraIsMoved();
    }
}

void GraphicsScene3dView::mouseReleaseTrigger(Qt::MouseButtons mouseButton, qreal x, qreal y, Qt::Key keyboardKey)
{
    Q_UNUSED(keyboardKey);

    // clearComboSelectionRect();

    QPoint pos = QPoint(x, y);

    m_lastMousePos = {x, y};


    /*- 截图模式 -*/
    if(screetShot_.isScreenMode_)
    {
        if(!screetShot_.firstScreenDown_) {
            screetShot_.firstScreenDown_ = true;
        }

        screetShot_.setScreetToolBar(true);
        qDebug() << "Screen capture completed";
        return;
    }

    // if (switchedToBottomTrackVertexComboSelectionMode_) {
    //     m_mode = lastMode_;
    //     m_bottomTrack->mouseReleaseEvent(mouseButton, x, y);
    // }

    if (!wasMoved_ && wasMovedMouseButton_ == Qt::MouseButton::NoButton) {
        m_bottomTrack->resetVertexSelection();
        boatTrack_->clearSelectedEpoch();
        m_bottomTrack->mousePressEvent(Qt::MouseButton::LeftButton, x, y);
        boatTrack_->mousePressEvent(Qt::MouseButton::LeftButton, x, y);
    }

    // switchedToBottomTrackVertexComboSelectionMode_ = false;
    wasMoved_ = false;
    wasMovedMouseButton_ = Qt::MouseButton::NoButton;

    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::mouseWheelTrigger(Qt::MouseButtons mouseButton, qreal x, qreal y, QPointF angleDelta, Qt::Key keyboardKey)
{
    bool cameraWasMoved{ false };
    Q_UNUSED(mouseButton)
    Q_UNUSED(x)
    Q_UNUSED(y)

    if (needToResetStartPos_) {
        m_camera->m_lookAtSave = m_camera->m_lookAt;
        m_startMousePos = QPointF(x, y);
        needToResetStartPos_ = false;
    }

    if (keyboardKey == Qt::Key_Control) {
        float tempVerticalScale = m_verticalScale;
        angleDelta.y() > 0.0f ? tempVerticalScale += 0.3f : tempVerticalScale -= 0.3f;
        setVerticalScale(tempVerticalScale);
    }
    else if (keyboardKey == Qt::Key_Shift) {
        if (!isNorth_) {
            angleDelta.y() > 0.0f ? shiftCameraZAxis(5) : shiftCameraZAxis(-5);
            cameraWasMoved = true;
        }
    }
    else {
        m_camera->zoom(angleDelta.y());
        cameraWasMoved = true;
    }

    QQuickFramebufferObject::update();

    if (cameraWasMoved) {
        emit cameraIsMoved();
    }

    updateDistance();
}

void GraphicsScene3dView::pinchTrigger(const QPointF& prevCenter, const QPointF& currCenter, qreal scaleDelta, qreal angleDelta)
{
    if(polygonOutline_->getOutlineMode()) {
        return;
    }

    m_camera->zoom(scaleDelta);   

    if (!isNorth_) {
        m_camera->rotate(prevCenter, currCenter, angleDelta, height());
        m_axesThumbnailCamera->rotate(prevCenter, currCenter, angleDelta , height());
    }

    QQuickFramebufferObject::update();

    emit cameraIsMoved();

    updateDistance();
}


void GraphicsScene3dView::zoomInOut(bool zoomIn)
{
    qreal delta = zoomIn ? 120.0 : -120.0;
#ifdef Q_OS_WIN
    m_camera->zoom(delta);
#elif defined(Q_OS_ANDROID)
    m_camera->zoomAndroid(delta);
#endif

    QQuickFramebufferObject::update();

    emit cameraIsMoved();

    updateDistance();
}

void GraphicsScene3dView::completeDrawOutline()
{
    if(polygonOutline_->getOutlineMode()) {
        if(datasetPtr_->endPolygonOutlineindex() > 1) {
            setOutlineCompleted(true);
            polygonOutline_->setOutlineMode(false);
            setCursorShape(Qt::ArrowCursor);
            GIF->dialogInfo(Dialog_OK, tr("Isobaths Outline Create Successful"));
        }
        else {
            GIF->dialogInfo(Dialog_OK, tr("Boundary Curve is Incomplete!"));
        }
    }
}

void GraphicsScene3dView::keyPressTrigger(Qt::Key key)
{
    m_bottomTrack->keyPressEvent(key);

    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::bottomTrackActionEvent(BottomTrack::ActionEvent actionEvent)
{
    m_bottomTrack->actionEvent(actionEvent);

    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::setCurrentMapLevel(int mapLevel)
{
    // qDebug() << "mapLevel...." << mapLevel;
    screetShot_.currMapLevel_ = mapLevel;
}

void GraphicsScene3dView::setScreenMode(bool isScreen)
{
    screetShot_.isScreenMode_ = isScreen;

    if (m_camera && m_camera->getIsPerspective()) {
        m_camera->resetRotationAngle();
        if (m_axesThumbnailCamera) {
            m_axesThumbnailCamera->resetRotationAngle();
        }

        m_camera->resetZAxis();
        updateProjection();
    }

    QQuickFramebufferObject::update();
    emit cameraIsMoved();

    screetShot_.shotRect_ = QRectF();

    if(isScreen) {
        screetShot_.firstScreenDown_ = false;
        screetCurrentMapLevel_ = currentMapLevel_;
        screetShot_.setLLARef(m_camera->viewLlaRef_, m_camera->getIsPerspective());
    }
    else {
        QGuiApplication::setOverrideCursor(Qt::ArrowCursor);
        screetShot_.setSelectionRectVisible(false);
    }

}

void GraphicsScene3dView::setDistMeasureMode(bool isDist)
{
    screetShot_.isDistMeasureMode_ = isDist;
    if (m_camera && m_camera->getIsPerspective()) {
        m_camera->resetRotationAngle();
        if (m_axesThumbnailCamera) {
            m_axesThumbnailCamera->resetRotationAngle();
        }

        m_camera->resetZAxis();
        updateProjection();
    }

    QQuickFramebufferObject::update();
    emit cameraIsMoved();

    if(isDist) {
        QGuiApplication::setOverrideCursor(Qt::PointingHandCursor);
        screetShot_.setLLARef(m_camera->viewLlaRef_, m_camera->getIsPerspective());
    }
    else {
        QGuiApplication::setOverrideCursor(Qt::ArrowCursor);
        screetShot_.setP1Visible(false);
        screetShot_.setP2Visible(false);
    }

    screetShot_.isDrawMeasure_ = 0;
}

void GraphicsScene3dView::setLandMarkMode(bool mark)
{
    screetShot_.setLandMarkMode(mark);
    if(mark) {
        screetShot_.setLandMarkPtX(screetShot_.getLandMarkPtX());
        screetShot_.setLandMarkPtY(screetShot_.getLandMarkPtY());
        double landMarkLat, landMarkLon;
        calculateLatLong(screetShot_.getLandMarkPtX(), screetShot_.getLandMarkPtX(), landMarkLat, landMarkLon);
        screetShot_.setSpotLatitude(QString::number(landMarkLat) + "°");
        screetShot_.setSpotLongitude(QString::number(landMarkLon) + "°");
    }
}

void GraphicsScene3dView::setTextureIdByTileIndx(const map::TileIndex &tileIndx, GLuint textureId)
{
    emit sendMapTextureIdByTileIndx(tileIndx, textureId);
}

void GraphicsScene3dView::setGridVisibility(bool state)
{
    gridVisibility_ = state;

    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::updateProjection()
{
    QMatrix4x4 currProj;
    if (m_camera) {
        //这里有个bug，14等级时，地图等级切换会出现抖动现象！！！
        float aspectRatio = width()/height();
        if (m_camera->getIsPerspective()) { //当地图等级大于14的某个值时为perspective透视投影
            float coeff = m_camera->getHeightAboveGround() / perspectiveEdge_;
            qreal fixFov = m_camera->fov() + m_camera->fov()*coeff;
            // qDebug() << "coeff: " << coeff << "............  :fixFov:" <<fixFov;
            currProj.perspective(fixFov, aspectRatio, nearPlanePersp_, farPlanePersp_);
        }
        else {  //当地图等级小于14的某个值时为ortho正交投影
            float orthV = m_camera->getHeightAboveGround();
            // qDebug() << "orthV:.........." << orthV << "        aspectRatio:" << aspectRatio;
            currProj.ortho(-orthV*aspectRatio, orthV*aspectRatio, -orthV, orthV, orthV*nearPlaneOrthoCoeff_, orthV*farPlaneOrthoCoeff_);
        }

        m_projection = std::move(currProj);
    }
}

void GraphicsScene3dView::setNeedToResetStartPos(bool state)
{
    needToResetStartPos_ = state;
}

void GraphicsScene3dView::forceUpdateDatasetLlaRef()
{
    if (datasetPtr_) {
        auto ref = datasetPtr_->getLlaRef();
        m_camera->datasetLlaRef_ = ref.isInit ? ref : LLARef(m_camera->startupInitLla);
    }

    m_camera->viewLlaRef_ = m_camera->datasetLlaRef_;
    mapView_->setViewLlaRef(m_camera->viewLlaRef_);

    QQuickFramebufferObject::update();
}


void GraphicsScene3dView::ensureInView(const QVector3D& worldPos)
{
    if (!m_camera) {
        return;
    }

    const QVector3D lookAt = m_camera->m_lookAt;
    const float dx = worldPos.x() - lookAt.x();
    const float dy = worldPos.y() - lookAt.y();
    const float dist = std::sqrt(dx * dx + dy * dy);

    float viewRadius;
    if (m_camera->getIsPerspective()) {
        const float halfFovRad = m_camera->fov() * 0.5f * M_PI / 180.0f;
        viewRadius = m_camera->distToFocusPoint() * std::tan(halfFovRad);
    }
    else {
        viewRadius = m_camera->distToFocusPoint() * 0.5f;
    }

    if ((viewRadius > 0) && (dist >= viewRadius)) {
        m_camera->focusOnPosition(QVector3D(worldPos.x(), worldPos.y(), lookAt.z()));
        QQuickFramebufferObject::update();
        emit cameraIsMoved();
    }
}

void GraphicsScene3dView::focusTrackBounds()
{
    if (!m_camera || !datasetPtr_) {
        return;
    }

    float minX = datasetPtr_->minX_;
    float maxX = datasetPtr_->maxX_;
    float minY = datasetPtr_->minY_;
    float maxY = datasetPtr_->maxY_;
    if(minX >= maxX || minY >= maxY) {
        return;
    }

    QVector3D center((minX + maxX) * 0.5f, (minY + maxY) * 0.5f, 0);

    float width  = maxX - minX;
    float height = maxY - minY;
    float maxSize = std::max(width, height);

    // 轨迹区域占70%
    constexpr float VIEW_RATIO = 0.70f;
    float targetViewSize = maxSize / VIEW_RATIO;


    //正交模式
    if(!m_camera->getIsPerspective()) {
        float orthV = targetViewSize * 0.5f;
        m_camera->setDistance(orthV);
    }
    else {
    //透视模式，根据FOV计算距离
        float fov = qDegreesToRadians(m_camera->fov());
        float distance = (targetViewSize*0.5f) / tan(fov*0.5f);
        m_camera->setDistance(distance);
    }

    m_camera->focusOnPosition(center);
    updateProjection();
    QQuickFramebufferObject::update();
    emit cameraIsMoved();
}

void GraphicsScene3dView::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickFramebufferObject::geometryChanged(newGeometry, oldGeometry);

    if (newGeometry.size() != oldGeometry.size()) {
       updateProjection();
       emit cameraIsMoved();
    }
}

void GraphicsScene3dView::setSceneBoundingBoxVisible(bool visible)
{
    m_isSceneBoundingBoxVisible = visible;

    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::fitAllInView()
{
    auto maxSize = std::max(m_bounds.width(), std::max(m_bounds.height(), m_bounds.length()));
    auto d = (maxSize/2.0f)/(std::tan(m_camera->fov() * 0.5f)) * 2.0f;
    if(d>0) m_camera->setDistance(d);

    m_camera->focusOnPosition(m_bounds.center());

    QQuickFramebufferObject::update();

    emit cameraIsMoved();
}

void GraphicsScene3dView::setIsometricView()
{
    m_camera->setIsometricView();
    m_axesThumbnailCamera->setIsometricView();

    fitAllInView();

    QQuickFramebufferObject::update();

    emit cameraIsMoved();
}

void GraphicsScene3dView::setCancelZoomView()
{
    m_verticalScale = 1.0f;

    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::setMapView()
{
    LLARef llaRef = datasetPtr_->getLlaRef();
    m_camera->viewLlaRef_ = llaRef.isInit ? llaRef : LLARef(m_camera->startupInitLla);
    mapView_->setViewLlaRef(m_camera->viewLlaRef_);

    m_camera->setMapView();
    m_axesThumbnailCamera->setMapView();

    fitAllInView();

    QQuickFramebufferObject::update();

    emit cameraIsMoved();
}

void GraphicsScene3dView::setIdleMode()
{
    m_mode = Idle;

    // clearComboSelectionRect();
    m_bottomTrack->resetVertexSelection();
    boatTrack_->clearSelectedEpoch();

    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::setVerticalScale(float scale)
{
    if(m_verticalScale == scale) return;
    else if(scale < 0.05f)  m_verticalScale = -0.05f;
    else if(scale > 10.f)   m_verticalScale = -10.0f;
    else                    m_verticalScale = -scale;

    if (surfaceView_) {
        surfaceView_->setVerticalScale(m_verticalScale);
    }

    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::onCursorChanged(Qt::CursorShape cursorShape)
{
    setCursor(cursorShape);
}

void GraphicsScene3dView::shiftCameraZAxis(float shift)
{
    m_camera->moveZAxis(shift);

    emit cameraIsMoved();
}

void GraphicsScene3dView::setBottomTrackVertexSelectionMode()
{
    setIdleMode();
    m_mode = BottomTrackVertexSelectionMode;
    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::setPolygonCreationMode()
{
    setIdleMode();
    m_mode = PolygonCreationMode;
    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::setPolygonEditingMode()
{
    setIdleMode();
    m_mode = PolygonEditingMode;
    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::setPolygonOutlineMode(bool isOutlineMode)
{
    dataProcessorPtr_->clearProcessing2(false);
    if(isOutlineMode) {
        setOutlineCompleted(false);
        clear(false);
        polygonOutline_->setOutlineMode(true);
    }
    else {
        setOutlineCompleted(true);
        polygonOutline_->setOutlineMode(false);
        datasetPtr_->resetPolygonOutline();
        polygonOutline_->clearData();
        setCursorShape(Qt::ArrowCursor);
    }
}

void GraphicsScene3dView::setDataset(Dataset *dataset)
{
    if (!dataset) {
        return;
    }

    if(datasetPtr_) {
        QObject::disconnect(datasetPtr_);
    }

    datasetPtr_ = dataset;

    boatTrack_->setDatasetPtr(datasetPtr_);
    m_bottomTrack->setDatasetPtr(datasetPtr_);
    polygonOutline_->setDatasetPtr(datasetPtr_);

    forceUpdateDatasetLlaRef();

    QObject::connect(datasetPtr_, &Dataset::bottomTrackUpdated,
        this, [this](const ChannelId& channelId, int lEpoch, int rEpoch, bool manual, bool redrawAll)->void {
            // qDebug() << "connect&Dataset::bottomTrackUpdated...........";
            //暂时注释
            // auto chList = datasetPtr_->channelsList();
            // if (!datasetPtr_ || chList.empty() || chList.first().channelId_ != channelId) {
            //     return;
            // }
            // clearComboSelectionRect();
            if(datasetPtr_->polygonNEDEmpty() && qmlRootObject_) {
                if(auto isobathsSet = qmlRootObject_->findChild<QObject*>("isobathsSet")) {
                    isobathsSet->setProperty("outlineMode", true);
                }
            }

            m_bottomTrack->isEpochsChanged(lEpoch, rEpoch, manual, redrawAll); //最终触发了绘制等高线
    }, Qt::DirectConnection);

    QObject::connect(datasetPtr_, &Dataset::updatedLlaRef, this,   [this]() -> void {
            surfaceView_->setLlaRef(datasetPtr_->getLlaRef());
            forceUpdateDatasetLlaRef();
            fitAllInView();
    }, Qt::DirectConnection);

    QObject::connect(datasetPtr_, &Dataset::locationToDest, this, [this](LLA targetLla) ->void {
        // 将经纬度转换为相对于当前参考点的NED坐标
        // LLARef llaRef = m_camera->datasetLlaRef_;
        // m_camera->datasetLlaRef_ = LLARef(targetLla);
        // North_East_Down targetNed(&targetLla, &m_camera->datasetLlaRef_, false);
        // 移动相机焦点到目标位置
        // m_camera->focusOnPosition(QVector3D(targetNed.n, targetNed.e, 0.0f));
        forceUpdateDatasetLlaRef();
        m_camera->setDistance(1600.0f);

        emit cameraIsMoved();
        QQuickFramebufferObject::update();
    });
}

void GraphicsScene3dView::setDataProcessorPtr(DataProcessor *dataProcessorPtr)
{
    dataProcessorPtr_ = dataProcessorPtr;

    m_bottomTrack->setDataProcessorPtr(dataProcessorPtr_);
}

void GraphicsScene3dView::addPoints(QVector<QVector3D> positions, QColor color, float width) {
    for(int i = 0; i < positions.size(); i++) {
        auto p = std::make_shared<PointObject>();
        p->setPosition(positions[i]);
        p->setColor(color);
        p->setWidth(width);
        pointGroup()->append(p);
    }
}

void GraphicsScene3dView::setQmlRootObject(QObject* object)
{
    qmlRootObject_ = object;
    polygonOutline_->setQmlRootObject(object);
}

void GraphicsScene3dView::updateBounds()
{
    // qDebug() << "GraphicsScene3dView::updateBounds........";
    m_bounds = boatTrack_->bounds()
                   .merge(isobathsView_->bounds())
                   .merge(m_bottomTrack->bounds())
                   .merge(polygonOutline_->bounds())
                   .merge(m_polygonGroup->bounds())
                   .merge(surfaceView_->bounds())
                   .merge(imageView_->bounds());

    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::updatePlaneGrid()
{
    m_planeGrid->setPlane(m_bounds.bottom());
    m_planeGrid->setCellSize(10);
}

// void GraphicsScene3dView::clearComboSelectionRect()
// {
    // m_comboSelectionRect = { 0, 0, 0, 0 };
// }

void GraphicsScene3dView::calculateLatLong(qreal x, qreal y, double& latitude, double& longitude)
{
    // 1. 用完整矩阵 unproject（必须乘 model）
    QVector3D rayOrigin = QVector3D(x, height() - y, -1.0f) .unproject(m_camera->m_view * m_model,
                                    m_projection,boundingRect().toRect());

    QVector3D rayEnd = QVector3D(x, height() - y, 1.0f) .unproject(m_camera->m_view * m_model,
                                    m_projection, boundingRect().toRect());

    QVector3D rayDir = (rayEnd - rayOrigin).normalized();

    // 2. 地面高度必须正确
    float groundZ = 0.0f;
    QVector3D hitPoint = calculateIntersectionPoint(rayOrigin, rayDir, groundZ);
    if (hitPoint == QVector3D())  return;

    // 3. hitPoint 本身就是 North_East_Down 坐标（不要减相机）
    North_East_Down ned;
    ned.n = hitPoint.x();
    ned.e = hitPoint.y();
    ned.d = 0.0;

    // 4. 转换成经纬度
    LLA lla(&ned, &m_camera->viewLlaRef_, m_camera->getIsPerspective());
    mapView_->setViewLlaRef(m_camera->viewLlaRef_);

    latitude  = lla.latitude;
    longitude = lla.longitude;

    // qDebug() << "mouseTrigger x:" << x << "   y:" << y << "   lati:" << lla.latitude << "   long:" << lla.longitude;
}

QVector3D GraphicsScene3dView::calculateToWorldCoor(qreal x, qreal y)
{
    QVector3D rayOrigin = QVector3D(x, height() - y, -1.0f) .unproject(m_camera->m_view * m_model,
                                                                m_projection,boundingRect().toRect());
    QVector3D rayEnd = QVector3D(x, height() - y, 1.0f) .unproject(m_camera->m_view * m_model,
                                                                m_projection, boundingRect().toRect());
    QVector3D rayDir = (rayEnd - rayOrigin).normalized();

    float groundZ = 0.0f;
    return calculateIntersectionPoint(rayOrigin, rayDir, groundZ);
}

void GraphicsScene3dView::updateDistance()
{
    QVector3D origin = calculateToWorldCoor(16, 16);
    QVector3D end    = calculateToWorldCoor(16 + screetShot_.rulerBar_, 16);
    const float dx   = (end.x() - origin.x());
    const float dy   = (end.y() - origin.y());
    double dist = std::sqrt(dx * dx + dy * dy);
    emit screetShot_.signalSreenBoxDist(dist);

    /*------- 测距模块 -------*/
    if(screetShot_.isDistMeasureMode_) {
        QPointF p1 = screetShot_.getDistLineP1();
        QPointF p2 = screetShot_.getDistLineP2();
        QVector3D worldCoorOrigin = calculateToWorldCoor(p1.x(), p1.y());
        QVector3D worldCoorEnd    = calculateToWorldCoor(p2.x(), p2.y());
        const float dx = (worldCoorEnd.x() - worldCoorOrigin.x());
        const float dy = (worldCoorEnd.y() - worldCoorOrigin.y());
        double dist = std::sqrt(dx * dx + dy * dy);
        emit screetShot_.signalStartToEndDist(dist);
    }
}


void GraphicsScene3dView::updateMapView()
{
    if (!m_camera || !mapView_) {
        return;
    }

    if (!mapView_->isVisible()) {
        return;
    }

    float reductorFactor = -0.05f;
    QVector<QPair<float, float>> cornerMultipliers = {
        {       reductorFactor,         reductorFactor },   // lt
        {       reductorFactor,  1.0f - reductorFactor },   // lb
        {1.0f - reductorFactor,  1.0f - reductorFactor },   // rb
        {1.0f - reductorFactor,         reductorFactor }    // rt
    };

    updateProjection();

    // calc ned
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();
    bool allPointsAreValid = true;
    for (const auto& multiplier : cornerMultipliers) {
        float currWidth  = width() * multiplier.first;
        float currHeight = height() * multiplier.second;

        QVector3D point;
        if (m_camera->getIsPerspective()) {
            auto toOrigin = QVector3D(currWidth, currHeight, -1.0f).unproject(m_camera->m_view * m_model, m_projection, boundingRect().toRect());
            auto toEnd = QVector3D(currWidth, currHeight,  1.0f).unproject(m_camera->m_view * m_model, m_projection, boundingRect().toRect());
            auto toDist = (toEnd - toOrigin).normalized();
            point = calculateIntersectionPoint(toOrigin, toDist, 0);
        }
        else {
            point = QVector3D(currWidth, currHeight, 0.0f).unproject(m_camera->m_view * m_model, m_projection, boundingRect().toRect());
        }

        if (point == QVector3D()) {
            allPointsAreValid = false;
            break;
        }

        minX = std::min(minX, point.x());
        minY = std::min(minY, point.y());
        maxX = std::max(maxX, point.x());
        maxY = std::max(maxY, point.y());
    }

    if (allPointsAreValid) {
        bool canRequest{ true };
        if (m_camera->getAngleToGround() > 5.0f) {
            const float maxSideSize = 14000.f;
            float maxS = std::pow(maxSideSize, 2.0f);
            float rectArea = std::fabs(maxX - minX) * std::fabs(maxY - minY);
            if (rectArea > maxS) { // TODO: using Z coeff
                canRequest = false;
            }
        }

        QVector<LLA> llaVerts;

        float dist = m_camera->distForMapView();
        lastCameraDist_ = dist;

        North_East_Down ltNed(minX, minY, 0.0);
        North_East_Down lbNed(minX, maxY, 0.0);
        North_East_Down rbNed(maxX, maxY, 0.0);
        North_East_Down rtNed(maxX, minY, 0.0);
        LLA ltLla(&ltNed, &m_camera->viewLlaRef_, m_camera->getIsPerspective());
        LLA lbLla(&lbNed, &m_camera->viewLlaRef_, m_camera->getIsPerspective());
        LLA rbLla(&rbNed, &m_camera->viewLlaRef_, m_camera->getIsPerspective());
        LLA rtLla(&rtNed, &m_camera->viewLlaRef_, m_camera->getIsPerspective());

        ltLla.latitude = map::clampLatitude(ltLla.latitude);
        lbLla.latitude = map::clampLatitude(lbLla.latitude);
        rbLla.latitude = map::clampLatitude(rbLla.latitude);
        rtLla.latitude = map::clampLatitude(rtLla.latitude);

        double edge = 180.0;
        if (ltLla.longitude >  edge) ltLla.longitude =  edge;
        if (ltLla.longitude < -edge) ltLla.longitude = -edge;
        if (lbLla.longitude >  edge) lbLla.longitude =  edge;
        if (lbLla.longitude < -edge) lbLla.longitude = -edge;
        if (rbLla.longitude >  edge) rbLla.longitude =  edge;
        if (rbLla.longitude < -edge) rbLla.longitude = -edge;
        if (rtLla.longitude >  edge) rtLla.longitude =  edge;
        if (rtLla.longitude < -edge) rtLla.longitude = -edge;

        llaVerts.append(LLA(ltLla.latitude, ltLla.longitude, dist));
        llaVerts.append(LLA(lbLla.latitude, lbLla.longitude, dist));
        llaVerts.append(LLA(rbLla.latitude, rbLla.longitude, dist));
        llaVerts.append(LLA(rtLla.latitude, rtLla.longitude, dist));

        if (canRequest) {
            emit sendRectRequest(llaVerts, m_camera->getIsPerspective(), m_camera->viewLlaRef_, false);
        }
        else {
            emit sendLlaRef(m_camera->viewLlaRef_);
        }
    } // is rect
    else {
        emit sendLlaRef(m_camera->viewLlaRef_);
    }

    currentLat_ = m_camera->viewLlaRef_.refLla.latitude;
    currentLon_ = m_camera->viewLlaRef_.refLla.longitude;
    emit currentLatChanged();
    emit currentLonChanged();
    mapView_->setViewLlaRef(m_camera->viewLlaRef_);

    QQuickFramebufferObject::update();

    updateDistance();
}

void GraphicsScene3dView::updateViews()
{
    if (isobathsView_) {
        isobathsView_->setCameraDistToFocusPoint(m_camera->distForMapView());
    }
}


void GraphicsScene3dView::onPositionAdded(uint64_t indx)
{
    qDebug() << "GraphicsScene3dView::onPositionAdded........";
    if (!datasetPtr_) {
        return;
    }
    auto* epPtr = datasetPtr_->fromIndex(indx);
    if (!epPtr) {
        return;
    }

    const Position boatPos = epPtr->getPositionGNSS();
    if (!boatPos.ned.isCoordinatesValid()) {
        return;
    }
    boatTrack_->onPositionAdded(indx);

    navigationArrow_->setPositionAndAngle(QVector3D(boatPos.ned.n, boatPos.ned.e,
                            !isfinite(boatPos.ned.d) ? 0.f : boatPos.ned.d), -90.f);
}

void GraphicsScene3dView::setIsNorth(bool state)
{
    if (isNorth_ == state) {
        return;
    }

    isNorth_ = state;

    if (isNorth_ && m_camera && m_camera->getIsPerspective()) {
        m_camera->resetRotationAngle();
        if (m_axesThumbnailCamera) {
            m_axesThumbnailCamera->resetRotationAngle();
        }

        m_camera->resetZAxis();
        updateProjection();
    }

    QQuickFramebufferObject::update();
    emit cameraIsMoved();
}


void GraphicsScene3dView::slotScreetGraphics()
{
    double minLat = std::min({screetShot_.topLeftLati_, screetShot_.bottomRightLati_});
    double maxLat = std::max({screetShot_.topLeftLati_, screetShot_.bottomRightLati_});
    double minLon = std::min({screetShot_.topLeftLong_, screetShot_.bottomRightLong_});
    double maxLon = std::max({screetShot_.topLeftLong_, screetShot_.bottomRightLong_});

    mapLevel_ = screetShot_.getTargetMapLevel();

    QDir dir;
    QString folderPath = screetShot_.getTargetDirPath();
    if (!dir.exists(folderPath)) {
        dir.mkpath(folderPath);
    }

    LLA topLeftLla(maxLat, minLon, 0.0);
    LLA bottomRightLla(minLat, maxLon, 0.0);
    North_East_Down topLeftNed(&topLeftLla, &m_camera->viewLlaRef_, m_camera->getIsPerspective());
    North_East_Down bottomRightNed(&bottomRightLla, &m_camera->viewLlaRef_, m_camera->getIsPerspective());

    // 在 NED 坐标系中计算宽度和高度（使用平面距离）
    double geoWidth  = std::abs(bottomRightNed.e - topLeftNed.e);
    double geoHeight = std::abs(bottomRightNed.n - topLeftNed.n);
    qDebug() << "NED width:..." << geoWidth << "m, height:..." << geoHeight << "m";
    screenshotTask_ = ScreenshotTask(mapLevel_, minLat, maxLat, minLon, maxLon, geoWidth, geoHeight, folderPath);
    originCameralookAt_ = m_camera->m_lookAt;
    originCameraDist_   = m_camera->distForMapView_;

    double centerLat = (minLat + maxLat) * 0.5;
    double centerLon = (minLon + maxLon) * 0.5;
    // 不修改 viewLlaRef_，只移动 lookAt
    LLA targetCenterLla(centerLat, centerLon, 0.0);
    North_East_Down targetCenterNed(&targetCenterLla, &m_camera->viewLlaRef_, m_camera->getIsPerspective());
    m_camera->m_lookAt = QVector3D(targetCenterNed.n, targetCenterNed.e, 0.0f);

    // double targetHeight = std::max(geoWidth, geoHeight) * 0.5;
    // 计算对应目标地图级别的高度值
    double targetHeight = TILE_CONSTANT / std::pow(2.0, 21);
    m_camera->m_distToFocusPoint = static_cast<float>(targetHeight);
    m_camera->distForMapView_    = static_cast<float>(targetHeight);
    m_camera->distToGround_      = static_cast<float>(targetHeight);
    m_camera->updateViewMatrix();

    targetHeight = TILE_CONSTANT / std::pow(2.0, mapLevel_);
    QVector<LLA> request;
    request.append(LLA(maxLat, minLon, targetHeight));
    request.append(LLA(maxLat, maxLon, targetHeight));
    request.append(LLA(minLat, maxLon, targetHeight));
    request.append(LLA(minLat, minLon, targetHeight));
    emit sendRectRequest(request, m_camera->getIsPerspective(), m_camera->viewLlaRef_, true);
    GIF->dialogInfo(Dialog_Loading, "show");
}

void GraphicsScene3dView::onTargetTilesLoaded()
{
    qDebug() << "onTargetTilesLoaded............";
    screenshotPending_ = true;
    QQuickFramebufferObject::update();
}

void GraphicsScene3dView::setProgressDialog(QObject* dialog)
{
    if (progressDialog_ != dialog) {
        progressDialog_ = dialog;
    }
}


/*-----------Renderer--------------------------------------*/
GraphicsScene3dView::InFboRenderer::InFboRenderer() :
    QQuickFramebufferObject::Renderer(), m_renderer(new GraphicsScene3dRenderer)
{
    m_renderer->initialize();
}

GraphicsScene3dView::InFboRenderer::~InFboRenderer()
{}

void GraphicsScene3dView::InFboRenderer::setupCameraForTask(const ScreenshotTask& task, double geoWidth, double geoHeight)
{
    // 设置新相机位置
    double centerLon = (task.minLon + task.maxLon) / 2.0;
    double centerLat = (task.minLat + task.maxLat) / 2.0;
    LLA centerLla(centerLat, centerLon, 0.0);
    North_East_Down targetCenterNed(&centerLla, &m_renderer->m_camera.viewLlaRef_, false);
    m_renderer->m_camera.m_lookAt = QVector3D(targetCenterNed.n, targetCenterNed.e, 0.0f);
    m_renderer->m_camera.isPerspective_ = false;

    // 直接设置正交投影边界，确保目标区域正好填满视口（以目标区域中心为原点）
    float orthoLeft   = static_cast<float>(-geoWidth  / 2.0);
    float orthoRight  = static_cast<float>( geoWidth  / 2.0);
    float orthoBottom = static_cast<float>(-geoHeight / 2.0);
    float orthoTop    = static_cast<float>( geoHeight / 2.0);

    m_renderer->setCustomOrtho(orthoLeft, orthoRight, orthoBottom, orthoTop);
}


bool GraphicsScene3dView::InFboRenderer::renderToOffscreen(const ScreenshotTask& task)
{
    qDebug() << "============ renderAndSaveTiles START ================";
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    QOpenGLFunctions* func = ctx ? ctx->functions() : nullptr;
    if (!ctx || !func) {
        qCritical() << "[FAIL] No OpenGL context!";
        return false;
    }

    // float metersPerPixel = TILE_CONSTANT / std::pow(2.0, task.mapLevel) / 256.0;
    float metersPerPixel = TILE_CONSTANT / std::pow(2.0, 21) / 256.0;
    int pixelWidth  = static_cast<int>(task.geoWidth / metersPerPixel);
    int pixelHeight = static_cast<int>(task.geoHeight / metersPerPixel);

    // 划分300m × 300m的小正方形
    constexpr double CHUNK_SIZE_METERS = 300.0;
    int rows = static_cast<int>(std::ceil(task.geoWidth  / CHUNK_SIZE_METERS));  // 经度方向（东西）
    int cols = static_cast<int>(std::ceil(task.geoHeight / CHUNK_SIZE_METERS));  // 纬度方向（南北）
    int chunkPixelWidth  = static_cast<int>(CHUNK_SIZE_METERS / metersPerPixel);
    int chunkPixelHeight = static_cast<int>(CHUNK_SIZE_METERS / metersPerPixel);
    qDebug() << "Geo area:" << task.geoWidth << "m x" << task.geoHeight << "m";
    qDebug() << "Chunk pixels:" << chunkPixelWidth << "x" << chunkPixelHeight;
    qDebug() << "Pixel dimensions:" << pixelWidth << "x" << pixelHeight;

    if (!offScreenFbo_) {
        offScreenFbo_ = createFramebufferObject(QSize(pixelWidth, pixelHeight));
    }

    // 保存当前状态
    GLint prevFbo;
    func->glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
    GLint prevViewport[4];
    func->glGetIntegerv(GL_VIEWPORT, prevViewport);

    offScreenFbo_->bind();
    func->glViewport(0, 0, pixelWidth, pixelHeight);
    func->glEnable(GL_DEPTH_TEST);
    func->glDepthFunc(GL_LEQUAL);
    func->glEnable(GL_BLEND);
    func->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    func->glDisable(GL_MULTISAMPLE);
    func->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    func->glClearStencil(0);
    func->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    setupCameraForTask(task, task.geoWidth, task.geoHeight);

    m_renderer->m_viewSize = QSizeF(pixelWidth, pixelHeight);
    m_renderer->render();

    func->glFinish();
    GLenum status = func->glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        GIF->dialogInfo(Dialog_OK, tr("Loading failed, please try again."));
        return false;
    }

    GIF->dialogInfo(Dialog_Loading, "hide");

#ifdef Q_OS_WIN
    QImage fullResult = QImage(pixelWidth, pixelHeight, QImage::Format_RGB32);
    func->glReadPixels(0, 0, pixelWidth, pixelHeight, GL_BGRA, GL_UNSIGNED_BYTE, fullResult.bits());
    fullResult = fullResult.mirrored(false, true);

    int chunkIndex = 0;
    int kmzCnt = rows * cols;
    QMetaObject::invokeMethod(graphicsView_->progressDialog_, "open");
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            // 左上角为原点，x向右，y向下
            int pixelX = row * chunkPixelWidth;
            int pixelY = col * chunkPixelHeight;

            int actualChunkWidth = std::min(chunkPixelWidth, pixelWidth - pixelX);
            int actualChunkHeight = std::min(chunkPixelHeight, pixelHeight - pixelY);
            if (actualChunkWidth <= 0 || actualChunkHeight <= 0) {
                continue;
            }

            // 根据像素位置计算在原区域中的比例
            double lonRatioLeft   = static_cast<double>(pixelX) / pixelWidth;
            double lonRatioRight  = static_cast<double>(pixelX + actualChunkWidth) / pixelWidth;
            double latRatioTop    = static_cast<double>(pixelY) / pixelHeight;
            double latRatioBottom = static_cast<double>(pixelY + actualChunkHeight) / pixelHeight;

            // 计算四个顶点的经纬度
            double westLon = task.minLon + lonRatioLeft * (task.maxLon - task.minLon);
            double eastLon = task.minLon + lonRatioRight * (task.maxLon - task.minLon);
            // 注意纬度方向：纬度增加向北，但像素Y增加向南
            double northLat = task.maxLat - latRatioTop * (task.maxLat - task.minLat);
            double southLat = task.maxLat - latRatioBottom * (task.maxLat - task.minLat);

            QImage chunkImage = fullResult.copy(pixelX, pixelY, actualChunkWidth, actualChunkHeight);

            QString rowStr = QString::number(row + 1);
            QString colStr = QString::number(col + 1);
            QString chunkBasePath = task.outputPath + "/" + rowStr + "_" + colStr;
            if (!chunkImage.save(chunkBasePath + ".png", "PNG")) {
                qDebug() << "Tiles saved failed to:" << chunkImage;
            }

            QString chunkImageName = rowStr + "_" + colStr + ".png";
            ScreetShot::createKmlFile(chunkBasePath + ".kml", chunkImageName, northLat, southLat, eastLon, westLon);
            ScreetShot::createXMAPFile(chunkBasePath + ".kml", chunkBasePath + ".png", chunkBasePath);

            chunkIndex++;
            double progress = static_cast<double>(chunkIndex) / kmzCnt;
            QString statusText = tr("Processing CSV %1 / %2 (%3%)").arg(chunkIndex)
                                     .arg(kmzCnt).arg(static_cast<int>(progress * 100));
            if (graphicsView_->progressDialog_) {
                QMetaObject::invokeMethod(graphicsView_->progressDialog_, "setProgress", Q_ARG(QVariant, progress));
                QMetaObject::invokeMethod(graphicsView_->progressDialog_, "setStatus",   Q_ARG(QVariant, statusText));
            }

            QCoreApplication::processEvents();
        }
    }

    ScreetShot::writeBoundaryFile(task.outputPath,task.maxLat,task.minLat,task.maxLon,task.minLon,task.mapLevel);
    ScreetShot::menu_renewMap(task.outputPath);

#elif defined(Q_OS_ANDROID)
    QImage fullResult(pixelWidth, pixelHeight, QImage::Format_RGBA8888);
    func->glReadPixels(0, 0, pixelWidth, pixelHeight, GL_RGBA, GL_UNSIGNED_BYTE, fullResult.bits());
    fullResult = fullResult.mirrored(false, true);

    int chunkIndex = 0;
    int kmzCnt = rows * cols;
    QMetaObject::invokeMethod(graphicsView_->progressDialog_, "open");
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            // 左上角为原点，x向右，y向下
            int pixelX = row * chunkPixelWidth;
            int pixelY = col * chunkPixelHeight;

            int actualChunkWidth = std::min(chunkPixelWidth, pixelWidth - pixelX);
            int actualChunkHeight = std::min(chunkPixelHeight, pixelHeight - pixelY);
            if (actualChunkWidth <= 0 || actualChunkHeight <= 0) {
                continue;
            }

            // 根据像素位置计算在原区域中的比例
            double lonRatioLeft   = static_cast<double>(pixelX) / pixelWidth;
            double lonRatioRight  = static_cast<double>(pixelX + actualChunkWidth) / pixelWidth;
            double latRatioTop    = static_cast<double>(pixelY) / pixelHeight;
            double latRatioBottom = static_cast<double>(pixelY + actualChunkHeight) / pixelHeight;

            // 计算四个顶点的经纬度
            double westLon = task.minLon + lonRatioLeft * (task.maxLon - task.minLon);
            double eastLon = task.minLon + lonRatioRight * (task.maxLon - task.minLon);
            // 注意纬度方向：纬度增加向北，但像素Y增加向南
            double northLat = task.maxLat - latRatioTop * (task.maxLat - task.minLat);
            double southLat = task.maxLat - latRatioBottom * (task.maxLat - task.minLat);

            QImage chunkImage = fullResult.copy(pixelX, pixelY, actualChunkWidth, actualChunkHeight);

            QString rowStr = QString::number(row + 1);
            QString colStr = QString::number(col + 1);
            QString chunkBasePath  = rowStr + "_" + colStr;
            QString chunkImageName = rowStr + "_" + colStr + ".png";
            if (!chunkImage.save(chunkBasePath + ".png", "PNG")) {
                qDebug() << "Tiles saved failed to:" << chunkImage;
            }

            ScreetShot::createKmlFile(chunkBasePath + ".kml", chunkImageName, northLat, southLat, eastLon, westLon);
            ScreetShot::createXMAPFile(chunkBasePath + ".kml", chunkBasePath + ".png", chunkBasePath);
            QFile xmapFile(chunkBasePath + ".kmz");
            if (xmapFile.open(QIODevice::ReadOnly)) {
                QByteArray fileData = xmapFile.readAll();
                xmapFile.close();

                //把 QByteArray 直接转成 jbyteArray   nie:test测试中，暂时保存为.kmz
                QAndroidJniEnvironment env;
                jbyteArray byteArray = env->NewByteArray(fileData.size());
                env->SetByteArrayRegion(byteArray, 0, fileData.size(), reinterpret_cast<const jbyte*>(fileData.constData()));
                QAndroidJniObject fileName2 = QAndroidJniObject::fromString(chunkBasePath + ".kmz");
                QAndroidJniObject mimeType = QAndroidJniObject::fromString("application/vnd.google-earth.kmz+xml");

                //调用 Java 端方法
                QAndroidJniObject::callStaticMethod<void>( "com/nqc/FileQtActivity", "saveBinaryFile",
                     "(Ljava/lang/String;[BLjava/lang/String;)V",fileName2.object<jstring>(),byteArray,mimeType.object<jstring>());
                env->DeleteLocalRef(byteArray);
            }

            chunkIndex++;
            double progress = static_cast<double>(chunkIndex) / kmzCnt;
            QString statusText = tr("Processing CSV %1 / %2 (%3%)").arg(chunkIndex)
                                     .arg(kmzCnt).arg(static_cast<int>(progress * 100));
            if (graphicsView_->progressDialog_) {
                QMetaObject::invokeMethod(graphicsView_->progressDialog_, "setProgress", Q_ARG(QVariant, progress));
                QMetaObject::invokeMethod(graphicsView_->progressDialog_, "setStatus",   Q_ARG(QVariant, statusText));
            }

            QCoreApplication::processEvents();
        }
    }

    ScreetShot::writeBoundaryFile(".",task.maxLat,task.minLat,task.maxLon,task.minLon,task.mapLevel);
    QFile boundaryFile("boundary.xrmap");
    if (boundaryFile.open(QIODevice::ReadOnly)) {
        QByteArray fileData = boundaryFile.readAll();
        boundaryFile.close();
        QAndroidJniEnvironment env;
        jbyteArray byteArray = env->NewByteArray(fileData.size());
        env->SetByteArrayRegion(byteArray, 0, fileData.size(), reinterpret_cast<const jbyte*>(fileData.constData()));
        QAndroidJniObject fileName2 = QAndroidJniObject::fromString("boundary.xrmap");
        QAndroidJniObject mimeType = QAndroidJniObject::fromString("application/vnd.google-earth.xrmap+xml");
        QAndroidJniObject::callStaticMethod<void>( "com/nqc/FileQtActivity", "saveBinaryFile",
            "(Ljava/lang/String;[BLjava/lang/String;)V",fileName2.object<jstring>(),byteArray,mimeType.object<jstring>());
        env->DeleteLocalRef(byteArray);
    }
    ScreetShot::menu_renewMap(".");
    QFile maplistFile("maplist.xrmap");
    if(maplistFile.open(QIODevice::ReadOnly)) {
        QByteArray fileData = maplistFile.readAll();
        maplistFile.close();
        QAndroidJniEnvironment env;
        jbyteArray byteArray = env->NewByteArray(fileData.size());
        env->SetByteArrayRegion(byteArray,0,fileData.size(),reinterpret_cast<const jbyte*>(fileData.constData()));
        QAndroidJniObject fileName2 = QAndroidJniObject::fromString("maplist.xrmap");
        QAndroidJniObject mimeType = QAndroidJniObject::fromString("application/vnd.google-earth.xrmap+xml");
        QAndroidJniObject::callStaticMethod<void>("com/nqc/FileQtActivity","saveBinaryFile",
            "(Ljava/lang/String;[BLjava/lang/String;)V",fileName2.object<jstring>(),byteArray,mimeType.object<jstring>());
        env->DeleteLocalRef(byteArray);
    }
#endif
    if (graphicsView_->progressDialog_) {
        QMetaObject::invokeMethod(graphicsView_->progressDialog_, "setProgress", Q_ARG(QVariant, 1.0));
        QMetaObject::invokeMethod(graphicsView_->progressDialog_, "setStatus", Q_ARG(QVariant, tr("Processing completed!")));
    }

    // 恢复状态
    offScreenFbo_->release();
    if(offScreenFbo_){
        delete offScreenFbo_;
        offScreenFbo_ = nullptr;
    }
    func->glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);
    func->glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
    m_renderer->clearCustomOrtho();
    graphicsView_->screenshotPending_ = false;
    graphicsView_->m_camera->m_lookAt = graphicsView_->originCameralookAt_;
    graphicsView_->m_camera->m_distToFocusPoint = graphicsView_->originCameraDist_;
    graphicsView_->m_camera->distForMapView_    = graphicsView_->originCameraDist_;
    graphicsView_->m_camera->distToGround_      = graphicsView_->originCameraDist_;
    graphicsView_->m_camera->updateViewMatrix();
    graphicsView_->updateMapView();

    qDebug() << "========= renderAndSaveTiles END =================";
    return true;
}


void GraphicsScene3dView::InFboRenderer::synchronize(QQuickFramebufferObject* fbo)
{
    //仅在 synchronize()中，将 Item 的属性复制到 Renderer 的成员变量中
    GraphicsScene3dView* graphicsView = qobject_cast<GraphicsScene3dView*>(fbo);  //线程安全：GUI线程在此处被阻塞
    if (!graphicsView) {
        return;
    }

    graphicsView_ = graphicsView;

    // process textures
    processMapTextures(graphicsView);
    processMosaicColorTableTexture(graphicsView);
    processMosaicTileTexture(graphicsView);
    processImageTexture(graphicsView);
    processSurfaceTexture(graphicsView);

    // read from renderer
    graphicsView->m_model      = m_renderer->m_model;
    graphicsView->m_projection = m_renderer->m_projection;

    // write to renderer
    m_renderer->m_coordAxesRenderImpl       = *(dynamic_cast<CoordinateAxes::CoordinateAxesRenderImplementation*>(graphicsView->m_coordAxes->m_renderImpl));
    m_renderer->m_planeGridRenderImpl       = *(dynamic_cast<PlaneGrid::PlaneGridRenderImplementation*>(graphicsView->m_planeGrid->m_renderImpl));
    m_renderer->m_boatTrackRenderImpl       = *(dynamic_cast<BoatTrack::BoatTrackRenderImplementation*>(graphicsView->boatTrack_->m_renderImpl));
    m_renderer->m_bottomTrackRenderImpl     = *(dynamic_cast<BottomTrack::BottomTrackRenderImplementation*>(graphicsView->m_bottomTrack->m_renderImpl));
    m_renderer->m_polygonOutlineRenderImpl  = *(dynamic_cast<PolygonOutline::PolygonOutlineRenderImplementation*>(graphicsView->polygonOutline_->m_renderImpl));
    m_renderer->isobathsViewRenderImpl_     = *(dynamic_cast<IsobathsView::IsobathsViewRenderImplementation*>(graphicsView->isobathsView_->m_renderImpl));
    m_renderer->surfaceViewRenderImpl_      = *(dynamic_cast<SurfaceView::SurfaceViewRenderImplementation*>(graphicsView->surfaceView_->m_renderImpl));
    m_renderer->imageViewRenderImpl_        = *(dynamic_cast<ImageView::ImageViewRenderImplementation*>(graphicsView->imageView_->m_renderImpl));
    m_renderer->m_polygonGroupRenderImpl    = *(dynamic_cast<PolygonGroup::PolygonGroupRenderImplementation*>(graphicsView->m_polygonGroup->m_renderImpl));
    m_renderer->navigationArrowRenderImpl_  = *(dynamic_cast<NavigationArrow::NavigationArrowRenderImplementation*>(graphicsView->navigationArrow_->m_renderImpl));
    m_renderer->m_viewSize                  = graphicsView->size();
    m_renderer->m_camera                    = *graphicsView->m_camera;
    m_renderer->m_axesThumbnailCamera       = *graphicsView->m_axesThumbnailCamera;
    // m_renderer->m_comboSelectionRect        = graphicsView->m_comboSelectionRect;
    m_renderer->m_verticalScale             = graphicsView->m_verticalScale;
    m_renderer->m_boundingBox               = graphicsView->m_bounds;
    m_renderer->m_isSceneBoundingBoxVisible = graphicsView->m_isSceneBoundingBoxVisible;
    m_renderer->gridVisibility_             = graphicsView->gridVisibility_;

    //随后触发void GraphicsScene3dView::InFboRenderer::render()................
}

void GraphicsScene3dView::InFboRenderer::render()
{
    m_renderer->render();

    if (graphicsView_->screenshotPending_)
    {
        renderToOffscreen(graphicsView_->screenshotTask_);
    }
}


QOpenGLFramebufferObject *GraphicsScene3dView::InFboRenderer::createFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

#if defined(Q_OS_ANDROID) || defined(LINUX_ES)
    format.setSamples(0);
#else
    // format.setSamples(4);
    format.setSamples(0);
#endif

    QSize scaledSize = size;
    scaledSize.setWidth(std::max(1, scaledSize.width()));
    scaledSize.setHeight(std::max(1, scaledSize.height()));

    return new QOpenGLFramebufferObject(scaledSize, format);
}

void GraphicsScene3dView::InFboRenderer::processMapTextures(GraphicsScene3dView *viewPtr) const
{
    auto& r = m_renderer->mapViewRenderImpl_;

    auto* src = dynamic_cast<MapView::MapViewRenderImplementation*>(viewPtr->mapView_->m_renderImpl);
    r.copyCpuSideFrom(*src);

    auto init = viewPtr->mapView_->takeInitTileTasks();
    r.pendingInit_.reserve(r.pendingInit_.size() + init.size());
    for (auto& itm : init) {
        r.pendingInit_.push_back({ itm.first, std::move(itm.second) });
    }
    auto upd = viewPtr->mapView_->takeUpdateTileTasks();
    r.pendingUpdate_.reserve(r.pendingUpdate_.size() + upd.size());
    for (auto& itm : upd) {
        r.pendingUpdate_.push_back({ itm.first, std::move(itm.second) });
    }
    auto del = viewPtr->mapView_->takeDeleteTileTasks();
    r.pendingDelete_ += del;
}

void GraphicsScene3dView::InFboRenderer::processMosaicColorTableTexture(GraphicsScene3dView* viewPtr) const
{
    auto surfacePtr = viewPtr->getSurfaceViewPtr();

    if (auto cTTDId = surfacePtr->takeMosaicColorTableToDelete(); cTTDId) {
        surfacePtr->setMosaicColorTableTextureId(0);
        glDeleteTextures(1, &cTTDId);
    }

    auto task = surfacePtr->takeMosaicColorTableToAppend();
    if (task.empty()) {
        return;
    }

    GLuint colorTableTextureId = surfacePtr->getMosaicColorTableTextureId();

#if defined(Q_OS_ANDROID) || defined(LINUX_ES)
    if (colorTableTextureId) {
        glBindTexture(GL_TEXTURE_2D, colorTableTextureId);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, task.size() / 4, 1, GL_RGBA, GL_UNSIGNED_BYTE, task.data());
    }
    else {
        glGenTextures(1, &colorTableTextureId);
        glBindTexture(GL_TEXTURE_2D, colorTableTextureId);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, task.size() / 4, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, task.data());

        surfacePtr->setMosaicColorTableTextureId(colorTableTextureId);
    }
#else
    if (colorTableTextureId) {
        glBindTexture(GL_TEXTURE_1D, colorTableTextureId);
        glTexSubImage1D(GL_TEXTURE_1D, 0, 0, task.size() / 4, GL_RGBA, GL_UNSIGNED_BYTE, task.data());
    }
    else {
        glGenTextures(1, &colorTableTextureId);
        glBindTexture(GL_TEXTURE_1D, colorTableTextureId);

        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, task.size() / 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, task.data());

        surfacePtr->setMosaicColorTableTextureId(colorTableTextureId);
    }
#endif
}

void GraphicsScene3dView::InFboRenderer::processMosaicTileTexture(GraphicsScene3dView* viewPtr) const // TODO CHECK
{
    auto surfacePtr = viewPtr->getSurfaceViewPtr();

    // delete
    {
        auto tasks = surfacePtr->takeMosaicTileTextureToDelete();
        for (auto it = tasks.begin(); it != tasks.end(); ++it) {
            if (*it != 0) {
                glDeleteTextures(1, &(*it));
            }
        }
    }

    // append or update
    {
        auto tasks = surfacePtr->takeMosaicTileTextureToAppend();

        for (auto it = tasks.begin(); it != tasks.end(); ++it) {
            const auto& tileId = it->first;
            const auto& data   = it->second;

            if (data.empty()) {
                continue;
            }

            const GLuint existingId = surfacePtr->getMosaicTextureIdByTileId(tileId);

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            if (existingId) { // update
                glBindTexture(GL_TEXTURE_2D, existingId);

                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, defaultTileSidePixelSize, defaultTileSidePixelSize, GL_RED, GL_UNSIGNED_BYTE, data.data());

                QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
                gl->glGenerateMipmap(GL_TEXTURE_2D);
            }
            else { // create
                GLuint texId = 0;
                glGenTextures(1, &texId);
                glBindTexture(GL_TEXTURE_2D, texId);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, defaultTileSidePixelSize, defaultTileSidePixelSize, 0, GL_RED, GL_UNSIGNED_BYTE, data.data());

                QOpenGLFunctions* gl = QOpenGLContext::currentContext()->functions();
                gl->glGenerateMipmap(GL_TEXTURE_2D);

                surfacePtr->setMosaicTextureIdByTileId(tileId, texId);
            }
        }
    }
}

void GraphicsScene3dView::InFboRenderer::processImageTexture(GraphicsScene3dView *viewPtr) const
{
    auto imagePtr = viewPtr->getImageViewPtr();
    auto& task = imagePtr->getTextureTasksRef();

    if (task.isNull())
        return;

    GLuint textureId = viewPtr->getImageViewPtr()->getTextureId();

    if (textureId) {
        glDeleteTextures(1, &textureId);
    }

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, viewPtr->getImageViewPtr()->getUseLinearFilter() ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, viewPtr->getImageViewPtr()->getUseLinearFilter() ? GL_LINEAR : GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    QImage glImage = task.convertToFormat(QImage::Format_RGBA8888).mirrored();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glImage.width(), glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glImage.bits());

    imagePtr->setTextureId(textureId);

    QOpenGLFunctions* glFuncs = QOpenGLContext::currentContext()->functions();
    glFuncs->glGenerateMipmap(GL_TEXTURE_2D);

    task = QImage();
}

void GraphicsScene3dView::InFboRenderer::processSurfaceTexture(GraphicsScene3dView *viewPtr) const
{
    // init / reinit
    auto surfacePtr = viewPtr->getSurfaceViewPtr();
    auto task = surfacePtr->takeSurfaceColorTableToAppend();

    if (task.empty()) {
        return;
    }

    GLuint textureId = surfacePtr->getSurfaceColorTableTextureId();
    if (textureId) {
        glDeleteTextures(1, &textureId);
    }

    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, task.size() / 4, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, task.data());

    surfacePtr->setSurfaceColorTableTextureId(textureId);

    // deleting
    auto textureIdtoDel = surfacePtr->takeSurfaceColorTableToDelete();
    if (textureIdtoDel) {
        glDeleteTextures(1, &textureIdtoDel);
    }
}

GraphicsScene3dView::Camera::Camera(GraphicsScene3dView* viewPtr) : viewPtr_(viewPtr)
{
    setMapView();
}

GraphicsScene3dView::Camera::Camera(qreal pitch, qreal yaw, qreal distToFocusPoint, qreal fov, qreal sensivity)
    :m_pitch(std::move(pitch))
    ,m_yaw(std::move(yaw))
    ,m_fov(std::move(fov))
    ,m_distToFocusPoint(std::move(distToFocusPoint))
    ,distForMapView_(m_distToFocusPoint)
    ,m_sensivity(std::move(sensivity))
{
   setIsometricView();
}

float GraphicsScene3dView::Camera::distForMapView() const
{
    return distForMapView_;
}

qreal GraphicsScene3dView::Camera::fov() const
{
    return m_fov;
}

qreal GraphicsScene3dView::Camera::pitch() const
{
    return m_pitch;
}

qreal GraphicsScene3dView::Camera::yaw() const
{
    return m_yaw;
}

QMatrix4x4 GraphicsScene3dView::Camera::viewMatrix() const
{
    return m_view;
}

void GraphicsScene3dView::Camera::setCameraListener(Camera* cameraListener)
{
    cameraListener_ = cameraListener;
}

void GraphicsScene3dView::Camera::rotate(const QVector2D& lastMouse, const QVector2D& mousePos)
{
    auto r = (lastMouse - mousePos)*0.2;
    r.setX(qDegreesToRadians(r.x()));
    r.setY(qDegreesToRadians(r.y()));

    m_rotAngle += r;

    tryResetRotateAngle();
    checkRotateAngle();
    updateCameraParams();
    updateViewMatrix();
}

void GraphicsScene3dView::Camera::rotate(const QPointF& prevCenter, const QPointF& currCenter, qreal angleDelta, qreal widgetHeight)
{
    const qreal increaseCoeff{ 1.3 };
    const qreal angleDeltaY = (prevCenter - currCenter).y() / widgetHeight * 90.0;

    m_rotAngle.setX(m_rotAngle.x() - qDegreesToRadians(angleDelta));
    m_rotAngle.setY(m_rotAngle.y() + qDegreesToRadians(angleDeltaY * increaseCoeff));

    tryResetRotateAngle();
    checkRotateAngle();
    updateCameraParams();
    updateViewMatrix();
}

void GraphicsScene3dView::Camera::move(const QVector2D &startPos, const QVector2D &endPos)
{
    QVector4D horizontalAxis{ -1.0f, 0.0f, 0.0f, 0.0f };
    QVector4D verticalAxis{ 0.0f, -1.0f, 0.0f, 0.0f };

    m_deltaOffset = ((horizontalAxis * (float)(endPos.x() - startPos.x()) +
                      verticalAxis * (float)(endPos.y() - startPos.y()))).toVector3D();

    m_lookAt = m_lookAtSave + m_deltaOffset;

    updateCameraParams();
    tryToChangeViewLlaRef();
    updateViewMatrix();
}

void GraphicsScene3dView::Camera::resetZAxis()
{
    m_lookAt.setZ(0);

    updateCameraParams();
    updateViewMatrix();
}

void GraphicsScene3dView::Camera::moveZAxis(float z)
{
    float xCamera = -sinf(m_rotAngle.y()) * cosf(-m_rotAngle.x()) * m_distToFocusPoint;
    float yCamera = -sinf(m_rotAngle.y()) * sinf(-m_rotAngle.x()) * m_distToFocusPoint;
    float zCamera = cosf(m_rotAngle.y()) * m_distToFocusPoint;
    float currLookAtHeight = -(m_lookAt.z() + z);
    float currCameraHeight = zCamera + currLookAtHeight;

    if (currCameraHeight > 0) {
        float cathetus = std::sqrt(std::pow(xCamera, 2) + std::pow(yCamera, 2));
        float hypotenuse = std::sqrt(std::pow(cathetus, 2) + std::pow(currCameraHeight, 2));
        distForMapView_ = hypotenuse;
    }
    else {
        distForMapView_ = 0.0f;
    }

    m_lookAt.setZ(m_lookAt.z() + z);

    updateCameraParams();
    updateViewMatrix();
}

void GraphicsScene3dView::Camera::zoom(qreal delta)
{
#if defined(Q_OS_ANDROID) || defined(LINUX_ES)
    const float increaseCoeff{ 0.95f };
    m_distToFocusPoint -= delta * m_distToFocusPoint * increaseCoeff;
    distForMapView_ = m_distToFocusPoint;
#else
    m_distToFocusPoint = delta > 0.f ? m_distToFocusPoint / 1.15f : m_distToFocusPoint * 1.15f;
    distForMapView_ = m_distToFocusPoint;
#endif
    // qDebug() << "distForMapView_........" << distForMapView_;


    const float minFocusDist = 2.0f;
    const float maxFocusDist = 100000.0f * 100.0f;
    if (m_distToFocusPoint < minFocusDist) {
        m_distToFocusPoint = minFocusDist;
        distForMapView_ = m_distToFocusPoint;
    }
    if (m_distToFocusPoint >= maxFocusDist) {
        m_distToFocusPoint = maxFocusDist;
        distForMapView_ = m_distToFocusPoint;
    }

    bool preIsPersp{ false };
    distToGround_ = std::max(0.0f, std::fabs(-cosf(m_rotAngle.y()) * m_distToFocusPoint));
    float perspEdge = viewPtr_ ? viewPtr_->perspectiveEdge_ : 5000.0f;
    preIsPersp = distToGround_ < perspEdge;
    bool projectionChanged    =  isPerspective_ !=  preIsPersp;

    North_East_Down lookAtNed(m_lookAt.x(), m_lookAt.y(), 0.0f);
    LLA lookAtLla(&lookAtNed, &viewLlaRef_, isPerspective_);
    LLARef lookAtLlaRef(lookAtLla);

    float datasetDist = map::calculateDistance(lookAtLlaRef, datasetLlaRef_);
/*    if (isPerspective_ && !projectionChanged && datasetDist < lowDistThreshold_ && getIsFarAwayFromOriginLla()) {
       viewPtr_->setNeedToResetStartPos(true);
       LLA datasetLla(datasetLlaRef_.refLla.latitude, datasetLlaRef_.refLla.longitude, 0.0);
       North_East_Down datasetNed(&datasetLla, &viewLlaRef_, isPerspective_);
       m_lookAt -= QVector3D(datasetNed.n, datasetNed.e, 0.0f);
       viewLlaRef_ = datasetLlaRef_;
    }
    else*/
    if ((!isPerspective_ && projectionChanged && (datasetDist < lowDistThreshold_) && getIsFarAwayFromOriginLla())) { // catching when ortho->persp trans and near place
        if (cameraListener_) {
            cameraListener_->resetRotationAngle();
        }

        viewPtr_->setNeedToResetStartPos(true);
        LLA datasetLla(datasetLlaRef_.refLla.latitude, datasetLlaRef_.refLla.longitude, 0.0);
        North_East_Down datasetNed(&datasetLla, &viewLlaRef_, !isPerspective_);
        m_lookAt -= QVector3D(datasetNed.n, datasetNed.e, 0.0f);
        viewLlaRef_ = datasetLlaRef_;
        m_rotAngle = {0.0f, 0.0f};
    }
    else if ((isPerspective_ && projectionChanged) || (!isPerspective_ && !projectionChanged)) { // 透视投影切换到正交投影
        viewPtr_->setNeedToResetStartPos(true);
        viewLlaRef_ = lookAtLlaRef;
        m_lookAt = QVector3D(0.0f, 0.0f, 0.0f);
        m_rotAngle = {0.0f, 0.0f};
    }

    updateCameraParams();
    updateViewMatrix();
}


void GraphicsScene3dView::Camera::zoomAndroid(qreal delta)
{
    m_distToFocusPoint = delta > 0.f ? m_distToFocusPoint / 1.15f : m_distToFocusPoint * 1.15f;
    distForMapView_ = m_distToFocusPoint;

    const float minFocusDist = 2.0f;
    const float maxFocusDist = 100000.0f * 100.0f;
    if (m_distToFocusPoint < minFocusDist) {
        m_distToFocusPoint = minFocusDist;
        distForMapView_ = m_distToFocusPoint;
    }
    if (m_distToFocusPoint >= maxFocusDist) {
        m_distToFocusPoint = maxFocusDist;
        distForMapView_ = m_distToFocusPoint;
    }

    bool preIsPersp{ false };
    distToGround_ = std::max(0.0f, std::fabs(-cosf(m_rotAngle.y()) * m_distToFocusPoint));
    float perspEdge = viewPtr_ ? viewPtr_->perspectiveEdge_ : 5000.0f;
    preIsPersp = distToGround_ < perspEdge;
    bool projectionChanged    =  isPerspective_ !=  preIsPersp;

    North_East_Down lookAtNed(m_lookAt.x(), m_lookAt.y(), 0.0f);
    LLA lookAtLla(&lookAtNed, &viewLlaRef_, isPerspective_);
    LLARef lookAtLlaRef(lookAtLla);

    float datasetDist = map::calculateDistance(lookAtLlaRef, datasetLlaRef_);

    if (isPerspective_ && !projectionChanged) {
    }
    else if ((!isPerspective_ && projectionChanged && (datasetDist < lowDistThreshold_) && getIsFarAwayFromOriginLla())) { // catching when ortho->persp trans and near place
        if (cameraListener_) {
            cameraListener_->resetRotationAngle();
        }

        viewPtr_->setNeedToResetStartPos(true);
        LLA datasetLla(datasetLlaRef_.refLla.latitude, datasetLlaRef_.refLla.longitude, 0.0);
        North_East_Down datasetNed(&datasetLla, &viewLlaRef_, !isPerspective_);
        m_lookAt -= QVector3D(datasetNed.n, datasetNed.e, 0.0f);
        viewLlaRef_ = datasetLlaRef_;
        m_rotAngle = { 0.0f, 0.0f };
    }
    else if ((isPerspective_ && projectionChanged) || (!isPerspective_ && !projectionChanged)) { // 透视投影切换到正交投影
        viewPtr_->setNeedToResetStartPos(true);
        viewLlaRef_ = lookAtLlaRef;
        m_lookAt = QVector3D(0.0f, 0.0f, 0.0f);
        m_rotAngle = { 0.0f, 0.0f };
    }

    updateCameraParams();
    updateViewMatrix();
}

void GraphicsScene3dView::Camera::commitMovement()
{
    m_lookAt += m_deltaOffset;
    m_deltaOffset = QVector3D();

    updateCameraParams();
    updateViewMatrix();
}

void GraphicsScene3dView::Camera::focusOnObject(std::weak_ptr<SceneObject> object)
{
    Q_UNUSED(object)
}

void GraphicsScene3dView::Camera::focusOnPosition(const QVector3D &point)
{
    m_lookAt = point;

    updateCameraParams();
    updateViewMatrix();
}

void GraphicsScene3dView::Camera::setDistance(qreal distance)
{
    m_distToFocusPoint = distance;
    distForMapView_ = m_distToFocusPoint;

    updateCameraParams();
    updateViewMatrix();
}

void GraphicsScene3dView::Camera::setIsometricView()
{
    reset();

    m_rotAngle.setX(qDegreesToRadians(135.0f));
    m_rotAngle.setY(qDegreesToRadians(45.0f));

    updateCameraParams();
    updateViewMatrix();
}

void GraphicsScene3dView::Camera::setMapView()
{
    reset();

    m_rotAngle.setX(qDegreesToRadians(0.0f));
    m_rotAngle.setY(qDegreesToRadians(0.0f));

    updateViewMatrix();
}

void GraphicsScene3dView::Camera::reset()
{
    m_eye = {0.0f, 0.0f, 20.0f};
    m_lookAt = {0.0f, 0.0f, 0.0f};
    m_relativeOrbitPos = m_eye;

    m_focusedObject.lock() = nullptr;
    m_deltaOffset = {0.0f, 0.0f, 0.0f};
    m_focusPoint = {0.0f, 0.0f, 0.0f};

    m_pitch = 0.f;
    m_yaw = 0.f;
    m_fov = 45.f;
    m_distToFocusPoint = 3050.f;
    distForMapView_ = m_distToFocusPoint;

    distToGround_ = 0.0f;
    angleToGround_ = 0.0f;
    isPerspective_ = false;

    updateCameraParams();
    updateViewMatrix();
}

void GraphicsScene3dView::Camera::resetRotationAngle()
{
    m_rotAngle = {0.0f, 0.0f};

    updateCameraParams();
    updateViewMatrix();
}

void GraphicsScene3dView::Camera::setYerevanLla(LLA yerevan)
{
    startupInitLla  = yerevan;
    viewLlaRef_ = LLARef(startupInitLla);
}

void GraphicsScene3dView::Camera::updateCameraParams()
{
    distToGround_ = std::max(0.0f, std::fabs(-cosf(m_rotAngle.y()) * m_distToFocusPoint));

    float perspEdge = 5000.f;
    if (viewPtr_) {
        perspEdge = viewPtr_->perspectiveEdge_;
    }

    isPerspective_ = distToGround_ < perspEdge;
}

void GraphicsScene3dView::Camera::tryToChangeViewLlaRef()
{
    if (isPerspective_ && viewPtr_) {
        North_East_Down lookAtNed(m_lookAt.x(), m_lookAt.y(), 0.0f);
        LLA lookAtLla(&lookAtNed, &viewLlaRef_, isPerspective_);
        LLARef lookAtLlaRef(lookAtLla);

        float viewDist = map::calculateDistance(lookAtLlaRef, viewLlaRef_);
        float datasetDist = map::calculateDistance(lookAtLlaRef, datasetLlaRef_);

        if (datasetDist < lowDistThreshold_ && getIsFarAwayFromOriginLla()) {
            viewPtr_->setNeedToResetStartPos(true);
            LLA datasetLla(datasetLlaRef_.refLla.latitude, datasetLlaRef_.refLla.longitude, 0.0);
            North_East_Down datasetNed(&datasetLla, &viewLlaRef_, isPerspective_);
            m_lookAt -= QVector3D(datasetNed.n, datasetNed.e, 0.0f);
            viewLlaRef_ = datasetLlaRef_;
        }
        else if (viewDist > highDistThreshold_) {
            viewPtr_->setNeedToResetStartPos(true);
            viewLlaRef_ = lookAtLlaRef;
            m_lookAt = QVector3D(0.0f, 0.0f, 0.0f);
        }
    }
}

void GraphicsScene3dView::Camera::updateViewMatrix()
{
    QVector3D cf;
    cf[0] = -sinf(m_rotAngle.y())*cosf(-m_rotAngle.x())*m_distToFocusPoint;
    cf[1] = -sinf(m_rotAngle.y())*sinf(-m_rotAngle.x())*m_distToFocusPoint;
    cf[2] = -cosf(m_rotAngle.y())*m_distToFocusPoint;

    if (!isPerspective_) {
        m_rotAngle = QVector2D();
    }

    QVector3D cu;
    cu[0] = cosf(m_rotAngle.y())*cosf(-m_rotAngle.x());
    cu[1] = cosf(m_rotAngle.y())*sinf(-m_rotAngle.x());
    cu[2] = -sinf(m_rotAngle.y());

    angleToGround_ = 90.f * std::fabs(cu.z());

    QMatrix4x4 view;
    //LookAt函数：创建一个看着(Look at)给定目标的观察矩阵。三个参数，相机位置pos、目标位置target、相机上向量up
    view.lookAt(cf + m_lookAt, m_lookAt, cu.normalized());
    view.scale(1.0f,1.0f,-1.0f);

    m_view = std::move(view);
}

void GraphicsScene3dView::Camera::checkRotateAngle()
{
    if (m_rotAngle[1] > M_PI_2) {
       m_rotAngle[1] = M_PI_2;
    }
    else if (m_rotAngle[1] < 0.0f) {
       m_rotAngle[1] = 0.0f;
    }
}

void GraphicsScene3dView::Camera::tryResetRotateAngle()
{
    bool preIsPersp{ false };
    distToGround_ = std::max(0.0f, std::fabs(-cosf(m_rotAngle.y()) * m_distToFocusPoint));
    float perspEdge = viewPtr_ ? viewPtr_->perspectiveEdge_ : 5000.0f;
    preIsPersp = distToGround_ < perspEdge;
    bool projectionChanged = isPerspective_ != preIsPersp;
    if (projectionChanged && isPerspective_) {
        m_rotAngle = { 0.0f, 0.0f };
    }
}

float GraphicsScene3dView::Camera::getHeightAboveGround() const
{
    return distToGround_;
}

float GraphicsScene3dView::Camera::getAngleToGround() const
{
    return angleToGround_;
}

bool GraphicsScene3dView::Camera::getIsPerspective() const
{
    return isPerspective_;
}

bool GraphicsScene3dView::Camera::getIsFarAwayFromOriginLla() const
{
    // qDebug() << "Camera:: isPerspective_:" << isPerspective_ << "   viewLlaRef_:" << viewLlaRef_.refLla.latitude << "  "
    //          << viewLlaRef_.refLla.longitude << "    datasetLlaRef_:" << datasetLlaRef_.refLla.latitude << "  "
    //          << datasetLlaRef_.refLla.longitude;
    return !isPerspective_ || (viewLlaRef_ != datasetLlaRef_);
}

map::CameraTilt GraphicsScene3dView::Camera::getCameraTilt() const
{
    float xRot = m_rotAngle.x();

    while (xRot >  M_PI) {
        xRot -= 2.f * M_PI;
    }
    while (xRot <= -M_PI) {
        xRot += 2.f * M_PI;
    }

    float deg = qRadiansToDegrees(xRot);

    if (deg > -45.f && deg <= 45.f) {
        return map::CameraTilt::Down;
    }
    else if (deg > 45.f && deg <= 135.f) {
        return map::CameraTilt::Right;
    }
    else if (deg >= -135.f && deg <= -45.f) {
        return map::CameraTilt::Left;
    }
    else {
        return map::CameraTilt::Up;
    }
}

QVector3D GraphicsScene3dView::Camera::getEyePosition() const
{
    return m_eye;
}

qreal GraphicsScene3dView::Camera::distToFocusPoint() const
{
    return m_distToFocusPoint;
}

QString GraphicsScene3dView::InFboRenderer::checkOpenGLError() const {
    GLenum errorCode = glGetError();
    QString retVal;

    if (errorCode != GL_NO_ERROR) {
        switch (errorCode) {
        case GL_INVALID_ENUM:
            retVal = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            retVal = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            retVal = "GL_INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            retVal = "GL_STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            retVal = "GL_STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            retVal = "GL_OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            retVal = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        default:
            retVal = QString("Unknown error: 0x%1").arg(errorCode, 0, 16);
            break;
        }
    }

    return retVal;
}
