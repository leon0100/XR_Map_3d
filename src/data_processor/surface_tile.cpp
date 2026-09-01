#include "surface_tile.h"

#include <cmath>
#include <QQueue>

SurfaceTile::SurfaceTile() :
    id_(QUuid::createUuid()),
    origin_(QVector3D(-1.0f, -1.0f, -1.0f)),
    textureId_(0),
    isUpdated_(false),
    isInited_(false),
    sidePixelSize_(0),
    heightMatrixRatio_(0),
    resolution_(0.0f),
    headIndx_(-1)
{}

SurfaceTile::SurfaceTile(QVector3D origin) :
    id_(QUuid::createUuid()),
    origin_(origin),
    textureId_(0),
    isUpdated_(false),
    isInited_(false),
    sidePixelSize_(0),
    heightMatrixRatio_(0),
    resolution_(0.0f),
    headIndx_(-1)
{}

void SurfaceTile::init(int sidePixelSize, int heightMatrixRatio, float resolution)
{
    // imageData_.resize(sidePixelSize * sidePixelSize, 0);
    sidePixelSize_     = sidePixelSize;
    heightMatrixRatio_ = heightMatrixRatio;
    resolution_        = resolution;

    // height vertices
    int heightMatSideSize = heightMatrixRatio + 1;
    float heightPixelStep = (sidePixelSize / heightMatrixRatio) * resolution;
    int heightMatSize = std::pow(heightMatSideSize, 2);
    heightVertices_.resize(heightMatSize);
    heightMarkVertices_.resize(heightMatSize);
    for (int i = 0; i < heightMatSideSize; ++i) {
        for (int j = 0; j < heightMatSideSize; ++j) {
            float x = origin_.x() + j * heightPixelStep;
            float y = origin_.y() + i * heightPixelStep;
            int currIndx = i * heightMatSideSize + j;
            heightVertices_[currIndx] = QVector3D(x, y, 0.0f);
            heightMarkVertices_[currIndx] = HeightType::kUndefined;
        }
    }

    // texture vertices
    textureVertices_.clear();
    textureVertices_.reserve(heightMatSideSize * heightMatSideSize);
    for (int i = 0; i < heightMatSideSize; ++i) {
        for (int j = 0; j < heightMatSideSize; ++j) {
            textureVertices_.append(QVector2D(float(j) / (heightMatSideSize - 1), float(i) / (heightMatSideSize - 1)));
        }
    } 

    isInited_ = true;
}

void SurfaceTile::initImageData(int sidePixelSize, int heightMatrixRatio)
{
    const int heightMatSideSize = heightMatrixRatio + 1;

    // image data
    imageData_.resize(sidePixelSize * sidePixelSize, 0);

    // texture vertices
    textureVertices_.clear();
    textureVertices_.reserve(heightMatSideSize * heightMatSideSize);
    for (int i = 0; i < heightMatSideSize; ++i) {
        for (int j = 0; j < heightMatSideSize; ++j) {
            textureVertices_.append(QVector2D(float(j) / (heightMatSideSize - 1), float(i) / (heightMatSideSize - 1)));
        }
    }
}

void SurfaceTile::updateHeightIndices()
{
    heightIndices_.clear();
    int heightMatSideSize = std::sqrt(static_cast<int>(heightVertices_.size()));
    for (int i = 0; i < heightMatSideSize - 1; ++i) { // 用于法向传递/用于法线传递
        for (int j = 0; j < heightMatSideSize - 1; ++j) {
            int topLeft = i * heightMatSideSize + j;
            int topRight = topLeft + 1;
            int bottomLeft = (i + 1) * heightMatSideSize + j;
            int bottomRight = bottomLeft + 1;

            if (!checkVerticesDepth(topLeft, topRight, bottomLeft, bottomRight)) {
                continue;
            }

            heightIndices_.append(topLeft);     // 1--3
            heightIndices_.append(bottomLeft);  // | /
            heightIndices_.append(topRight);    // 2
            heightIndices_.append(topRight);    //    1
            heightIndices_.append(bottomLeft);  //  / |
            heightIndices_.append(bottomRight); // 2--3
        }
    }
}

void SurfaceTile::resetInitData()
{
    textureId_ = 0;
    isUpdated_ = false;

    originHeightVertices_.clear();
    std::vector<uint8_t>().swap(imageData_);
    QVector<QVector3D>().swap(heightVertices_);
    QVector<HeightType>().swap(heightMarkVertices_);
    QVector<int>().swap(heightIndices_);
    QVector<QVector2D>().swap(textureVertices_);

    isInited_ = false;
    headIndx_ = -1;
}

void SurfaceTile::setOrigin(const QVector3D &val)
{
    origin_ = val;
}

void SurfaceTile::setHeadIndx(int indx)
{
    headIndx_ = indx;
}

void SurfaceTile::setUpdateHint(UpdateHint h)
{
    updateHint_ = h;
}

void SurfaceTile::setInFov(bool state)
{
    inFov_ = state;
}

UpdateHint SurfaceTile::updateHint() const
{
    return updateHint_;
}

void SurfaceTile::setMosaicTextureId(GLuint val)
{
    textureId_ = val;
}

void SurfaceTile::setIsUpdated(bool state)
{
    isUpdated_ = state;
}

const TileKey &SurfaceTile::getKey() const
{
    return key_;
}

QUuid SurfaceTile::getUuid() const
{
    return id_;
}

QVector3D SurfaceTile::getOrigin() const
{
    return origin_;
}

