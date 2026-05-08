#include "surface_tile.h"


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

QVector<QVector3D>& SurfaceTile::getHeightVerticesRef()
{
    return heightVertices_;
}

QVector<HeightType> &SurfaceTile::getHeightMarkVerticesRef()
{
    return heightMarkVertices_;
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



QVector<QVector3D> SurfaceTile::getBoundaryStepVertices() const
{
    QVector<QVector3D> stepVertices;
    const int calculatedHvSide = std::sqrt(heightVertices_.size());

    for (int y = 0; y < calculatedHvSide; ++y) {
        for (int x = 0; x < calculatedHvSide; ++x) {
            int idx = y * calculatedHvSide + x;

            // 只检查有效的高度顶点
            if (!isValidHeightVertex(heightVertices_[idx], heightMarkVertices_[idx])) {
                continue;
            }

            // 检查是否是锯齿顶点（相邻有无效顶点）
            bool isStepVertex = false;

            // 检查四个方向的相邻顶点
            if (x > 0) {
                int leftIdx = y * calculatedHvSide + (x - 1);
                if (!isValidHeightVertex(heightVertices_[leftIdx], heightMarkVertices_[leftIdx])) {
                    isStepVertex = true;
                }
            }
            if (x < calculatedHvSide - 1 && !isStepVertex) {
                int rightIdx = y * calculatedHvSide + (x + 1);
                if (!isValidHeightVertex(heightVertices_[rightIdx], heightMarkVertices_[rightIdx])) {
                    isStepVertex = true;
                }
            }
            if (y > 0 && !isStepVertex) {
                int topIdx = (y - 1) * calculatedHvSide + x;
                if (!isValidHeightVertex(heightVertices_[topIdx], heightMarkVertices_[topIdx])) {
                    isStepVertex = true;
                }
            }
            if (y < calculatedHvSide - 1 && !isStepVertex) {
                int bottomIdx = (y + 1) * calculatedHvSide + x;
                if (!isValidHeightVertex(heightVertices_[bottomIdx], heightMarkVertices_[bottomIdx])) {
                    isStepVertex = true;
                }
            }

            if (isStepVertex) {
                stepVertices.append(heightVertices_[idx]);
            }
        }
    }

    qDebug() << "Found" << stepVertices.size() << "step vertices";
    return stepVertices;
}
// QVector<QVector3D> SurfaceTile::getBoundaryStepVertices() const
// {
//     QVector<QVector3D> stepVertices;
//     const int calculatedHvSide = std::sqrt(heightVertices_.size());

//     // 使用 map 按坐标排序，确保点按顺序排列
//     std::map<std::pair<float, float>, QVector3D> sortedPoints;

//     for (int y = 0; y < calculatedHvSide; ++y) {
//         for (int x = 0; x < calculatedHvSide; ++x) {
//             int idx = y * calculatedHvSide + x;

//             // 只检查有效的高度顶点
//             if (!isValidHeightVertex(heightVertices_[idx], heightMarkVertices_[idx])) {
//                 continue;
//             }

//             // 检查是否是边界顶点（相邻有无效顶点）
//             bool isBoundaryVertex = false;

//             // 检查四个方向的相邻顶点
//             if (x == 0 || !isValidHeightVertex(heightVertices_[y * calculatedHvSide + x - 1], heightMarkVertices_[y * calculatedHvSide + x - 1])) {
//                 isBoundaryVertex = true;
//             } else if (x == calculatedHvSide - 1 || !isValidHeightVertex(heightVertices_[y * calculatedHvSide + x + 1], heightMarkVertices_[y * calculatedHvSide + x + 1])) {
//                 isBoundaryVertex = true;
//             } else if (y == 0 || !isValidHeightVertex(heightVertices_[(y - 1) * calculatedHvSide + x], heightMarkVertices_[(y - 1) * calculatedHvSide + x])) {
//                 isBoundaryVertex = true;
//             } else if (y == calculatedHvSide - 1 || !isValidHeightVertex(heightVertices_[(y + 1) * calculatedHvSide + x], heightMarkVertices_[(y + 1) * calculatedHvSide + x])) {
//                 isBoundaryVertex = true;
//             }

//             if (isBoundaryVertex) {
//                 sortedPoints[{heightVertices_[idx].x(), heightVertices_[idx].y()}] = heightVertices_[idx];
//             }
//         }
//     }

//     // 转换为有序的 QVector
//     for (const auto& pair : sortedPoints) {
//         stepVertices.append(pair.second);
//     }

//     qDebug() << "Found" << stepVertices.size() << "boundary vertices";
//     return stepVertices;
// }



// 添加到文件末尾
inline bool SurfaceTile::isValidHeightVertex(const QVector3D& vertex, HeightType mark) const
{
    // 有效的高度顶点需要满足：
    // 1. mark 不是 kUndefined
    // 2. 高度值有效（非零）
    return mark != HeightType::kUndefined;
}


QVector<QVector<QVector3D>> SurfaceTile::getBoundaryGroups() const
{
    QVector<QVector<QVector3D>> groups;
    const int calculatedHvSide = std::sqrt(heightVertices_.size());

    // ========== 第一步：找出所有锯齿顶点 ==========
    QVector<QVector<bool>> isStepVertex(calculatedHvSide, QVector<bool>(calculatedHvSide, false));

    for (int y = 0; y < calculatedHvSide; ++y) {
        for (int x = 0; x < calculatedHvSide; ++x) {
            int idx = y * calculatedHvSide + x;

            // 只检查有效的高度顶点
            if (!isValidHeightVertex(heightVertices_[idx], heightMarkVertices_[idx])) {
                continue;
            }

            // 检查是否是锯齿顶点（相邻有无效顶点）
            bool isStep = false;

            // 检查左边
            if (x > 0) {
                int leftIdx = y * calculatedHvSide + (x - 1);
                if (!isValidHeightVertex(heightVertices_[leftIdx], heightMarkVertices_[leftIdx])) {
                    isStep = true;
                }
            }
            // 检查右边
            if (x < calculatedHvSide - 1 && !isStep) {
                int rightIdx = y * calculatedHvSide + (x + 1);
                if (!isValidHeightVertex(heightVertices_[rightIdx], heightMarkVertices_[rightIdx])) {
                    isStep = true;
                }
            }
            // 检查上边
            if (y > 0 && !isStep) {
                int topIdx = (y - 1) * calculatedHvSide + x;
                if (!isValidHeightVertex(heightVertices_[topIdx], heightMarkVertices_[topIdx])) {
                    isStep = true;
                }
            }
            // 检查下边
            if (y < calculatedHvSide - 1 && !isStep) {
                int bottomIdx = (y + 1) * calculatedHvSide + x;
                if (!isValidHeightVertex(heightVertices_[bottomIdx], heightMarkVertices_[bottomIdx])) {
                    isStep = true;
                }
            }

            isStepVertex[y][x] = isStep;
        }
    }

    // ========== 第二步：使用 BFS 分组锯齿顶点 ==========
    QVector<QVector<bool>> visited(calculatedHvSide, QVector<bool>(calculatedHvSide, false));

    // 4个方向
    const int dx[] = {-1, 1, 0, 0};
    const int dy[] = {0, 0, -1, 1};

    for (int y = 0; y < calculatedHvSide; ++y) {
        for (int x = 0; x < calculatedHvSide; ++x) {
            if (visited[y][x]) continue;
            if (!isStepVertex[y][x]) continue;  // 只处理锯齿顶点

            // 使用 BFS 找到连通的锯齿顶点组
            QVector<QVector3D> group;
            QQueue<QPair<int, int>> queue;
            queue.enqueue({x, y});
            visited[y][x] = true;

            while (!queue.isEmpty()) {
                auto pos = queue.dequeue();
                int cx = pos.first;
                int cy = pos.second;
                int cIdx = cy * calculatedHvSide + cx;

                group.append(heightVertices_[cIdx]);

                // 检查四个方向的相邻锯齿顶点
                for (int d = 0; d < 4; ++d) {
                    int nx = cx + dx[d];
                    int ny = cy + dy[d];

                    if (nx < 0 || nx >= calculatedHvSide || ny < 0 || ny >= calculatedHvSide) continue;
                    if (visited[ny][nx]) continue;
                    if (!isStepVertex[ny][nx]) continue;  // 只连接锯齿顶点

                    visited[ny][nx] = true;
                    queue.enqueue({nx, ny});
                }
            }

            // 只添加有多个顶点的组
            if (group.size() >= 2) {
                // 按 x, y 坐标排序
                std::sort(group.begin(), group.end(),
                          [](const QVector3D& a, const QVector3D& b) {
                              if (a.x() != b.x()) return a.x() < b.x();
                              return a.y() < b.y();
                          });

                groups.append(group);
            }
        }
    }

    qDebug() << "Found" << groups.size() << "step vertex groups";
    return groups;
}

