#include "surface_tile.h"
#include "dataset_defs.h"

#include <cmath>
#include <QQueue>

SurfaceTile::SurfaceTile() :
    id_(QUuid::createUuid()),
    origin_(QVector3D(-1.0f, -1.0f, -1.0f)),
    textureId_(0),
    isUpdated_(false),
    isInited_(false)
{}

SurfaceTile::SurfaceTile(QVector3D origin) :
    id_(QUuid::createUuid()),
    origin_(origin),
    textureId_(0),
    isUpdated_(false),
    isInited_(false)
{}

void SurfaceTile::init(int sidePixelSize, int heightMatrixRatio, float resolution)
{
    // image data
    imageData_.resize(sidePixelSize * sidePixelSize, 0);

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
    // height indices
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


QVector<QVector3D> SurfaceTile::getBoundaryStepVertices()
{
    QVector<QVector3D> stepVertices;
    const int calculateHvSide = std::sqrt(heightVertices_.size());

    for (int y = 0; y < calculateHvSide; ++y) {
        for (int x = 0; x < calculateHvSide; ++x) {
            int idx = y * calculateHvSide + x;

            // 只检查有效的高度顶点
            if (!isValidHeightVertex(heightVertices_[idx], heightMarkVertices_[idx])) {
                continue;
            }

            // 检查是否是锯齿顶点（相邻有无效顶点）
            bool isStepVertex = false;

            // 检查四个方向的相邻顶点
            if (x > 0) {
                int leftIdx = y * calculateHvSide + (x - 1);
                if (!isValidHeightVertex(heightVertices_[leftIdx], heightMarkVertices_[leftIdx])) {
                    isStepVertex = true;
                }
            }
            if (x < calculateHvSide - 1 && !isStepVertex) {
                int rightIdx = y * calculateHvSide + (x + 1);
                if (!isValidHeightVertex(heightVertices_[rightIdx], heightMarkVertices_[rightIdx])) {
                    isStepVertex = true;
                }
            }
            if (y > 0 && !isStepVertex) {
                int topIdx = (y - 1) * calculateHvSide + x;
                if (!isValidHeightVertex(heightVertices_[topIdx], heightMarkVertices_[topIdx])) {
                    isStepVertex = true;
                }
            }
            if (y < calculateHvSide - 1 && !isStepVertex) {
                int bottomIdx = (y + 1) * calculateHvSide + x;
                if (!isValidHeightVertex(heightVertices_[bottomIdx], heightMarkVertices_[bottomIdx])) {
                    isStepVertex = true;
                }
            }

            if (isStepVertex) {
                contractVertex(heightVertices_[idx], x, y, calculateHvSide);
                stepVertices.append(heightVertices_[idx]);
            }
        }
    }

    return stepVertices;
}

// 在原地收缩顶点（直接修改传入的顶点）还能继续改进，收缩方向加到8个
void SurfaceTile::contractVertex(QVector3D& vertex, int gridX, int gridY, int hvSide) const
{
    float originalX = vertex.x();
    float originalY = vertex.y();

    QVector2D shrinkDirection(0, 0);
    int validDirections = 0;

    const int dx[] = {-1, 1, 0, 0};
    const int dy[] = {0, 0, -1, 1};

    // 检查四个主方向，向有效区域方向收缩
    for (int d = 0; d < 4; ++d) {
        int nx = gridX + dx[d];
        int ny = gridY + dy[d];

        if (nx < 0 || nx >= hvSide || ny < 0 || ny >= hvSide) continue;

        int nIdx = ny * hvSide + nx;
        if (isValidHeightVertex(heightVertices_[nIdx], heightMarkVertices_[nIdx])) {
            shrinkDirection += QVector2D(dx[d], dy[d]);
            validDirections++;
        }
    }

    // 如果主方向没有有效顶点，检查对角线方向
    if (validDirections == 0) {
        const int diagDx[] = {-1, -1, 1, 1};
        const int diagDy[] = {-1, 1, -1, 1};

        for (int d = 0; d < 4; ++d) {
            int nx = gridX + diagDx[d];
            int ny = gridY + diagDy[d];

            if (nx < 0 || nx >= hvSide || ny < 0 || ny >= hvSide) continue;

            int nIdx = ny * hvSide + nx;

            if (isValidHeightVertex(heightVertices_[nIdx], heightMarkVertices_[nIdx])) {
                shrinkDirection += QVector2D(diagDx[d] * 0.707f, diagDy[d] * 0.707f);
                validDirections++;
            }
        }
    }

    if (validDirections == 0 || qFuzzyIsNull(shrinkDirection.length())) {
        qDebug() << "No shrink direction found at (" << gridX << "," << gridY << ")";
        return;
    }

    shrinkDirection.normalize();

    // ========== 第二步：计算收缩距离（三角剖分最短边长）==========
    float stepSize = sidePixelSize_ / heightMatrixRatio_;
    float shrinkDistance = stepSize * 1.0f;  // 收缩距离

    vertex.setX(vertex.x() + shrinkDirection.x() * shrinkDistance);
    vertex.setY(vertex.y() + shrinkDirection.y() * shrinkDistance);

    float actualShrinkX = vertex.x() - originalX;
    float actualShrinkY = vertex.y() - originalY;

    qDebug() << "=========== Shrink Info ============";
    qDebug() << "Grid position: (" << gridX << ", " << gridY << ")";
    qDebug() << "Original position: (" << originalX << ", " << originalY << ")";
    qDebug() << "Shrink direction: (" << shrinkDirection.x() << ", " << shrinkDirection.y() << ")";
    qDebug() << "Shrink distance: " << shrinkDistance;
    qDebug() << "Actual shrink X,Y: " << actualShrinkX << "  " << actualShrinkY;
    qDebug() << "New position: (" << vertex.x() << ", " << vertex.y() << ")";
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

