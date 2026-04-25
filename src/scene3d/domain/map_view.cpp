#include "map_view.h"
#include <QObject>

#include "scene3d_view.h"


static inline bool toTightRGBA8888(const QImage& in, QByteArray& out, int& w, int& h)
{
    if (in.isNull()) {
        return false;
    }

    QImage img = (in.format() == QImage::Format_RGBA8888) ? in : in.convertToFormat(QImage::Format_RGBA8888);

    w = img.width();
    h = img.height();
    if (w <= 0 || h <= 0) {
        return false;
    }

    const int dstStride = w * 4;
    const int srcStride = img.bytesPerLine();

    out.resize(dstStride * h);

    const uchar* src = img.constBits();
    uchar*       dst = reinterpret_cast<uchar*>(out.data());

    if (srcStride == dstStride) {
        memcpy(dst, src, out.size());
    }
    else {
        for (int y = 0; y < h; ++y) {
            memcpy(dst + y * dstStride, src + y * srcStride, dstStride);
        }
    }

    return true;
}


// MapView::MapView(QObject *parent) : SceneObject(new MapViewRenderImplementation, parent)
MapView::MapView(QObject *parent) : SceneObject(new MapViewRenderImplementation(this), parent)
{
    qRegisterMetaType<map::TileIndex>("map::TileIndex");
    qRegisterMetaType<GLuint>("GLuint");
}

MapView::~MapView()
{

}

void MapView::clear()
{
    appendTasks_.clear();
    updateImageTasks_.clear();

    tileImages_.clear();

    auto r = RENDER_IMPL(MapView);
    for (const auto& [tileIndx, tile] : r->tilesHash_) {
        deleteTasks_.append(tileIndx);
    }
    r->tilesHash_.clear();

    Q_EMIT changed();
    Q_EMIT boundsChanged();
}

void MapView::update()
{
    Q_EMIT changed();
    Q_EMIT boundsChanged();
}

std::unordered_map<map::TileIndex, QImage> MapView::takeInitTileTasks()
{
    auto tmp = std::move(appendTasks_);
    appendTasks_.clear();
    return tmp;
}

std::unordered_map<map::TileIndex, QImage> MapView::takeUpdateTileTasks()
{
    auto tmp = std::move(updateImageTasks_);
    updateImageTasks_.clear();
    return tmp;
}

QVector<map::TileIndex> MapView::takeDeleteTileTasks()
{
    auto tmp = std::move(deleteTasks_);
    deleteTasks_.clear();
    return tmp;
}

bool MapView::getTileImage(const map::TileIndex& tileIndx, QImage& out) const
{
    qDebug().noquote()
        << QString("[MapView::getTileImage] Looking for TileIndex(x=%1,y=%2,z=%3,provider=%4) hash=%5, total tiles=%6")
               .arg(tileIndx.x_).arg(tileIndx.y_).arg(tileIndx.z_).arg(tileIndx.providerId_)
               .arg(std::hash<map::TileIndex>()(tileIndx)).arg(tileImages_.size());

    auto it = tileImages_.find(tileIndx);
    if (it == tileImages_.end() || it->second.isNull()) {
        qDebug() << "[MapView::getTileImage] Tile NOT FOUND or is NULL";

        qDebug() << "[MapView::getTileImage] Stored tiles in MapView:";
        int count = 0;
        for (const auto& [idx, img] : tileImages_) {
            if (count < 10) {
                qDebug().noquote()
                    << QString("  [%1] TileIndex(x=%2,y=%3,z=%4,provider=%5) hash=%6")
                           .arg(count).arg(idx.x_).arg(idx.y_).arg(idx.z_)
                           .arg(idx.providerId_).arg(std::hash<map::TileIndex>()(idx));
                count++;
            }
        }
        if (tileImages_.size() > 10) {
            qDebug() << "  ... and" << (tileImages_.size() - 10) << "more tiles";
        }

        return false;
    }

    // qDebug() << "[MapView::getTileImage] Tile FOUND, size:" << it->second.size();
    out = it->second;
    return true;
}

void MapView::setCurrentMapSource(MapSourceType source)
{
    auto r = RENDER_IMPL(MapView);
    r->currentMapType_ = source;
}

