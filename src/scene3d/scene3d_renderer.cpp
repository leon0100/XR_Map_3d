#include "scene3d_renderer.h"
#include "draw_utils.h"

#include "bottom_track.h"
#include "point_group.h"
#include "polygon_group.h"

#include <QThread>
#include <QDebug>

#include "text_renderer.h"


QString getShaderPath(const QString& name)
{
#if defined(Q_OS_ANDROID) || defined(LINUX_ES)
    return QString(":/shaders/%1_android").arg(name);
#else
    return QString(":/shaders/%1").arg(name);
#endif
}

GraphicsScene3dRenderer::GraphicsScene3dRenderer() : scaleFactor_(1.0f)
{
#if defined(Q_OS_ANDROID) || defined(LINUX_ES)
    scaleFactor_ = 2.0f;
#endif
    m_shaderProgramMap["height"]     = std::make_shared<QOpenGLShaderProgram>();
    m_shaderProgramMap["static"]     = std::make_shared<QOpenGLShaderProgram>();
    m_shaderProgramMap["static_sec"] = std::make_shared<QOpenGLShaderProgram>();
    m_shaderProgramMap["text"]       = std::make_shared<QOpenGLShaderProgram>();
    m_shaderProgramMap["text_back"]  = std::make_shared<QOpenGLShaderProgram>();
    m_shaderProgramMap["mosaic"]     = std::make_shared<QOpenGLShaderProgram>();
    m_shaderProgramMap["image"]      = std::make_shared<QOpenGLShaderProgram>();
    m_shaderProgramMap["isobaths"]   = std::make_shared<QOpenGLShaderProgram>();
}

GraphicsScene3dRenderer::~GraphicsScene3dRenderer()
{
    TextRenderer::instance().cleanup(); // using working ctx
}

