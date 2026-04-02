#include "polygon_outline.h"
#include "scene3d_view.h"


PolygonOutline::PolygonOutline(GraphicsScene3dView* view, QObject* parent) :
    SceneObject(new PolygonOutlineRenderImplementation, view, parent),
    datasetPtr_(nullptr),
    lastIndx_(0),
    trackColor_(QColor(255, 0, 0)), // 默认红色
    trackWidth_(2.0f)
{
    setPrimitiveType(GL_POLYGON);

    // setColor({0, 0, 205});
    setColor({255, 64, 64});
    setWidth(4.0f);
}

PolygonOutline::~PolygonOutline()
{
}

// 坐标转换：LLA 到 NED
NED PolygonOutline::llaToNed(const LLA& lla, LLARef* ref)
{
    if (!ref || !ref->isInit) {
        qDebug() << "lla:" << lla.latitude << "  " << lla.longitude << " " << ref->refLla.latitude;
        return NED();
    }

    // 使用 NED 构造函数进行转换
    return NED(const_cast<LLA*>(&lla), ref);
}

// 坐标转换：LLA 到 QVector3D
QVector3D PolygonOutline::llaToVector3D(const LLA& lla, LLARef* ref)
{
    NED ned = llaToNed(lla, ref);
    if (!ned.isCoordinatesValid()) {
        return QVector3D();
    }

    return QVector3D(ned.n, ned.e, ned.d);
}


void PolygonOutline::drawPolygonOutline(double latitide, double longitude)
{
    // datasetPtr_->addPosition_track(currentLat_, currentLon_);



    // Epoch* lastEp = datasetPtr_->last();
    // if (!lastEp) {
    //     return;
    // }

    // Position pos;
    // pos.lla = LLA(latitide, longitude);
    // if (pos.lla.isCoordinatesValid()) {
    //     if (lastEp->getPositionGNSS().lla.isCoordinatesValid()) {
    //         lastEp = datasetPtr_->addNewEpoch();  //不断累加帧数的下标index
    //         // lastEp->setDistProcesing_CSV(depth);
    //     }



    //     if (!_llaRef.isInit) {
    //         _llaRef = LLARef(pos.lla);

    //         surfaceView_->setLlaRef(_llaRef);
    //         m_camera->datasetLlaRef_ = _llaRef.isInit ? _llaRef : LLARef(m_camera->yerevanLla);
    //         m_camera->viewLlaRef_ = m_camera->datasetLlaRef_;

    //         QQuickFramebufferObject::update();
    //         fitAllInView();
    //     }
    //     lastEp->setPositionLLA(pos);
    //     lastEp->setPositionRef(&llaRef); //在这里将LLA坐标转化成本地NED坐标

    //     QVector<Epoch> pool = datasetPtr_->getPool();
    //     qDebug() << "pool_size().............................. " << pool.size();
    //     uint64_t lastIndx = pool.size() - 1;
    //     auto* epPtr = datasetPtr_->fromIndex(lastIndx);
    //     if (!epPtr) {
    //         return;
    //     }

    //     const Position boatPos = epPtr->getPositionGNSS();
    //     if (!boatPos.ned.isCoordinatesValid()) {
    //         return;
    //     }
    //     boatTrack_->onPositionAdded(lastIndx);
    // }

}

bool PolygonOutline::getOutlineMode() const
{
    return isDrawOutlineMode_;
}

LLARef PolygonOutline::getLlaRef()
{
    return llaRef_;
}

void PolygonOutline::setLlaRef(const LLARef &val)
{
    llaRef_ = val;
}

SceneObject::SceneObjectType PolygonOutline::type() const
{
    return SceneObject::SceneObjectType::PolygonOutline;
}

void PolygonOutline::setDatasetPtr(Dataset* datasetPtr)
{
    datasetPtr_ = datasetPtr;
}

