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

// 坐标转换：LLA 到 North_East_Down
North_East_Down PolygonOutline::llaToNed(const LLA& lla, LLARef* ref)
{
    if (!ref || !ref->isInit) {
        qDebug() << "lla:" << lla.latitude << "  " << lla.longitude << " " << ref->refLla.latitude;
        return North_East_Down();
    }

    // 使用 North_East_Down 构造函数进行转换
    return North_East_Down(const_cast<LLA*>(&lla), ref);
}

// 坐标转换：LLA 到 QVector3D
QVector3D PolygonOutline::llaToVector3D(const LLA& lla, LLARef* ref)
{
    North_East_Down ned = llaToNed(lla, ref);
    if (!ned.isCoordinatesValid()) {
        return QVector3D();
    }

    return QVector3D(ned.n, ned.e, ned.d);
}


void PolygonOutline::drawPolygonOutline(double latitide, double longitude)
{


}

void PolygonOutline::setOutlineMode(bool outline)
{
    isDrawOutlineMode_ = outline;
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
    if(datasetPtr_ == nullptr) {
        return;
    }
    connect(datasetPtr_, &Dataset::signalDrawOutline, this, &PolygonOutline::slot_setDrawOutlineMode);
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

void PolygonOutline::slot_setDrawOutlineMode(bool outlineMode)
{
    isDrawOutlineMode_ = outlineMode;
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
