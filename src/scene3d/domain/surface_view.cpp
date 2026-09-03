#include "surface_view.h"


#include <QFile>


SurfaceView::SurfaceView(QObject* parent)
    : SceneObject(new SurfaceViewRenderImplementation, parent),
    mosaicColorTableToDelete_(0), surfaceColorTableToDelete_(0)
{}


// void SurfaceView::setBoundaryVertices(const QVector<QVector3D>& vertices)
// {
//     if (auto* r = RENDER_IMPL(SurfaceView); r) {
//         r->boundaryVertices_ = vertices;
//         Q_EMIT changed();
//     }
// }

void SurfaceView::setBoundaryVerticesVisible(bool visible)
{
    if (auto* r = RENDER_IMPL(SurfaceView); r) {
        r->boundaryVerticesVisible_ = visible;
        Q_EMIT changed();
    }
}

void SurfaceView::setVerticalScale(float scale)
{
    if (auto* r = RENDER_IMPL(SurfaceView); r) {
        if (!qFuzzyCompare(r->verticalScale_, scale)) {
            r->verticalScale_ = scale;
            Q_EMIT changed();
        }
    }
}

void SurfaceView::setGroundVisible(bool visible)
{
    if(auto* r = RENDER_IMPL(SurfaceView); r) {
        if(r->groundVisible_ != visible) {
            r->groundVisible_ = visible;
            Q_EMIT changed();
        }
    }
}

QRectF SurfaceView::getSurfaceBounds() const
{
    auto r = RENDER_IMPL(SurfaceView);
    if(!r){
        return QRectF();
    }
    return r->getSurfaceBounds();
}

SurfaceView::~SurfaceView()
{
    auto* r = RENDER_IMPL(SurfaceView);
    if (!r) {
        return;
    }
    const auto& rTiles = r->tiles_;
    for (const auto& itm : rTiles) {
        mosaicTileTextureToDelete_.append(itm.getMosaicTextureId());
    }
    mosaicColorTableToDelete_ = getMosaicColorTableTextureId();
    r->mosaicColorTableTextureId_ = 0;

    if (auto* r = RENDER_IMPL(SurfaceView); r) {
        surfaceColorTableToDelete_ = r->surfaceColorTableTextureId_;
    }
}

void SurfaceView::setMosaicTextureIdByTileId(QUuid tileId, GLuint textureId)
{
    if (auto* r = RENDER_IMPL(SurfaceView); r) {
        if (auto it = r->tiles_.find(tileId); it != r->tiles_.end()) {
            if (it.value().getMosaicTextureId() != textureId) {
                it.value().setMosaicTextureId(textureId);
                Q_EMIT changed();
            }
        }
    }
}

void SurfaceView::setMosaicColorTableTextureId(GLuint value)
{
    if (auto* r = RENDER_IMPL(SurfaceView); r) {
        auto& cTTId = r->mosaicColorTableTextureId_;
        if (cTTId != value) {
            cTTId = value;
            Q_EMIT changed();
        }
    }
}

GLuint SurfaceView::getMosaicTextureIdByTileId(QUuid tileId) const
{
    GLuint retVal = 0;
    if (auto* r = RENDER_IMPL(SurfaceView); r) {
        if (auto it = r->tiles_.find(tileId); it != r->tiles_.end()) {
            retVal =  it.value().getMosaicTextureId();
        }
    }

    return retVal;
}

GLuint SurfaceView::getMosaicColorTableTextureId() const
{
    if (auto* r = RENDER_IMPL(SurfaceView); r) {
        return r->mosaicColorTableTextureId_;
    }
    return 0;
}

QVector<GLuint> SurfaceView::takeMosaicTileTextureToDelete()
{
    QMutexLocker lock(&mosaicTexTasksMutex_);

    std::sort(mosaicTileTextureToDelete_.begin(), mosaicTileTextureToDelete_.end());
    mosaicTileTextureToDelete_.erase(std::unique(mosaicTileTextureToDelete_.begin(), mosaicTileTextureToDelete_.end()), mosaicTileTextureToDelete_.end());

    QVector<GLuint> out;
    out.swap(mosaicTileTextureToDelete_);
    return out;
}

