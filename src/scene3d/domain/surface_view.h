#pragma once

#include <vector>
#include <QMutex>
#include <QVector>
#include <QVector3D>
#include <QUuid>

#include "surface_tile.h"
#include "scene_object.h"
#include "dataset_defs.h"


class SurfaceView : public SceneObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(SurfaceView)

public:
    QRectF getSurfaceBounds() const;


public:
    class SurfaceViewRenderImplementation : public SceneObject::RenderImplementation
    {
    public:
        SurfaceViewRenderImplementation();
        virtual void render(QOpenGLFunctions* ctx,  const QMatrix4x4& mvp,  const QMap<QString,
                    std::shared_ptr<QOpenGLShaderProgram>>& shaderProgramMap) const override final;

        float getMaxZ();
        float getMinZ();

        QRectF getSurfaceBounds() const;

        // void renderBoundaryVertices(QOpenGLFunctions* ctx, const QMatrix4x4& mvp,
        //         const QMap<QString, std::shared_ptr<QOpenGLShaderProgram>> &shaderProgramMap) const;

    private:
        friend class SurfaceView;

        QHash<QUuid, SurfaceTile> tiles_;
        GLuint surfaceColorTableTextureId_;
        GLuint mosaicColorTableTextureId_;
        GLenum mosaicColorTableTextureType_;
        float minZ_;
        float maxZ_;
        float surfaceStep_;
        int   colorIntervalsSize_;
        bool  iVis_;
        bool  mVis_;

        // =====边界顶点 =====
        // QVector<QVector<QVector3D>> boundaryGroups_;  // 存储边界顶点
        // QVector<QVector3D> boundaryVertices_;  // 存储边界顶点
        bool boundaryVerticesVisible_;  // 是否显示边界顶点

        float verticalScale_ = -1.0f;  // 与 renderer 一致的负值 vs，用于补偿水平陆地 Z
        bool groundVisible_ = true;
    };

    explicit SurfaceView(QObject* parent = nullptr);
    virtual ~SurfaceView();

    void   setMosaicTextureIdByTileId(QUuid tileId, GLuint textureId);
    void   setMosaicColorTableTextureId(GLuint value);
    void   setSurfaceColorTableTextureId(GLuint textureId);
    void   setIVisible(bool state);
    void   setMVisible(bool state);
    GLuint getMosaicTextureIdByTileId(QUuid tileId) const;
    GLuint getMosaicColorTableTextureId() const;
    GLuint getSurfaceColorTableTextureId() const;
    bool   getMVisible() const;
    bool   getIVisible() const;

    QVector<GLuint>                                 takeMosaicTileTextureToDelete();
    QVector<std::pair<QUuid, std::vector<uint8_t>>> takeMosaicTileTextureToAppend();
    std::vector<uint8_t>                            takeMosaicColorTableToAppend();
    GLuint                                          takeMosaicColorTableToDelete();
    std::vector<uint8_t>                            takeSurfaceColorTableToAppend();
    GLuint                                          takeSurfaceColorTableToDelete();

    void setLlaRef(LLARef llaRef);
    void saveVerticesToFile(const QString& path);

    // ===== 边界顶点相关方法 =====
    void setBoundaryVerticesVisible(bool visible);
    void setVerticalScale(float scale);
    void setGroundVisible(bool visible);

public slots: // from dataprocessor
    void clear();
    void setTiles(const QHash<QUuid, SurfaceTile>& tiles, bool useTextures); // TODO: separate (now from mosaic)
    void setMinZ(float minZ);
    void setMaxZ(float maxZ);
    void setSurfaceStep(float surfaceStep);
    void setTextureTask(const std::vector<uint8_t>& textureTask);
    void setColorIntervalsSize(int size);
    void removeTiles(const QSet<QUuid>& ids); 

private:
    void updateMosaicTileTextureTask(const QHash<QUuid, SurfaceTile>& newTiles);

private:
    QMutex mosaicTexTasksMutex_;

    GLuint                                    mosaicColorTableToDelete_;
    QHash<QUuid, std::vector<uint8_t>>        mosaicTileTextureToAppend_;
    QVector<GLuint>                           mosaicTileTextureToDelete_;
    std::vector<uint8_t>                      surfaceColorTableToAppend_;
    GLuint                                    surfaceColorTableToDelete_;
    LLARef llaRef_;
};