void MapView::setViewLlaRef(LLARef viewLlaRef)
{
    auto r = RENDER_IMPL(MapView);
    r->viewLlaRef_ = viewLlaRef;
}

void MapView::onTileAppend(const map::Tile &tile)
{
    auto r = RENDER_IMPL(MapView);
    auto tileIndx = tile.getIndex();
    r->tilesHash_.emplace(tileIndx, tile);
    appendTasks_[tileIndx] = tile.getImage();
    tileImages_[tileIndx]  = tile.getImage();
    Q_EMIT changed();  
}

void MapView::onTileDelete(const map::TileIndex& tileIndx)
{
    deleteTasks_.append(tileIndx);
    tileImages_.erase(tileIndx);

    auto r = RENDER_IMPL(MapView);
    r->tilesHash_.erase(tileIndx); // CPU-часть можно убрать сразу
    Q_EMIT changed();
}

void MapView::onTileImageUpdated(const map::TileIndex& tileIndx, const QImage& image)
{
    updateImageTasks_[tileIndx] = image;
    tileImages_[tileIndx] = image;

    Q_EMIT changed();
}

void MapView::onTileVerticesUpdated(const map::TileIndex& tileIndx, const QVector<QVector3D>& vertices)
{
    auto r = RENDER_IMPL(MapView);

    if (auto it = r->tilesHash_.find(tileIndx); it != r->tilesHash_.end()) {
        it->second.setVertices(vertices);
    }

    Q_EMIT changed();
}

void MapView::onClearAppendTasks()
{
    auto copyAppendTasks = std::move(appendTasks_);
    appendTasks_.clear();
    auto r = RENDER_IMPL(MapView);

    for (auto it = copyAppendTasks.begin(); it != copyAppendTasks.end(); ++it) {
        if (auto itSec = r->tilesHash_.find(it->first); itSec != r->tilesHash_.end()) {
            r->tilesHash_.erase(itSec->first);
        }

        tileImages_.erase(it->first);

        emit deletedFromAppend(it->first);
    }
}


// MapView::MapViewRenderImplementation::MapViewRenderImplementation()
// {}
MapView::MapViewRenderImplementation::MapViewRenderImplementation(MapView* mapView) : mapView_(mapView)
{
}

MapView::MapViewRenderImplementation::MapViewRenderImplementation() : mapView_(nullptr)
{}

MapView::MapViewRenderImplementation::~MapViewRenderImplementation()
{}

void MapView::MapViewRenderImplementation::copyCpuSideFrom(const MapView::MapViewRenderImplementation& s)
{
    m_isVisible     = s.m_isVisible;
    currentMapType_ = s.currentMapType_;
    viewLlaRef_     = s.viewLlaRef_;
    mapView_        = s.mapView_;

    for (const auto& [idx, srcTile] : s.tilesHash_) { // update verts, not textId
        auto it = tilesHash_.find(idx);
        if (it == tilesHash_.end()) {
            auto t = srcTile;
            t.setTextureId(0);
            tilesHash_.emplace(idx, std::move(t));
        }
        else {
            it->second.setVertices(srcTile.getVerticesRef());
        }
    }
}

// 使用射线法判断点是否在多边形内
bool MapView::MapViewRenderImplementation::isPointInPolygon(const QVector3D& point, const QVector<QVector3D>& polygon) const
{
    bool inside = false;
    int n = polygon.size();
    for (int i = 0, j = n - 1; i < n; j = i++) {
        if (((polygon[i].y() > point.y()) != (polygon[j].y() > point.y())) &&
            (point.x() < (polygon[j].x() - polygon[i].x()) * (point.y() - polygon[i].y()) / (polygon[j].y() - polygon[i].y()) + polygon[i].x())) {
            inside = !inside;
        }
    }
    return inside;
}