QVector<std::pair<QUuid, std::vector<uint8_t>>> SurfaceView::takeMosaicTileTextureToAppend()
{
    QMutexLocker lock(&mosaicTexTasksMutex_);

    QVector<std::pair<QUuid, std::vector<uint8_t>>> out;
    out.reserve(mosaicTileTextureToAppend_.size());
    for (auto it = mosaicTileTextureToAppend_.cbegin(); it != mosaicTileTextureToAppend_.cend(); ++it) {
        out.push_back({ it.key(), it.value() });
    }
    mosaicTileTextureToAppend_.clear();
    return out;
}

std::vector<uint8_t> SurfaceView::takeMosaicColorTableToAppend()
{
    // auto retVal = std::move(mosaicColorTableToAppend_);
    auto retVal = std::vector<uint8_t>();
    return retVal;
}

GLuint SurfaceView::takeMosaicColorTableToDelete()
{
    GLuint retVal = 0;
    std::swap(mosaicColorTableToDelete_, retVal);
    return retVal;
}

std::vector<uint8_t> SurfaceView::takeSurfaceColorTableToAppend()
{
    auto retVal = std::move(surfaceColorTableToAppend_);
    return retVal;
}

GLuint SurfaceView::takeSurfaceColorTableToDelete()
{
    GLuint retVal = 0;
    std::swap(surfaceColorTableToDelete_, retVal);
    return retVal;
}

void SurfaceView::setLlaRef(LLARef llaRef)
{
    llaRef_ = llaRef;
}

void SurfaceView::saveVerticesToFile(const QString &path)
{
    auto* r = RENDER_IMPL(SurfaceView);
    if (!r) {
        qWarning() << "SurfaceView::saveVerticesToFile: no render impl";
        return;
    }

#ifdef Q_OS_ANDROID
    const QString filePath = path;
#else
    const QString filePath = QUrl(path).toLocalFile();
#endif

    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        qWarning() << "Failed to open file for writing:" << filePath;
        return;
    }

    QTextStream out(&file);
    out.setLocale(QLocale::c());
    out.setRealNumberNotation(QTextStream::FixedNotation);
    out.setRealNumberPrecision(8);
    out << "lat,lon,alt,x,y,z\n";

    for (auto it = r->tiles_.cbegin(); it != r->tiles_.cend(); ++it) {
        const SurfaceTile& tile = it.value();
        if (!tile.getIsInited())
            continue;

        const QVector<QVector3D>&  verts = tile.getHeightVerticesCRef();
        const QVector<HeightType>& marks = tile.heightMarkVertices_;

        if (verts.isEmpty() || marks.isEmpty())  continue;

        const QVector3D& v = verts[0]; // левая верхняя вершина (0,0) => idx = 0
        if (marks[0] == HeightType::kUndefined) {
            continue;
        }

        if (!qIsFinite(v.x()) || !qIsFinite(v.y()) || !qIsFinite(v.z())) {
            continue;
        }

        double lat = std::numeric_limits<double>::quiet_NaN();
        double lon = std::numeric_limits<double>::quiet_NaN();

        if (llaRef_.isInit) {
            North_East_Down ned(v.x(), v.y(), v.z());
            LLA lla(&ned, &llaRef_);
            if (qIsFinite(lla.latitude) && qIsFinite(lla.longitude)) {
                lat = lla.latitude;
                lon = lla.longitude;
            }
        }

        out << lat << "," << lon << "," << v.z() << "," << v.x() << "," << v.y() << "," << v.z() << "\n";
    }

    file.close();
}

GLuint SurfaceView::getSurfaceColorTableTextureId() const
{
    if (auto* r = RENDER_IMPL(SurfaceView); r) {
        return r->surfaceColorTableTextureId_;
    }

    return 0;
}

bool SurfaceView::getMVisible() const
{
    auto* r = RENDER_IMPL(SurfaceView);
    return r->mVis_;
}