void PolygonOutline::polygonAddPoint()
{
    qDebug() << "PolygonOutline::polygonAddPoint...............";
    if (!datasetPtr_) {
        return;
    }

    QVector<Epoch> polygonOutline = datasetPtr_->getPolygonOutline();
    qDebug() << "polygonOutline.size().............................. " << polygonOutline.size();
    const int toIndx = polygonOutline.size() - 1;
    Epoch* epochPtr = datasetPtr_->fromPolygonOutlineIndex(toIndx);
    if (!epochPtr) {
        qDebug() << "to* epochPtr.................";
        return;
    }

    const Position boatPos = epochPtr->getPositionGNSS();
    if (!boatPos.ned.isCoordinatesValid()) {
        qDebug() << "!boatPos...............";
        return;
    }

    const int fromIndx = lastIndx_;
    if (fromIndx >= toIndx) {
        qDebug() << "fromIndx >= toIndx..............";
        return;
    }

    const int need = toIndx - fromIndx;
    QVector<QVector3D> prepData;
    prepData.reserve(need);

    for (int i = fromIndx + 1; i <= toIndx; ++i) {
        if (auto* ep = datasetPtr_->fromPolygonOutlineIndex(i); ep) {
            if (auto posNed = ep->getPositionGNSS().ned; posNed.isCoordinatesValid()) {
                prepData.push_back(QVector3D(posNed.n, posNed.e, 0));
            }
        }
    }

    lastIndx_ = toIndx;

    SceneObject::appendData(prepData);
}

void PolygonOutline::addLLAPoints(const QVector<LLA>& llaPoints)
{
    qDebug() << "PolygonOutline::addLLAPoints - start, input size:" << llaPoints.size();
    qDebug() << "PolygonOutline::addLLAPoints - current data size:" << data().size();
    if (llaPoints.isEmpty()) {
        return;
    }

    LLA llaa = llaPoints.first();
    LLARef llaRef = LLARef(llaa);

    QVector<QVector3D> prepData;
    prepData.reserve(llaPoints.size());

    for (const LLA& lla : llaPoints) {
        qDebug() << "PolygonOutline::addLLAPoints - adding point:" << lla.latitude << lla.longitude << lla.altitude;
        if (std::isfinite(lla.latitude) && std::isfinite(lla.longitude)) {
            // 使用新的坐标转换函数
            QVector3D point = llaToVector3D(lla, &llaRef);
            if (!point.isNull()) {
                qDebug() << "ustomTrack::addLLAPoints.....333333333.........";
                prepData.push_back(point);
            }
        }
    }

    if (!prepData.isEmpty()) {
        SceneObject::appendData(prepData);
    }
}

void PolygonOutline::clearPolygonOutline()
{
    SceneObject::clearData();
}

void PolygonOutline::setTrackColor(const QColor& color)
{
    trackColor_ = color;
}

void PolygonOutline::setTrackWidth(float width)
{
    trackWidth_ = width;
}

void PolygonOutline::clearData()
{
    SceneObject::clearData();
}

// RenderImplementation 实现
PolygonOutline::PolygonOutlineRenderImplementation::PolygonOutlineRenderImplementation()
{
}

PolygonOutline::PolygonOutlineRenderImplementation::~PolygonOutlineRenderImplementation()
{
}

void PolygonOutline::PolygonOutlineRenderImplementation::render(QOpenGLFunctions* ctx,  const QMatrix4x4& mvp,
                      const QMap<QString, std::shared_ptr<QOpenGLShaderProgram>>& shaderProgramMap) const
{
    // qDebug() << "PolygonOutline::PolygonOutlineRenderImplementa...........";
    if (!m_isVisible)
        return;

    // qDebug() << "m_isVisible" << m_isVisible << "  data().size(): " << data().size();
    if (!m_isVisible || data().empty()) {
        return;
    }

    auto shaderProgram = shaderProgramMap["static"].get();
    if (!shaderProgram->bind()) {
        qDebug() << "shaderProgram->bind()............";
        return;
    }

    int posLoc    = shaderProgram->attributeLocation("position");
    int colorLoc  = shaderProgram->uniformLocation("color");
    int matrixLoc = shaderProgram->uniformLocation("matrix");
    int widthLoc  = shaderProgram->uniformLocation("width");

    shaderProgram->setUniformValue(colorLoc, DrawUtils::colorToVector4d(color()));
    shaderProgram->setUniformValue(matrixLoc, mvp);

    shaderProgram->enableAttributeArray(posLoc);
    shaderProgram->setAttributeArray(posLoc, data().constData());

    ctx->glLineWidth(width());
    ctx->glDrawArrays(GL_POLYGON, 0, data().size());
    ctx->glLineWidth(1.0f);

    shaderProgram->disableAttributeArray(posLoc);
    shaderProgram->release();
}

void PolygonOutline::PolygonOutlineRenderImplementation::render(QOpenGLFunctions* ctx,
                                                          const QMatrix4x4& model,
                                                          const QMatrix4x4& view,
                                                          const QMatrix4x4& projection,
                                                          const QMap<QString, std::shared_ptr<QOpenGLShaderProgram>>& shaderProgramMap) const
{
    SceneObject::RenderImplementation::render(ctx, model, view, projection, shaderProgramMap);
}