bool SurfaceTile::getIsInited() const
{
    return isInited_;
}

GLuint SurfaceTile::getMosaicTextureId() const
{
    return textureId_;
}

int SurfaceTile::getIsUpdated() const
{
    return isUpdated_;
}

std::vector<uint8_t>& SurfaceTile::getMosaicImageDataRef()
{
    if (imageData_.empty() && sidePixelSize_ > 0) {
        imageData_.resize(sidePixelSize_ * sidePixelSize_, 0);
    }
    return imageData_;
}

const std::vector<uint8_t> &SurfaceTile::getMosaicImageDataCRef() const
{
    return imageData_;
}

QVector<HeightType> &SurfaceTile::getHeightMarkVerticesRef()
{
    return heightMarkVertices_;
}

QVector<QVector3D>& SurfaceTile::getHeightVerticesRef()
{
    return heightVertices_;
}

const QVector<QVector2D>& SurfaceTile::getMosaicTextureVerticesCRef() const
{
    return textureVertices_;
}

const QVector<QVector3D>& SurfaceTile::getHeightVerticesCRef() const
{
    return heightVertices_;
}

const QVector<HeightType> &SurfaceTile::getHeightMarkVerticesCRef() const
{
    return heightMarkVertices_;
}

const QVector<int>& SurfaceTile::getHeightIndicesCRef() const
{
    return heightIndices_;
}

int SurfaceTile::getHeadIndx() const
{
    return headIndx_;
}

bool SurfaceTile::getInFov() const
{
    return inFov_;
}

bool SurfaceTile::checkVerticesDepth(int topLeft, int topRight, int bottomLeft, int bottomRight) const
{
    if (qFuzzyIsNull(heightVertices_[topLeft].z())     || qFuzzyIsNull(heightVertices_[topRight].z())    ||
        qFuzzyIsNull(heightVertices_[bottomLeft].z())  || qFuzzyIsNull(heightVertices_[bottomRight].z())) {
        return false;
    }
    return true;
}


void SurfaceTile::updateBoundaryStepVertices()
{
    for(auto it : originHeightVertices_) {
        heightVertices_[it.first] = it.second;
    }
    originHeightVertices_.clear();

    const int hvSide = std::sqrt(heightVertices_.size());
    for (int y = 0; y < hvSide; ++y) {
        for (int x = 0; x < hvSide; ++x) {
            int idx = y * hvSide + x;

            // 只检查有效的高度顶点
            if (!isValidHeightVertex(heightVertices_[idx], heightMarkVertices_[idx])) {
                continue;
            }

            // 检查是否是锯齿顶点（相邻有无效顶点）
            bool isStepVertex = false;

            // 检查四个方向的相邻顶点
            if (x > 0) {
                int leftIdx = y * hvSide + (x - 1);
                if (!isValidHeightVertex(heightVertices_[leftIdx], heightMarkVertices_[leftIdx])) {
                    isStepVertex = true;
                }
            }
            if (x < hvSide - 1 && !isStepVertex) {
                int rightIdx = y * hvSide + (x + 1);
                if (!isValidHeightVertex(heightVertices_[rightIdx], heightMarkVertices_[rightIdx])) {
                    isStepVertex = true;
                }
            }
            if (y > 0 && !isStepVertex) {
                int topIdx = (y - 1) * hvSide + x;
                if (!isValidHeightVertex(heightVertices_[topIdx], heightMarkVertices_[topIdx])) {
                    isStepVertex = true;
                }
            }
            if (y < hvSide - 1 && !isStepVertex) {
                int bottomIdx = (y + 1) * hvSide + x;
                if (!isValidHeightVertex(heightVertices_[bottomIdx], heightMarkVertices_[bottomIdx])) {
                    isStepVertex = true;
                }
            }

            if (isStepVertex) {
                // ========== 8个方向 ==========
                const int dirCnt = 8;
                const int dx[dirCnt] = { 0, -1, 0, 1, -1, 1, -1, 1 };  // x方向增量
                const int dy[dirCnt] = { -1, 0, 1, 0, -1, -1, 1, 1 };  // y方向增量
                // 主方向权重1.0，对角线方向权重√2/2
                const float weight[dirCnt] = { 1.0f, 0.707f, 1.0f, 0.707f, 0.707f, 0.707f, 0.707f, 0.707f};

                QVector2D shrinkDirection(0, 0);
                for (int d = 0; d < dirCnt; ++d) {
                    int nx = x + dx[d];
                    int ny = y + dy[d];
                    if (nx < 0 || nx >= hvSide || ny < 0 || ny >= hvSide) {
                        continue;
                    }

                    int nIdx = ny * hvSide + nx;
                    if (isValidHeightVertex(heightVertices_[nIdx], heightMarkVertices_[nIdx])) {
                        // 使用方向权重，确保主方向比对角线方向更重要
                        shrinkDirection += QVector2D(dx[d], dy[d]) * weight[d];
                    }
                }

                shrinkDirection.normalize();
                originHeightVertices_[idx] = heightVertices_[idx];
                heightVertices_[idx].setX(heightVertices_[idx].x() + shrinkDirection.x());
                heightVertices_[idx].setY(heightVertices_[idx].y() + shrinkDirection.y());
            }
        }
    }

}


inline bool SurfaceTile::isValidHeightVertex(const QVector3D& vertex, HeightType mark) const
{
    if(mark == HeightType::kUndefined) {
        return false;
    }

    if(qFuzzyIsNull(vertex.z())) {
        return false;
    }

    return true;
}