bool SurfaceView::getIVisible() const
{
    auto* r = RENDER_IMPL(SurfaceView);
    return r->iVis_;
}

void SurfaceView::setSurfaceColorTableTextureId(GLuint textureId)
{
    if (auto* r = RENDER_IMPL(SurfaceView); r) {
        r->surfaceColorTableTextureId_ = textureId;
        Q_EMIT changed();
    }
}

void SurfaceView::setIVisible(bool state)
{
    auto* r = RENDER_IMPL(SurfaceView);
    r->iVis_ = state;
    Q_EMIT changed();
}

void SurfaceView::setMVisible(bool state)
{
    auto* r = RENDER_IMPL(SurfaceView);
    r->mVis_ = state;
    Q_EMIT changed();
}

void SurfaceView::clear()
{
    auto* r = RENDER_IMPL(SurfaceView);
    if (!r) {
        return;
    }

    const auto& rTiles = r->tiles_;
    for (const auto& itm : rTiles) {
        mosaicTileTextureToDelete_.append(itm.getMosaicTextureId());
    }

    r->tiles_.clear();

    {
        QMutexLocker lock(&mosaicTexTasksMutex_);
        mosaicTileTextureToAppend_.clear();
    }

    r->minZ_ = std::numeric_limits<float>::max();
    r->maxZ_ = std::numeric_limits<float>::lowest();
    r->colorIntervalsSize_ = -1;

    surfaceColorTableToAppend_.clear();

    Q_EMIT changed();
    Q_EMIT boundsChanged();
}

void SurfaceView::setTiles(const QHash<QUuid, SurfaceTile> &tiles, bool useTextures)
{
    // qDebug() << "SurfaceView::setTiles" << tiles.size();
    if (auto* r = RENDER_IMPL(SurfaceView); r) {
        auto& rTRef = r->tiles_;
        for (auto itT = tiles.cbegin(); itT != tiles.cend(); ++itT) {
            auto& iKey   = itT.key();
            auto& iValue = itT.value();
            if (auto itRT = rTRef.find(iKey); itRT != rTRef.end()) {
                auto& itRTVRef = itRT.value();
                const auto savedTexId = itRTVRef.textureId_;
                itRTVRef = std::move(iValue);
                itRTVRef.textureId_ = savedTexId;
            }
            else {
                rTRef.insert(iKey, iValue);
            }
        }

        if (useTextures) {
            updateMosaicTileTextureTask(tiles);
        }

        Q_EMIT changed();
    }
}

void SurfaceView::setMinZ(float minZ)
{
    if (auto* r = RENDER_IMPL(SurfaceView); r) {
        r->minZ_ = minZ;
        Q_EMIT changed();
    }
}

void SurfaceView::setMaxZ(float maxZ)
{
    // qDebug() << "SurfaceView::setMaxZ....." << maxZ;
    if (auto* r = RENDER_IMPL(SurfaceView); r) {
        r->maxZ_ = maxZ;
        Q_EMIT changed();
    }
}

void SurfaceView::setSurfaceStep(float surfaceStep)
{
    //qDebug() << "SurfaceView::setSurfaceStep" << levelStep;
    if (auto* r = RENDER_IMPL(SurfaceView); r) {
        r->surfaceStep_ = surfaceStep;
        Q_EMIT changed();
    }
}

void SurfaceView::setTextureTask(const std::vector<uint8_t> &textureTask)
{
    surfaceColorTableToAppend_ = textureTask;

    Q_EMIT changed();
}

void SurfaceView::setColorIntervalsSize(int size)
{
    if (auto* r = RENDER_IMPL(SurfaceView); r) {
        r->colorIntervalsSize_ = size;
        Q_EMIT changed();
    }
}

void SurfaceView::removeTiles(const QSet<QUuid> &ids)
{
    if (ids.isEmpty()) return;
    if (auto* r = RENDER_IMPL(SurfaceView); r) {
        for (const auto& id : ids) {
            auto it = r->tiles_.find(id);
            if (it != r->tiles_.end()) {
                if (auto oldId = it.value().getMosaicTextureId(); oldId != 0) {
                    mosaicTileTextureToDelete_.append(oldId);
                }
                r->tiles_.erase(it);
            }
        }
        Q_EMIT changed();
    }
}

