#include "polygon_outline.h"
#include "scene3d_view.h"


PolygonOutline::PolygonOutline(GraphicsScene3dView* view, QObject* parent) :
    SceneObject(new PolygonOutlineRenderImplementation, view, parent),
    datasetPtr_(nullptr)
{
    setPrimitiveType(GL_LINE_LOOP);

    // setColor({0, 0, 205});
    setColor({255, 64, 64});
    setWidth(4.0f);
}

PolygonOutline::~PolygonOutline()
{

}

void PolygonOutline::setQmlRootObject(QObject* object)
{
    qmlRootObject_ = object;
}

void PolygonOutline::setOutlineMode(bool outline)
{
    isDrawOutlineMode_ = outline;
}

bool PolygonOutline::getOutlineMode() const
{
    return isDrawOutlineMode_;
}

void PolygonOutline::setDraggingPoint(bool dragging)
{
    isDraggingPoint_ = dragging;
    selectVertex(draggingPtIndex_, dragging);
}

bool PolygonOutline::getDraggingPoint()
{
    return isDraggingPoint_;
}

SceneObject::SceneObjectType PolygonOutline::type() const
{
    return SceneObject::SceneObjectType::PolygonOutline;
}

void PolygonOutline::setDatasetPtr(Dataset* datasetPtr)
{
    datasetPtr_ = datasetPtr;
    if(datasetPtr_ == nullptr) {
        return;
    }
}

void PolygonOutline::polygonAddPoint(double latitude, double longitude)
{
    Epoch* lastEp = datasetPtr_->lastPolygonOutline();
    if (!lastEp) {
        return;
    }

    Position pos;
    pos.lla = LLA(latitude, longitude);
    if (pos.lla.isCoordinatesValid()) {
        if (lastEp->getPositionGNSS().lla.isCoordinatesValid()) {
            lastEp = datasetPtr_->addNewEpochPolygonOutline();  //不断累加帧数的下标index
            // lastEp->setDistProcesing_CSV(depth);
        }
        LLARef llaRef = datasetPtr_->getLlaRef();
        lastEp->setPositionLLA(pos);
        lastEp->setPositionRef(&llaRef); //在这里将LLA坐标转化成本地North_East_Down坐标
    }
    QVector<Epoch> polygonOutline = datasetPtr_->getPolygonOutline();
    const int toIndx = polygonOutline.size() - 1;
    Epoch* epochPtr = datasetPtr_->fromPolygonOutlineIndex(toIndx);
    if (!epochPtr) {
        return;
    }

    const Position boatPos = epochPtr->getPositionGNSS();
    if (!boatPos.ned.isCoordinatesValid()) {
        return;
    }

    const int fromIndx = lastIndx_;
    if (fromIndx >= toIndx) {
        qDebug() << "fromIndx >= toIndx......fromIndx:" << fromIndx << "  lastIndex_:" << lastIndx_;
        return;
    }

    const int need = toIndx - fromIndx;
    QVector<QVector3D> prepData;
    prepData.reserve(need);

    for (int i = fromIndx + 1; i <= toIndx; ++i) {
        if (Epoch* ep = datasetPtr_->fromPolygonOutlineIndex(i); ep) {
            if (North_East_Down posNed = ep->getPositionGNSS().ned; posNed.isCoordinatesValid()) {
                prepData.push_back(QVector3D(posNed.n, posNed.e, 0));
                datasetPtr_->addPolygonOutlineNED(posNed);
            }
        }
    }

    lastIndx_ = toIndx;

    SceneObject::appendData(prepData);
}

void PolygonOutline::modifyPolygonVertex(int index, LLA lla)
{
    LLARef llaRef = datasetPtr_->getLlaRef();
    North_East_Down ned = North_East_Down(&lla, &llaRef);
    datasetPtr_->modifyPolygonOutline(index, ned);

    // 更新 Epoch 对象中的位置信息
    Epoch* epochPtr = datasetPtr_->fromPolygonOutlineIndex(index);
    if (epochPtr) {
        Position boatPos = epochPtr->getPositionGNSS();
        boatPos.lla = lla;
        boatPos.ned = ned;
        epochPtr->setPositionLLA(boatPos);  //写回Epoch对象
        datasetPtr_->modifyPolygonOutlineEpoch(index, *epochPtr);
    }

    // 更新 SceneObject 中的渲染数据
    QVector<QVector3D> data = this->data();
    if (index < data.size()) {
        data[index] = QVector3D(ned.n, ned.e, 0.0f);
        this->setData(data);
    }

    SceneObject::setData(data, GL_LINE_LOOP);
}

int PolygonOutline::getNearestVertexIndex(LLA lla) const
{
    QVector<North_East_Down> nedVec = datasetPtr_->getPolygonOutlineNED();
    if (nedVec.isEmpty()) return -1;

    LLARef llaRef = datasetPtr_->getLlaRef();
    North_East_Down ned = North_East_Down(&lla, &llaRef);
    QVector3D point = QVector3D(ned.n, ned.e, 0.0);

    const float threshold = 2.0;
    int nearestIndex = -1;

    for (int i = 0; i < nedVec.size(); i++) {
        QVector3D vertexLocal = QVector3D(nedVec.at(i).n, nedVec.at(i).e, 0.0f);
        float distance = (vertexLocal - point).length();
        if (distance < threshold) {
            nearestIndex = i;
        }
    }

    return nearestIndex;
}