void MapView::MapViewRenderImplementation::processPendingTextureTasks(QOpenGLFunctions *gl) const
{
    if (!gl)  return;
    gl->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // del
    if (!pendingDelete_.isEmpty()) {
        for (auto it = pendingDelete_.cbegin(); it != pendingDelete_.cend(); ++it) {
        //for (const auto& idx : pendingDelete_) {
            auto tIt = tilesHash_.find(*it);
            if (tIt == tilesHash_.end()) {
                continue;
            }

            GLuint tex = tIt->second.getTextureId();
            if (tex) {
                gl->glDeleteTextures(1, &tex);
                tIt->second.setTextureId(0); //
            }
            tilesHash_.erase(tIt);
        }
        pendingDelete_.clear();
    }

    // append
    if (!pendingInit_.isEmpty()) {
        for (auto& t : pendingInit_) {
            auto& idx = t.idx;

            QByteArray tight;
            int w = 0, h = 0;
            if (!toTightRGBA8888(t.img, tight, w, h)) {
                continue;
            }

            GLuint tex = 0;
            gl->glGenTextures(1, &tex);
            if (tex == 0) {
                qWarning() << "Failed to generate texture";
                continue;
            }

            gl->glBindTexture(GL_TEXTURE_2D, tex);
            gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tight.constData());

            auto it = tilesHash_.find(idx);
            if (it != tilesHash_.end()) {
                it->second.setTextureId(tex);
            }
            else {
                map::Tile tile;
                tile.setTextureId(tex);
                tilesHash_.emplace(idx, std::move(tile));
            }
        }
        pendingInit_.clear();
    }

    // refresh
    if (!pendingUpdate_.isEmpty()) {
        for (auto& t : pendingUpdate_) {
            auto it = tilesHash_.find(t.idx);

            QByteArray tight;
            int w = 0, h = 0;
            if (!toTightRGBA8888(t.img, tight, w, h)) {
                continue;
            }

            if (it != tilesHash_.end() && it->second.getTextureId()) {
                gl->glBindTexture(GL_TEXTURE_2D, it->second.getTextureId());
                gl->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, tight.constData());
            }
            else {
                // texture does not exist
                GLuint tex = 0;
                gl->glGenTextures(1, &tex);
                if (tex == 0) {
                    qWarning() << "Failed to generate texture";
                    continue;
                }

                gl->glBindTexture(GL_TEXTURE_2D, tex);
                gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tight.constData());

                if (it != tilesHash_.end()) {
                    it->second.setTextureId(tex);
                }
                else {
                    map::Tile tile; tile.setTextureId(tex);
                    tilesHash_.emplace(t.idx, std::move(tile));
                }
            }
        }
        pendingUpdate_.clear();
    }
}