void SurfaceView::updateMosaicTileTextureTask(const QHash<QUuid, SurfaceTile>& newTiles)
{
    if (newTiles.isEmpty()) {
        return;
    }
    auto* r = RENDER_IMPL(SurfaceView);
    if (!r) {
        return;
    }

    {
        QMutexLocker lock(&mosaicTexTasksMutex_);
        auto& rTiles = r->tiles_;

        for (auto it = newTiles.cbegin(); it != newTiles.cend(); ++it) {
            const auto& key   = it.key();
            const auto& value = it.value();

            if (auto rIt = rTiles.find(key); rIt != rTiles.end()) { // 仅更新
                auto& tile = rIt.value();
                tile.imageData_ = value.imageData_;
                mosaicTileTextureToAppend_.insert(key, tile.getMosaicImageDataCRef());
            }
            else {
                qWarning() << "SurfaceView: no tile for key" << key;
            }
        }
    }
}


SurfaceView::SurfaceViewRenderImplementation::SurfaceViewRenderImplementation() : surfaceColorTableTextureId_(0),
    mosaicColorTableTextureId_(0),
    minZ_(std::numeric_limits<float>::max()),
    maxZ_(std::numeric_limits<float>::lowest()),
    surfaceStep_(3.0f),
    colorIntervalsSize_(-1),
    iVis_(false),
    mVis_(false),
    boundaryVerticesVisible_(false)  // 初始化
{
#if defined(Q_OS_ANDROID) || defined(LINUX_ES)
    mosaicColorTableTextureType_ = GL_TEXTURE_2D;
#else
    mosaicColorTableTextureType_ = GL_TEXTURE_1D;
#endif
}

