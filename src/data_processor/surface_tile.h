#pragma once

#include <stdint.h>
#include <vector>
#include <QUuid>
#include <QVector>
#include <QVector3D>
#include <QVector2D>
#include <QOpenGLFunctions>


class SurfaceTile;
using TileMap = QHash<QUuid, SurfaceTile>;

static constexpr int   defaultTileSidePixelSize     = 256;
static constexpr int   defaultTileHeightMatrixRatio = 16;
static constexpr float defaultTileResolution        = 1.0f / 10.f;

enum class HeightType {
    kUndefined = 0,
    kExrtapolation,
    kMosaic,
    kTriangulation
};

class SurfaceTile {
public:
    /*methods*/
    SurfaceTile();
    SurfaceTile(QVector3D origin);
    void init(int sidePixelSize, int heightMatrixRatio, float resolution);
    void updateHeightIndices(); // 更新用于绘制的索引

    void                        setMosaicTextureId(GLuint val);
    void                        setIsUpdated(bool state);
    QUuid                       getUuid() const;
    QVector3D                   getOrigin() const;
    bool                        getIsInited() const;
    GLuint                      getMosaicTextureId() const;
    int                         getIsUpdated() const;
    std::vector<uint8_t>&       getMosaicImageDataRef();
    const std::vector<uint8_t>& getMosaicImageDataCRef() const;
    QVector<QVector3D>&         getHeightVerticesRef();
    QVector<HeightType>&        getHeightMarkVerticesRef();
    const QVector<QVector2D>&   getMosaicTextureVerticesCRef() const;
    const QVector<QVector3D>&   getHeightVerticesCRef() const;
    const QVector<int>&         getHeightIndicesCRef() const;

private:
    friend class SurfaceView;

    /*methods*/
    inline bool checkVerticesDepth(int topLeft, int topRight, int bottomLeft, int bottomRight) const;

    /*data*/
    QUuid id_;
    QVector3D origin_;
    std::vector<uint8_t> imageData_;        // 纹理
    QVector<QVector3D> heightVertices_;     // 高度矩阵/高程矩阵
    QVector<HeightType> heightMarkVertices_;// 在高度矩阵里处理什么
    QVector<int> heightIndices_;            // 在高度矩阵里绘制什么
    QVector<QVector2D> textureVertices_;    // 纹理坐标
    GLuint textureId_;
    bool isUpdated_;
    bool isInited_; //
};
