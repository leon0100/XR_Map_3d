#include "custom_track.h"
#include "scene3d_view.h"


CustomTrack::CustomTrack(GraphicsScene3dView* view, QObject* parent) :
    SceneObject(new CustomTrackRenderImplementation, view, parent),
    trackColor_(QColor(255, 0, 0)), // 默认红色
    trackWidth_(2.0f)
{
    setPrimitiveType(GL_LINE_STRIP);
}

CustomTrack::~CustomTrack()
{
}

// 坐标转换：LLA 到 NED
NED CustomTrack::llaToNed(const LLA& lla, LLARef* ref)
{
    if (!ref || !ref->isInit) {
        return NED();
    }

    // 使用 NED 构造函数进行转换
    return NED(const_cast<LLA*>(&lla), ref);
}

// 坐标转换：LLA 到 QVector3D
QVector3D CustomTrack::llaToVector3D(const LLA& lla, LLARef* ref)
{
    NED ned = llaToNed(lla, ref);
    if (!ned.isCoordinatesValid()) {
        return QVector3D();
    }

    return QVector3D(ned.n, ned.e, ned.d);
}

SceneObject::SceneObjectType CustomTrack::type() const
{
    return SceneObject::SceneObjectType::CustomTrack;
}

void CustomTrack::addLLAPoints(const QVector<LLA>& llaPoints)
{
    if (llaPoints.isEmpty()) {
        return;
    }

    LLA llaa = llaPoints.first();
    LLARef llaRef = LLARef(llaa);

    qDebug() << "ustomTrack::addLLAPoints..............";

    QVector<QVector3D> prepData;
    prepData.reserve(llaPoints.size());

    for (const LLA& lla : llaPoints) {
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

void CustomTrack::clearCustomTrack()
{
    SceneObject::clearData();
}

void CustomTrack::setTrackColor(const QColor& color)
{
    trackColor_ = color;
}

void CustomTrack::setTrackWidth(float width)
{
    trackWidth_ = width;
}

void CustomTrack::clearData()
{
    SceneObject::clearData();
}

// RenderImplementation 实现
CustomTrack::CustomTrackRenderImplementation::CustomTrackRenderImplementation()
{
}

CustomTrack::CustomTrackRenderImplementation::~CustomTrackRenderImplementation()
{
}

void CustomTrack::CustomTrackRenderImplementation::render(QOpenGLFunctions* ctx,
                                                          const QMatrix4x4& mvp,
                                                          const QMap<QString, std::shared_ptr<QOpenGLShaderProgram>>& shaderProgramMap) const
{
    if (!m_isVisible || data().empty()) {
        return;
    }

    auto shaderProgram = shaderProgramMap["static"].get();
    if (!shaderProgram->bind()) {
        return;
    }

    // auto* customTrack = static_cast<const CustomTrack*>(m_sceneObject);
    int posLoc = shaderProgram->attributeLocation("position");
    int colorLoc = shaderProgram->uniformLocation("color");
    int matrixLoc = shaderProgram->uniformLocation("matrix");
    int widthLoc = shaderProgram->uniformLocation("width");

    shaderProgram->setUniformValue(colorLoc, DrawUtils::colorToVector4d(color()));
    shaderProgram->setUniformValue(matrixLoc, mvp);
    shaderProgram->setAttributeArray(posLoc, data().constData());

    shaderProgram->enableAttributeArray(posLoc);
    shaderProgram->setAttributeArray(posLoc, data().constData());

    ctx->glLineWidth(width());
    ctx->glDrawArrays(GL_LINE_STRIP, 0, data().size());
    ctx->glLineWidth(1.0f);

    shaderProgram->disableAttributeArray(posLoc);
    shaderProgram->release();
}

void CustomTrack::CustomTrackRenderImplementation::render(QOpenGLFunctions* ctx,
                                                          const QMatrix4x4& model,
                                                          const QMatrix4x4& view,
                                                          const QMatrix4x4& projection,
                                                          const QMap<QString, std::shared_ptr<QOpenGLShaderProgram>>& shaderProgramMap) const
{
    render(ctx, projection * view * model, shaderProgramMap);
}