void SurfaceView::SurfaceViewRenderImplementation::render(QOpenGLFunctions *ctx,  const QMatrix4x4 &mvp,
                        const QMap<QString, std::shared_ptr<QOpenGLShaderProgram>> &shaderProgramMap) const
{
    if (!iVis_ && !mVis_) {
        return;
    }

    auto mShP = shaderProgramMap.value("mosaic", nullptr);
    auto iShP = shaderProgramMap.value("isobaths", nullptr);
    if (!mShP || !iShP) {
        qWarning() << "Shader program 'mosaic'|'isobaths' not found!";
        return;
    }

    auto sShP = shaderProgramMap.value("static", nullptr);
    if (!sShP) {
        qWarning() << "Shader program 'mosaic'|'isobaths'|'static' not found!";
        return;
    }

    // 使用模板缓冲区实现"盆地"效果：高度场区域从陆地地面中挖空
    QRectF bounds  = getSurfaceBounds();
    bool minZValid = qIsFinite(minZ_) && minZ_ < 1e6f && minZ_ > -1e6f;

    if (groundVisible_ && !bounds.isEmpty() && minZValid) {
        // === Pass 1: 将高度场几何体写入模板缓冲区（标记高度场区域为1） ===
        ctx->glEnable(GL_STENCIL_TEST);
        ctx->glClearStencil(0);
        ctx->glClear(GL_STENCIL_BUFFER_BIT);
        ctx->glStencilFunc(GL_ALWAYS, 1, 0xFF);
        ctx->glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        ctx->glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        ctx->glDepthMask(GL_FALSE);
        ctx->glDisable(GL_DEPTH_TEST);

        sShP->bind();
        sShP->setUniformValue("matrix", mvp);
        int posLoc = sShP->attributeLocation("position");

        sShP->enableAttributeArray(posLoc);
        for (auto& itm : tiles_) {
            if (!itm.getIsInited()) {
                continue;
            }
            sShP->setAttributeArray(posLoc, itm.getHeightVerticesCRef().constData());
            ctx->glDrawElements(GL_TRIANGLES, itm.getHeightIndicesCRef().size(),
                                GL_UNSIGNED_INT, itm.getHeightIndicesCRef().constData());
        }
        sShP->disableAttributeArray(posLoc);
        sShP->release();

        ctx->glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        ctx->glDepthMask(GL_TRUE);
        ctx->glEnable(GL_DEPTH_TEST);

        // === Pass 2: 绘制陆地地面（仅在模板值!=1的区域，即高度场外部） ===
        ctx->glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        ctx->glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        sShP->bind();
        sShP->setUniformValue("matrix", mvp);
        posLoc = sShP->attributeLocation("position");
        int colorLoc = sShP->uniformLocation("color");
        sShP->enableAttributeArray(posLoc);

        float zRange = qMax(0.1f, maxZ_ - minZ_);
        float groundZBase = minZ_ - zRange * 0.05f - 0.05f;
        // 补偿surfaceModel的Z平移+缩放，使水平陆地经变换后Z位置恒定不随垂直缩放变化
        float groundZ = qFuzzyIsNull(verticalScale_) ? groundZBase : (-groundZBase / verticalScale_
                                        + minZ_ * (1.0f + verticalScale_));

        float minX = static_cast<float>(bounds.left());
        float minY = static_cast<float>(bounds.top());
        float maxX = static_cast<float>(bounds.right());
        float maxY = static_cast<float>(bounds.bottom());

        QVector<QVector3D> groundPlane{
            { minX, minY, groundZ },
            { maxX, minY, groundZ },
            { minX, maxY, groundZ },
            { maxX, maxY, groundZ }
        };

        sShP->setUniformValue(colorLoc, QVector4D(0.82f, 0.75f, 0.55f, 1.0f));
        sShP->setAttributeArray(posLoc, groundPlane.constData());
        ctx->glDrawArrays(GL_TRIANGLE_STRIP, 0, groundPlane.size());

        sShP->disableAttributeArray(posLoc);
        sShP->release();

        ctx->glDisable(GL_STENCIL_TEST);
    }

    for (auto& itm : tiles_) {
        if (!itm.getIsInited()) {
            continue;
        }

        GLuint textureId = itm.getMosaicTextureId();

        if (mVis_) {
            auto& shP = mShP;
            shP->bind();
            shP->setUniformValue("mvp", mvp);

            int positionLoc = shP->attributeLocation("position");
            int texCoordLoc = shP->attributeLocation("texCoord");

            shP->enableAttributeArray(positionLoc);
            shP->enableAttributeArray(texCoordLoc);

            shP->setAttributeArray(positionLoc, itm.getHeightVerticesCRef().constData());
            shP->setAttributeArray(texCoordLoc, itm.getMosaicTextureVerticesCRef().constData());

            QOpenGLFunctions* glFuncs = QOpenGLContext::currentContext()->functions();

            glFuncs->glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureId);
            shP->setUniformValue("indexedTexture", 0);

            glFuncs->glActiveTexture(GL_TEXTURE1);
            glBindTexture(mosaicColorTableTextureType_, mosaicColorTableTextureId_);
            shP->setUniformValue("colorTable", 1);

            ctx->glDrawElements(GL_TRIANGLES, itm.getHeightIndicesCRef().size(),
                                GL_UNSIGNED_INT, itm.getHeightIndicesCRef().constData());

            shP->disableAttributeArray(texCoordLoc);
            shP->disableAttributeArray(positionLoc);

            shP->release();
        }
        else if (iVis_) {
            auto &shP = iShP;
            shP->bind();

            shP->setUniformValue("matrix",     mvp);
            shP->setUniformValue("depthMin",   minZ_);
            shP->setUniformValue("levelStep",  surfaceStep_);
            shP->setUniformValue("levelCount", colorIntervalsSize_);
            shP->setUniformValue("linePass",   false);   // 无线条，无标签

            ctx->glActiveTexture(GL_TEXTURE0);
            ctx->glBindTexture(GL_TEXTURE_2D, surfaceColorTableTextureId_);
            shP->setUniformValue("paletteSampler", 0);

            const int posLoc = shP->attributeLocation("position");
            shP->enableAttributeArray(posLoc);
            shP->setAttributeArray(posLoc, itm.getHeightVerticesCRef().constData());

            ctx->glDrawElements(GL_TRIANGLES, itm.getHeightIndicesCRef().size(),
                                GL_UNSIGNED_INT,itm.getHeightIndicesCRef().constData());

            shP->disableAttributeArray(posLoc);
            shP->release();
        }
    }

    // ======= 渲染边界顶点 =======
    // renderBoundaryVertices(ctx, mvp,shaderProgramMap);
}