void PolygonOutline::clearData()
{
    SceneObject::clearData();

    isDrawOutlineMode_ = false;
    isDraggingPoint_ = false;

    draggingPtIndex_ = -1;
    lastIndx_ = -1;
}

void PolygonOutline::selectVertex(int index, bool isDraggingPt)
{
    draggingPtIndex_ = index;
    // 同步到渲染实现
    auto renderImpl = dynamic_cast<PolygonOutlineRenderImplementation*>(m_renderImpl);
    if (renderImpl) {
        renderImpl->draggingPtIndex_ = index;
        renderImpl->isDraggingPoint_ = isDraggingPoint_;
    }
    emit changed();
}

void PolygonOutline::setFatherVisible(bool visible)
{
    isVisible_ = visible;
}

bool PolygonOutline::getFatherVisible()
{
    return isVisible_;
}

void PolygonOutline::autoGenerateBoundary(bool generate)
{
    if (!datasetPtr_) return;
    datasetPtr_->resetPolygonOutline();
    clearData();

    if(generate) {
        QVector<QVector3D>& boundary = datasetPtr_->getAutoBounadry();
        for (const auto& pt : boundary) {
            QVector<QVector3D> prepData;
            North_East_Down posNed = North_East_Down(pt.y(), pt.x(), 0);
            prepData.push_back(QVector3D(posNed.n, posNed.e, 0));
            SceneObject::appendData(prepData);
        }

        Q_EMIT changed();
    }
}


PolygonOutline::PolygonOutlineRenderImplementation::PolygonOutlineRenderImplementation()
{
}

PolygonOutline::PolygonOutlineRenderImplementation::~PolygonOutlineRenderImplementation()
{
}

void PolygonOutline::PolygonOutlineRenderImplementation::render(QOpenGLFunctions* ctx,  const QMatrix4x4& mvp,
                      const QMap<QString, std::shared_ptr<QOpenGLShaderProgram>>& shaderProgramMap) const
{
    // qDebug() << "PolygonOutline::PolygonOutlineRenderImplementa...........";
    if (!m_isVisible || data().empty()) {
        return;
    }

    auto shaderProgram = shaderProgramMap["static"].get();
    if (!shaderProgram->bind()) {
        return;
    }

    int posLoc    = shaderProgram->attributeLocation("position");
    int colorLoc  = shaderProgram->uniformLocation("color");
    int matrixLoc = shaderProgram->uniformLocation("matrix");

    shaderProgram->setUniformValue(colorLoc, DrawUtils::colorToVector4d(color()));
    shaderProgram->setUniformValue(matrixLoc, mvp);
    shaderProgram->enableAttributeArray(posLoc);
    shaderProgram->setAttributeArray(posLoc, data().constData());

    ctx->glLineWidth(width());
    ctx->glDrawArrays(GL_LINE_LOOP, 0, data().size());
    ctx->glLineWidth(1.0f);

    // 为每个顶点绘制实心圆
    const float radius = 1.6f; // 圆的半径
    const int segments = 16; // 圆的分段数
    // 生成圆的顶点数据
    QVector<QVector3D> circleVertices;
    circleVertices.reserve(segments + 2);

    // 绘制每个顶点的圆
    for (int i = 0; i < data().size(); i++) {
        const QVector3D& vertex = data().at(i);

        QVector4D vertexColor;
        if (i == draggingPtIndex_) {
            vertexColor = isDraggingPoint_ ? QVector4D(0.0f, 1.0f, 0.0f, 1.0f) : QVector4D(1.0f, 0.0f, 1.0f, 1.0f);
        } else {
            vertexColor = QVector4D(1.0f, 0.0f, 1.0f, 1.0f);
        }

        shaderProgram->setUniformValue(colorLoc, vertexColor);
        shaderProgram->setUniformValue(matrixLoc, mvp);

        circleVertices.clear();
        circleVertices.append(vertex);

        // 添加圆周上的点
        for (int j = 0; j <= segments; j++) {
            float angle = 2.0f * M_PI * j / segments;
            float x = vertex.x() + radius * std::cos(angle);
            float y = vertex.y() + radius * std::sin(angle);
            circleVertices.append(QVector3D(x, y, vertex.z()));
        }

        shaderProgram->setAttributeArray(posLoc, circleVertices.constData());
        ctx->glDrawArrays(GL_TRIANGLE_FAN, 0, circleVertices.size());
    }

    shaderProgram->disableAttributeArray(posLoc);
    shaderProgram->release();
}

void PolygonOutline::PolygonOutlineRenderImplementation::render(QOpenGLFunctions* ctx,
                    const QMatrix4x4& model, const QMatrix4x4& view,  const QMatrix4x4& projection,
                    const QMap<QString, std::shared_ptr<QOpenGLShaderProgram>>& shaderProgramMap) const
{
    SceneObject::RenderImplementation::render(ctx, model, view, projection, shaderProgramMap);
}
