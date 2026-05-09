#pragma once

#include <stdint.h>
#include <vector>
#include <QUuid>
#include <QVector>
#include <QVector3D>
#include <QVector2D>
#include <QOpenGLFunctions>
#include "data_processor_defs.h"


enum class UpdateHint : uint8_t
{
    kUndefined = 0,
    kUpdateTexture,
    kAddOrUpdateTexture
};

class SurfaceTile;
using TileMap = QHash<QUuid, SurfaceTile>;

static constexpr int   defaultTileSidePixelSize     = 256;
static constexpr int   defaultTileHeightMatrixRatio = 16;
static constexpr float defaultTileResolution        = 1.0f / 10.f;


class SurfaceTile {
public:
    /*methods*/
    SurfaceTile();
    SurfaceTile(QVector3D origin);
    void init(int sidePixelSize, int heightMatrixRatio, float resolution);
    void initImageData(int sidePixelSize, int heightMatrixRatio);
    void updateHeightIndices(); // 更新用于绘制的索引

    void       resetInitData();

    void       setOrigin(const QVector3D& val);
    void       setHeadIndx(int indx);
    void       setUpdateHint(UpdateHint h);
    void       setInFov(bool state);

    UpdateHint updateHint() const;

    void                        setMosaicTextureId(GLuint val);
    void                        setIsUpdated(bool state);
    const TileKey&              getKey() const;
    QUuid                       getUuid() const;
    QVector3D                   getOrigin() const;
    bool                        getIsInited() const;
    GLuint                      getMosaicTextureId() const;
    int                         getIsUpdated() const;
    std::vector<uint8_t>&       getMosaicImageDataRef();
    const std::vector<uint8_t>& getMosaicImageDataCRef() const;
    QVector<HeightType>&        getHeightMarkVerticesRef();
    const QVector<QVector2D>&   getMosaicTextureVerticesCRef() const;
    QVector<QVector3D>&         getHeightVerticesRef();
    const QVector<QVector3D>&   getHeightVerticesCRef() const;
    const QVector<HeightType>&  getHeightMarkVerticesCRef() const;
    const QVector<int>&         getHeightIndicesCRef() const;
    int                         getHeadIndx() const;
    bool                        getInFov() const;



    QVector<QVector3D> getBoundaryStepVertices();
    bool isValidHeightVertex(const QVector3D& vertex, HeightType mark) const;
    QVector<QVector<QVector3D>> getBoundaryGroups();
    void contractVertex(QVector3D& vertex, int gridX, int gridY, int hvSide) const;

private:
    friend class SurfaceView;

    /*methods*/
    inline bool checkVerticesDepth(int topLeft, int topRight, int bottomLeft, int bottomRight) const;

    /*data*/
    TileKey   key_;
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
    // tile size
    int   sidePixelSize_;
    int   heightMatrixRatio_;
    float resolution_;
    UpdateHint updateHint_;
    int headIndx_;
    bool inFov_;
};