void MapView::MapViewRenderImplementation::ensureQuadBuffers(QOpenGLFunctions *gl) const
{
    if (!vboUV_) {
        gl->glGenBuffers(1, &vboUV_);
        gl->glBindBuffer(GL_ARRAY_BUFFER, vboUV_);
        // 固定 UV 正方形
        static const GLfloat kUV[8] = { 0,0, 1,0, 1,1, 0,1 };
        gl->glBufferData(GL_ARRAY_BUFFER, sizeof(kUV), kUV, GL_STATIC_DRAW);
        gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    if (!vboPos_) {
        gl->glGenBuffers(1, &vboPos_);
    }
}

// void MapView::MapViewRenderImplementation::render(QOpenGLFunctions *ctx,
//                         const QMatrix4x4 &model, const QMatrix4x4 &view,  const QMatrix4x4 &projection,
//                         const QMap<QString, std::shared_ptr<QOpenGLShaderProgram>> &shaderProgramMap) const
// {
//     if (!m_isVisible || !ctx) {
//         return;
//     }

//     processPendingTextureTasks(ctx);

//     if (!tilesHash_.empty()) {
//         auto shaderProgram = shaderProgramMap.value("image", nullptr);
//         if (!shaderProgram) {
//             qWarning() << "Shader program 'image' not found!";
//             return;
//         }

//         ensureQuadBuffers(ctx);

//         shaderProgram->bind();
//         shaderProgram->setUniformValue("mvp", projection * view * model);

//         const int posLoc = shaderProgram->attributeLocation("position");
//         const int texLoc = shaderProgram->attributeLocation("texCoord");
//         if (posLoc < 0 || texLoc < 0) {
//             shaderProgram->release();
//             return;
//         }

//         ctx->glBindBuffer(GL_ARRAY_BUFFER, vboUV_);
//         ctx->glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const void*>(0));
//         ctx->glEnableVertexAttribArray(texLoc);



//         // 获取多边形轮廓
//         QVector<QVector3D> polygonOutline;
//         bool hasPolygon = false;

//         if (mapView_) {
//             auto graphicsScene3dView = qobject_cast<GraphicsScene3dView*>(mapView_->parent());
//             if (graphicsScene3dView) {
//                 auto dataset = graphicsScene3dView->dataset();
//                 if (dataset) {
//                     const QVector<North_East_Down>& polygonOutlineNed = dataset->getPolygonOutlineNED();
//                     for(const auto& ned : polygonOutlineNed) {
//                         polygonOutline.append(QVector3D(ned.n, ned.e, 0.0f));
//                     }
//                     hasPolygon = !polygonOutline.isEmpty();
//                 }
//             }
//         }

//         for (auto& [tileIndx, tile] : tilesHash_) {
//             // 检查瓦片是否在多边形轮廓内
//             if (hasPolygon) {
//                 const auto& verts = tile.getVerticesRef();
//                 if (verts.size() >= 4) {
//                     // 检查瓦片的中心点是否在多边形轮廓内
//                     QVector3D center = (verts[0] + verts[1] + verts[2] + verts[3]) / 4.0f;
//                     if (isPointInPolygon(center, polygonOutline)) {
//                         // 瓦片在多边形轮廓内，不渲染
//                         continue;
//                     }
//                 }
//             }

//             const GLuint tex = tile.getTextureId();
//             if (!tex) {
//                 continue;
//             }

//             const auto& verts = tile.getVerticesRef();
//             if (verts.size() < 4) {
//                 continue;
//             }

//             GLfloat pos[12] = {
//                 verts[0].x(), verts[0].y(), verts[0].z(),
//                 verts[1].x(), verts[1].y(), verts[1].z(),
//                 verts[2].x(), verts[2].y(), verts[2].z(),
//                 verts[3].x(), verts[3].y(), verts[3].z()
//             };

//             ctx->glBindBuffer(GL_ARRAY_BUFFER, vboPos_);
//             ctx->glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STREAM_DRAW);
//             ctx->glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const void*>(0));
//             ctx->glEnableVertexAttribArray(posLoc);

//             ctx->glActiveTexture(GL_TEXTURE0);
//             ctx->glBindTexture(GL_TEXTURE_2D, tex);
//             shaderProgram->setUniformValue("imageTexture", 0);

//             ctx->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

//             ctx->glDisableVertexAttribArray(posLoc);
//         }

//         ctx->glDisableVertexAttribArray(texLoc);
//         ctx->glBindBuffer(GL_ARRAY_BUFFER, 0);
//         shaderProgram->release();
//     }
// }



void MapView::MapViewRenderImplementation::render(QOpenGLFunctions *ctx,
        const QMatrix4x4 &model, const QMatrix4x4 &view,  const QMatrix4x4 &projection,
        const QMap<QString, std::shared_ptr<QOpenGLShaderProgram>> &shaderProgramMap) const
{
    if (!m_isVisible || !ctx) {
        return;
    }

    processPendingTextureTasks(ctx);

    if (!tilesHash_.empty()) {
        auto shaderProgram = shaderProgramMap.value("image", nullptr);
        if (!shaderProgram) {
            qWarning() << "Shader program 'image' not found!";
            return;
        }

        ensureQuadBuffers(ctx);

        // 获取多边形轮廓
        QVector<QVector3D> polygonOutline;
        bool hasPolygon = false;

        if (mapView_) {
            auto graphicsScene3dView = qobject_cast<GraphicsScene3dView*>(mapView_->parent());
            if (graphicsScene3dView) {
                auto dataset = graphicsScene3dView->dataset();
                if (dataset) {
                    const QVector<North_East_Down>& polygonOutlineNed = dataset->getPolygonOutlineNED();
                    for(const auto& ned : polygonOutlineNed) {
                        polygonOutline.append(QVector3D(ned.n, ned.e, 0.0f));
                    }
                    hasPolygon = !polygonOutline.isEmpty();
                }
            }
        }


        // 启用模板测试
        if (hasPolygon) {
            // 禁用深度测试和混合
            ctx->glDisable(GL_DEPTH_TEST);
            ctx->glDisable(GL_BLEND);

            // 启用模板测试  镂空模版
            ctx->glEnable(GL_STENCIL_TEST);
            ctx->glClearStencil(0);
            ctx->glClear(GL_STENCIL_BUFFER_BIT);
            ctx->glStencilFunc(GL_ALWAYS, 1, 0xFF);
            ctx->glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

            // 绘制多边形轮廓到模板缓冲区
            auto staticShaderProgram = shaderProgramMap.value("static", nullptr);
            if (staticShaderProgram) {
                staticShaderProgram->bind();
                // staticShaderProgram->setUniformValue("matrix", projection * view * model);
                // staticShaderProgram->setUniformValue("color", QVector3D(0, 0, 0));

                const int posLoc = staticShaderProgram->attributeLocation("position");
                if (posLoc >= 0) {
                    // 创建多边形顶点缓冲区
                    std::vector<GLfloat> polygonVerts(polygonOutline.size() * 3);
                    for (int i = 0; i < polygonOutline.size(); ++i) {
                        polygonVerts[i * 3]     = polygonOutline[i].x();
                        polygonVerts[i * 3 + 1] = polygonOutline[i].y();
                        polygonVerts[i * 3 + 2] = polygonOutline[i].z();
                    }

                    GLuint polygonVbo;
                    ctx->glGenBuffers(1, &polygonVbo);
                    ctx->glBindBuffer(GL_ARRAY_BUFFER, polygonVbo);
                    ctx->glBufferData(GL_ARRAY_BUFFER, polygonVerts.size() * sizeof(GLfloat), polygonVerts.data(), GL_STATIC_DRAW);
                    ctx->glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const void*>(0));
                    ctx->glEnableVertexAttribArray(posLoc);
                }
                staticShaderProgram->release();
            }

            // 设置模板测试条件：只渲染模板值为0的像素（即多边形外的区域）
            ctx->glStencilFunc(GL_EQUAL, 0, 0xFF);
            ctx->glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        }


        shaderProgram->bind();
        shaderProgram->setUniformValue("mvp", projection * view * model);

        const int posLoc = shaderProgram->attributeLocation("position");
        const int texLoc = shaderProgram->attributeLocation("texCoord");
        if (posLoc < 0 || texLoc < 0) {
            shaderProgram->release();
            return;
        }

        ctx->glBindBuffer(GL_ARRAY_BUFFER, vboUV_);
        ctx->glVertexAttribPointer(texLoc, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const void*>(0));
        ctx->glEnableVertexAttribArray(texLoc);

        for (auto& [tileIndx, tile] : tilesHash_) {
            const GLuint tex = tile.getTextureId();
            if (!tex) {
                continue;
            }

            const auto& verts = tile.getVerticesRef();
            if (verts.size() < 4) {
                continue;
            }

            GLfloat pos[12] = {
                verts[0].x(), verts[0].y(), verts[0].z(),
                verts[1].x(), verts[1].y(), verts[1].z(),
                verts[2].x(), verts[2].y(), verts[2].z(),
                verts[3].x(), verts[3].y(), verts[3].z()
            };

            ctx->glBindBuffer(GL_ARRAY_BUFFER, vboPos_);
            ctx->glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STREAM_DRAW);
            ctx->glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const void*>(0));
            ctx->glEnableVertexAttribArray(posLoc);

            ctx->glActiveTexture(GL_TEXTURE0);
            ctx->glBindTexture(GL_TEXTURE_2D, tex);
            shaderProgram->setUniformValue("imageTexture", 0);

            ctx->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            ctx->glDisableVertexAttribArray(posLoc);
        }

        ctx->glDisableVertexAttribArray(texLoc);
        ctx->glBindBuffer(GL_ARRAY_BUFFER, 0);
        shaderProgram->release();

        // 禁用模板测试
        if (hasPolygon) {
            ctx->glDisable(GL_STENCIL_TEST);
        }
    }
}