// void SurfaceView::SurfaceViewRenderImplementation::renderBoundaryVertices(
//                             QOpenGLFunctions* ctx, const QMatrix4x4& mvp,
//                             const QMap<QString, std::shared_ptr<QOpenGLShaderProgram>>& shaderProgramMap) const
// {
//     if (boundaryVertices_.isEmpty()) {
//         qDebug() << "renderBoundaryVertices: no boundary vertices";
//         return;
//     }

//     qDebug() << "renderBoundaryVertices: vertex count =" << boundaryVertices_.size();

//     // 获取shader程序
//     auto shaderProgram = shaderProgramMap.value("static", nullptr);
//     if (!shaderProgram) {
//         qDebug() << "renderBoundaryVertices: shader program is null";
//         return;
//     }

//     // 绑定shader
//     if (!shaderProgram->bind()) {
//         qDebug() << "renderBoundaryVertices: failed to bind shader";
//         return;
//     }

//     // 获取属性位置
//     int posLoc    = shaderProgram->attributeLocation("position");
//     int colorLoc  = shaderProgram->uniformLocation("color");
//     int matrixLoc = shaderProgram->uniformLocation("matrix");

//     qDebug() << "renderBoundaryVertices: posLoc =" << posLoc
//              << ", colorLoc =" << colorLoc
//              << ", matrixLoc =" << matrixLoc;

//     shaderProgram->setUniformValue(matrixLoc, mvp);

//     // 禁用深度测试，启用混合
//     ctx->glDisable(GL_DEPTH_TEST);
//     ctx->glEnable(GL_BLEND);
//     ctx->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//     // 使用 setAttributeArray 批量设置顶点
//     // glPointSize(15.0f);  // 增大点大小

//     // 设置点颜色（红色）
//     QVector4D pointColor(1.0f, 0.0f, 0.0f, 1.0f);
//     shaderProgram->setUniformValue(colorLoc, pointColor);

//     // 启用顶点属性数组并设置数据
//     shaderProgram->enableAttributeArray(posLoc);
//     shaderProgram->setAttributeArray(posLoc, boundaryVertices_.constData());

//     // 一次性绘制所有点
//     ctx->glDrawArrays(GL_POINTS, 0, boundaryVertices_.size());

//     shaderProgram->disableAttributeArray(posLoc);  // 禁用属性数组

//     qDebug() << "renderBoundaryVertices: drawn" << boundaryVertices_.size() << "points";

//     shaderProgram->release();
//     ctx->glDisable(GL_BLEND);
//     ctx->glEnable(GL_DEPTH_TEST);
// }


float SurfaceView::SurfaceViewRenderImplementation::getMaxZ()
{
    return maxZ_;
}

float SurfaceView::SurfaceViewRenderImplementation::getMinZ()
{
    return minZ_;
}

QRectF SurfaceView::SurfaceViewRenderImplementation::getSurfaceBounds() const
{
    if (tiles_.isEmpty()) {
        return QRectF();
    }

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();

    for (auto tile : tiles_) {
        if (!tile.getIsInited()) {
            continue;
        }

        auto& vertices = tile.getHeightVerticesRef();
        for (const auto& vert : vertices) {
            minX = std::min(minX, vert.x());
            maxX = std::max(maxX, vert.x());
            minY = std::min(minY, vert.y());
            maxY = std::max(maxY, vert.y());
        }
    }

    return QRectF(minX, minY, maxX - minX, maxY - minY);
}
