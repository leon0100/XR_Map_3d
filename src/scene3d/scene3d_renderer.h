#pragma once

#include "coordinate_axes.h"
#include "plane_grid.h"
#include "bottom_track.h"
#include "isobaths_view.h"
#include "surface_view.h"
#include "image_view.h"
#include "map_view.h"
#include "contacts.h"
#include "point_group.h"
#include "polygon_group.h"
#include "scene3d_view.h"
#include "navigation_arrow.h"
#include "usbl_view.h"
#include <QMatrix4x4>
#include "qsystemdetection.h"
#if !defined(Q_OS_ANDROID) && !defined(LINUX_ES)
#include <GL/gl.h>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QOpenGLTexture>
#else
// #include <QOpenGLFunctions_3_0>
#include <QOpenGLExtraFunctions>
#include <GLES2/gl2.h>
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>
#include <GLES/gl.h>
#endif

#include <QVector2D>
#include <QMutex>

#include <memory>



/*-QOpenGLExtraFunctions到底做了什么
 * 1、封装OpenGL API,提供类型安全的C++成员函数
 * 2、自动加载函数指针，代替GLAD/Glew
 * 3、跨平台抽象，屏蔽 WGL/GLX/CGL 差异
 * 4、集成Qt上下文，与 QOpenGLContext 自动绑定
 * 提供现代OpenGL支持，包含 VAO、UBO、Instancing、纹理等关键特性
一句话：QOpenGLExtraFunctions = Qt 版的 GLAD + 跨平台抽象 + 与 Qt GUI 无缝集成
*/

class QOpenGLShaderProgram;//用于处理OpenGL着色器程序的类
class GraphicsScene3dRenderer : protected QOpenGLExtraFunctions
{
public:
    GraphicsScene3dRenderer();
    virtual ~GraphicsScene3dRenderer();
    void render();

private:
    void initialize();
    void drawObjects();
    QMatrix4x4 model() const;
    QMatrix4x4 view() const;
    QMatrix4x4 projection() const;

protected:
    QMap <QString, std::shared_ptr<QOpenGLShaderProgram>> m_shaderProgramMap;
    bool m_isInitialized = false;

private:
    friend class GraphicsScene3dView;
    friend class InFboRenderer;

    QSizeF m_viewSize;
    GraphicsScene3dView::Camera m_camera;
    GraphicsScene3dView::Camera m_axesThumbnailCamera;
    CoordinateAxes::CoordinateAxesRenderImplementation m_coordAxesRenderImpl;
    PlaneGrid::PlaneGridRenderImplementation           m_planeGridRenderImpl;
    IsobathsView::IsobathsViewRenderImplementation     isobathsViewRenderImpl_;
    SurfaceView::SurfaceViewRenderImplementation       surfaceViewRenderImpl_;
    ImageView::ImageViewRenderImplementation           imageViewRenderImpl_;
    MapView::MapViewRenderImplementation               mapViewRenderImpl_;
    Contacts::ContactsRenderImplementation             contactsRenderImpl_;
    BottomTrack::BottomTrackRenderImplementation       m_bottomTrackRenderImpl;
    PolygonGroup::PolygonGroupRenderImplementation     m_polygonGroupRenderImpl;
    PointGroup::PointGroupRenderImplementation         m_pointGroupRenderImpl;
    BoatTrack::BoatTrackRenderImplementation           m_boatTrackRenderImpl;
    NavigationArrow::NavigationArrowRenderImplementation navigationArrowRenderImpl_;
    UsblView::UsblViewRenderImplementation             usblViewRenderImpl_;
    PolygonOutline::PolygonOutlineRenderImplementation m_polygonOutlineRenderImpl;

    QMatrix4x4 m_model;
    QMatrix4x4 m_projection;
    QRect m_comboSelectionRect;
    Cube m_boundingBox;
    float m_verticalScale = 1.0f;
    bool m_isSceneBoundingBoxVisible = true;
    GLuint VAO, VBO;
    float scaleFactor_;
    float gridVisibility_ = true;

    bool hasDepthRange_ = false;
    float minZ_, maxZ_;

    
    // 框选相关
    bool m_isBoxSelecting = false;
    QPoint m_boxSelectStart;
    QPoint m_boxSelectEnd;
};