void GraphicsScene3dRenderer::initialize()
{
    initializeOpenGLFunctions(); //加载所有OpenGL函数指针

    m_isInitialized = true;

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.3f, 0.3f, 0.3f, 0.0f);


    // static
    if (!m_shaderProgramMap["static"]->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/base.vsh"))
        qCritical() << "Error adding vertex shader from source file.";
    if (!m_shaderProgramMap["static"]->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/static_color.fsh"))
        qCritical() << "Error adding fragment shader from source file.";
    if (!m_shaderProgramMap["static"]->link())
        qCritical() << "Error linking shaders in shader program.";
    // static sec
    if (!m_shaderProgramMap["static_sec"]->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/base_sec.vsh"))
        qCritical() << "Error adding vertex shader from source file.";
    if (!m_shaderProgramMap["static_sec"]->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/static_color.fsh"))
        qCritical() << "Error adding fragment shader from source file.";
    if (!m_shaderProgramMap["static_sec"]->link())
        qCritical() << "Error linking shaders in shader program.";
    // height
    if (!m_shaderProgramMap["height"]->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/base.vsh"))
        qCritical() << "Error adding vertex shader from source file.";
    if (!m_shaderProgramMap["height"]->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/height_color.fsh"))
        qCritical() << "Error adding fragment shader from source file.";
    if (!m_shaderProgramMap["height"]->link())
        qCritical() << "Error linking shaders in shader program.";

    // mosaic
    if (!m_shaderProgramMap["mosaic"]->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/mosaic.vsh"))
        qCritical() << "Error adding mosaic vertex shader from source file.";
    if (!m_shaderProgramMap["mosaic"]->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/mosaic.fsh"))
        qCritical() << "Error adding mosaic fragment shader from source file.";
    if (!m_shaderProgramMap["mosaic"]->link())
        qCritical() << "Error linking mosaic shaders in shader program.";

    // image
    if (!m_shaderProgramMap["image"]->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/image.vsh"))
        qCritical() << "Error adding image vertex shader from source file.";
    if (!m_shaderProgramMap["image"]->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/image.fsh"))
        qCritical() << "Error adding image fragment shader from source file.";
    if (!m_shaderProgramMap["image"]->link())
        qCritical() << "Error linking image shaders in shader program.";

    // text
    if (!m_shaderProgramMap["text"]->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/text.vsh"))
        qCritical() << "Error adding text vertex shader from source file.";
    if (!m_shaderProgramMap["text"]->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/text.fsh"))
        qCritical() << "Error adding text fragment shader from source file.";
    if (!m_shaderProgramMap["text"]->link())
        qCritical() << "Error linking text shaders in shader program.";

    // text back
    if (!m_shaderProgramMap["text_back"]->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/text_back.vsh"))
        qCritical() << "Error adding text_back vertex shader from source file.";
    if (!m_shaderProgramMap["text_back"]->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/text_back.fsh"))
        qCritical() << "Error adding text_back fragment shader from source file.";
    if (!m_shaderProgramMap["text_back"]->link())
        qCritical() << "Error linking text_back shaders in shader program.";

    // isobaths
    if (!m_shaderProgramMap["isobaths"]->addCacheableShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/isobaths_colored.vsh"))
        qCritical() << "Error adding isobaths vertex shader from source file.";
    if (!m_shaderProgramMap["isobaths"]->addCacheableShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/isobaths_colored.fsh"))
        qCritical() << "Error adding isobaths fragment shader from source file.";
    if (!m_shaderProgramMap["isobaths"]->link())
        qCritical() << "Error linking isobaths shaders in shader program.";
}

void GraphicsScene3dRenderer::render()
{
    glDepthMask(true);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // back color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawObjects();

    TextRenderer::instance();
    TextRenderer::instance().setColor("white");
}

void GraphicsScene3dRenderer::setCustomOrtho(float left, float right, float bottom, float top)
{
    orthoLeft_ = left;
    orthoRight_ = right;
    orthoBottom_ = bottom;
    orthoTop_ = top;
    useCustomOrtho_ = true;
}

void GraphicsScene3dRenderer::clearCustomOrtho()
{
    useCustomOrtho_ = false;
}


void GraphicsScene3dRenderer::drawObjects()
{
    QMatrix4x4 model, view, projection;

    const float perspectiveEdge{ 5000.0f };
    const float nearPlanePersp{ 1.0f };
    const float farPlanePersp{ 20000.0f };
    const float nearPlaneOrthoCoeff{ 0.05f };
    const float farPlaneOrthoCoeff{ 1.2f };

    float perspCoeff = m_camera.getHeightAboveGround() / perspectiveEdge;
    qreal perspFixFov = m_camera.fov() + m_camera.fov() * perspCoeff;

    if (m_camera.getIsPerspective()) {
        projection.perspective(perspFixFov, m_viewSize.width() / m_viewSize.height(), nearPlanePersp, farPlanePersp);
    }
    else {
        // 检查是否使用自定义正交投影边界
        if (useCustomOrtho_) {
            float orth_v = std::max(std::abs(orthoTop_ - orthoBottom_), std::abs(orthoRight_ - orthoLeft_)) / 2.0f;
            projection.ortho(orthoLeft_, orthoRight_, orthoBottom_,
                             orthoTop_, orth_v * nearPlaneOrthoCoeff, orth_v * farPlaneOrthoCoeff);
        }
        else {
            // 使用默认的相机距离方式
            float orth_v = m_camera.getHeightAboveGround();
            float aspectRatio = m_viewSize.width() / m_viewSize.height();
            projection.ortho(-orth_v*aspectRatio, orth_v*aspectRatio, -orth_v, orth_v, orth_v * nearPlaneOrthoCoeff,
                            orth_v * farPlaneOrthoCoeff);
        }
    }

    view = m_camera.m_view;
    QMatrix4x4 surfaceModel = model;//nie:test，新建一个锚点缩放surfaceModel矩阵

    model.scale(1.0f, 1.0f, m_verticalScale);
    m_model = std::move(model);
    m_projection = std::move(projection);

    QMatrix4x4 trackModel = m_model;
    if (m_camera.viewLlaRef_.isInit && m_camera.datasetLlaRef_.isInit) {
        LLA datasetLla(m_camera.datasetLlaRef_.refLla.latitude,
                       m_camera.datasetLlaRef_.refLla.longitude, 0.0);
        North_East_Down datasetNed(&datasetLla, &m_camera.viewLlaRef_, m_camera.getIsPerspective());
        trackModel.translate(QVector3D(datasetNed.n, datasetNed.e, 0.0f));
    }

    float anchorZ = surfaceViewRenderImpl_.getMinZ();
    if (!qIsFinite(anchorZ) || anchorZ > 1e6f) anchorZ = 0.0f;
    surfaceModel.translate(0.0f, 0.0f, -anchorZ * (1.0f + m_verticalScale));
    surfaceModel.scale(1.0f, 1.0f, m_verticalScale);

    bool isOut = m_camera.getIsFarAwayFromOriginLla();
    // qDebug() << "........isOut...................." << isOut;

    // 先渲染瓦片地图作为背景层
    mapViewRenderImpl_.render(this, m_model, view, m_projection, m_shaderProgramMap);
    if(useCustomOrtho_) {
        // 保存当前深度测试状态
        GLboolean depthTestEnabled;
        glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);

        // 启用深度测试并设置合适的深度函数
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        // 渲染高度场
        // surfaceViewRenderImpl_.render(this,  m_projection * view * m_model, m_shaderProgramMap);  //高度场
        // isobathsViewRenderImpl_.render(this, m_model, view, m_projection, m_shaderProgramMap);    //等值线
        surfaceViewRenderImpl_.render(this,  m_projection * view * trackModel, m_shaderProgramMap);  //高度场
        isobathsViewRenderImpl_.render(this, trackModel, view, m_projection, m_shaderProgramMap);    //等值线

        // 恢复深度测试状态
        if (depthTestEnabled) {
            glEnable(GL_DEPTH_TEST);
        }
    }


    // 启用深度测试，渲染3D对象
    glEnable(GL_DEPTH_TEST);
    if (!isOut) {
        imageViewRenderImpl_.render(this, m_projection * view * m_model, m_shaderProgramMap);
        m_polygonGroupRenderImpl.render(this, m_projection * view * m_model, m_shaderProgramMap);
        usblViewRenderImpl_.render(this, m_projection * view * m_model, m_shaderProgramMap);
    }

    glDisable(GL_DEPTH_TEST);

    // m_boatTrackRenderImpl.render(this, m_model, view, m_projection, m_shaderProgramMap); //船轨迹
    // m_polygonOutlineRenderImpl.render(this, m_model, view, m_projection, m_shaderProgramMap);
    m_boatTrackRenderImpl.render(this, trackModel, view, m_projection, m_shaderProgramMap); //船轨迹
    m_polygonOutlineRenderImpl.render(this, trackModel, view, m_projection, m_shaderProgramMap);

    glEnable(GL_DEPTH_TEST);
    // float zOffset = surfaceViewRenderImpl_.getMaxZ() + 0.01f;
    // zOffset = qMax(0.01f, zOffset);
    // QMatrix4x4 upModel = m_model;
    // upModel.translate(0.0f, 0.0f, -zOffset);  //向上提升


    surfaceViewRenderImpl_.render(this,  m_projection * view * trackModel, m_shaderProgramMap);  //高度场
    isobathsViewRenderImpl_.render(this, trackModel, view, m_projection, m_shaderProgramMap);    //等值线

    {
        glEnable(GL_DEPTH_TEST);

        QMatrix4x4 nModel;
        nModel.setToIdentity();
        nModel.translate(navigationArrowRenderImpl_.getPosition());
        nModel.rotate(navigationArrowRenderImpl_.getAngle(), 0.f, 0.f, 1.f);
        // float distance =  m_camera.distToFocusPoint();
        float distance = m_camera.getHeightAboveGround();
        float perspFixFovRad = qDegreesToRadians(perspFixFov);
        float factor = 2.0f * distance * std::tan(perspFixFovRad * 0.5f) / m_viewSize.height();
        float worldScale = factor * 10.f * scaleFactor_;
        worldScale  = (worldScale < 0.7) ? 0.7 : worldScale;
        if(distance > 1500.0f) {
            worldScale = 18.0;
        }
        nModel.scale(worldScale);
        navigationArrowRenderImpl_.render(this, projection * view * nModel, m_shaderProgramMap);
        glDisable(GL_DEPTH_TEST);
    }



    //-----------Draw axes-------------
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    // glViewport(viewport[2]-120,0,90,90);
    glViewport(0, 0, 90, 90);

    QMatrix4x4 axesView;
    QMatrix4x4 axesProjection;
    QMatrix4x4 axesModel;

    m_axesThumbnailCamera.setDistance(25);
    axesView = m_axesThumbnailCamera.m_view;
    axesProjection.perspective(m_camera.fov(), 150/150, 1.0f, 11000.0f);
    m_coordAxesRenderImpl.render(this, axesModel, axesView, axesProjection, m_shaderProgramMap);

    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}


