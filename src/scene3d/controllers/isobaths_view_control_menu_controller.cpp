#include "isobaths_view_control_menu_controller.h"
#include "scene3d_view.h"


IsobathsViewControlMenuController::IsobathsViewControlMenuController(QObject* parent)
    : QmlComponentController(parent),
    graphicsSceneViewPtr_(nullptr),
    dataProcessorPtr_(nullptr),
    pendingLambda_(nullptr),
    themeId_(0),
    edgeLimit_(100),
    extraWidth_(0),
    visibility_(false),
    edgesVisible_(false),
    trianglesVisible_(false),
    debugModeView_(false),
    processState_(true)
{
    qRegisterMetaType<DataProcessorType>("DataProcessorType");
}

void IsobathsViewControlMenuController::setGraphicsSceneView(GraphicsScene3dView* sceneView)
{
    graphicsSceneViewPtr_ = sceneView;

    if (graphicsSceneViewPtr_) {
        if (pendingLambda_) {
            pendingLambda_();
            pendingLambda_ = nullptr;
        }
    }
}

void IsobathsViewControlMenuController::setDataProcessorPtr(DataProcessor *dataProcessorPtr)
{
    dataProcessorPtr_ = dataProcessorPtr;
}

void IsobathsViewControlMenuController::findComponent()
{
    m_component = m_engine->findChild<QObject*>("activeObjectParamsMenuLoader");
}

void IsobathsViewControlMenuController::tryInitPendingLambda()
{
    // qDebug() << "tryInitPendingLambda.............";
    if (!pendingLambda_) {
        pendingLambda_ = [this] () -> void {
            if (graphicsSceneViewPtr_) {
                if (dataProcessorPtr_) {
                    QMetaObject::invokeMethod(dataProcessorPtr_, "setUpdateIsobaths",               Qt::QueuedConnection, Q_ARG(bool,  processState_));
                    QMetaObject::invokeMethod(dataProcessorPtr_, "setSurfaceIsobathsLevelCnt",      Qt::QueuedConnection, Q_ARG(int, levelCnt_));
                    QMetaObject::invokeMethod(dataProcessorPtr_, "setSurfaceColorTableThemeById",   Qt::QueuedConnection, Q_ARG(int,   themeId_));
                    QMetaObject::invokeMethod(dataProcessorPtr_, "setSurfaceEdgeLimit",             Qt::QueuedConnection, Q_ARG(int,   edgeLimit_));
                    QMetaObject::invokeMethod(dataProcessorPtr_, "setExtraWidth",                   Qt::QueuedConnection, Q_ARG(int,   extraWidth_));
                }

                if (auto surfacePtr = graphicsSceneViewPtr_->getSurfaceViewPtr(); surfacePtr) {
                    surfacePtr->setIVisible(visibility_);
                }

                if (auto isobathsViewPtr = graphicsSceneViewPtr_->getIsobathsViewPtr(); isobathsViewPtr) {
                    isobathsViewPtr->setVisible(visibility_);
                }
            }
        };
    }
}


void IsobathsViewControlMenuController::onContoursVisibilityCheckBoxCheckedChanged(bool checked)
{
    if (graphicsSceneViewPtr_) {
        graphicsSceneViewPtr_->getSurfaceViewPtr()->setIVisible(checked);
    }
    else {
        tryInitPendingLambda();
    }
}

void IsobathsViewControlMenuController::onVertexVisibilityCheckBoxCheckedChanged(bool checked)
{
    if (graphicsSceneViewPtr_) {
        graphicsSceneViewPtr_->getSurfaceViewPtr()->setBoundaryVerticesVisible(checked);
    }
    else {
        tryInitPendingLambda();
    }
}

void IsobathsViewControlMenuController::onIsobathsVisibilityCheckBoxCheckedChanged(bool checked)
{
    // qDebug() << "onIsobathsVisibilityCheckBoxCheckedChanged " << checked;
    visibility_ = checked;

    if (graphicsSceneViewPtr_) {
        graphicsSceneViewPtr_->getIsobathsViewPtr()->setVisible(checked);

        if (visibility_) {
            if (dataProcessorPtr_) {
                if (checked) {
                    // QMetaObject::invokeMethod(dataProcessorPtr_, "clearProcessing", Qt::QueuedConnection, Q_ARG(DataProcessorType, DataProcessorType::kSurface));
                    // QMetaObject::invokeMethod(dataProcessorPtr_, "clearProcessing", Qt::QueuedConnection, Q_ARG(DataProcessorType, DataProcessorType::kIsobaths));
                    // QMetaObject::invokeMethod(dataProcessorPtr_, "onIsobathsUpdated", Qt::QueuedConnection);
                }
            }
        }
    }
    else {
        tryInitPendingLambda();
    }
}

void IsobathsViewControlMenuController::onOutlineVisibleChanged(bool visible)
{
    if (graphicsSceneViewPtr_) {
        graphicsSceneViewPtr_->polygonOutline()->setVisible(visible);
        graphicsSceneViewPtr_->polygonOutline()->setFatherVisible(visible);
    }
}

void IsobathsViewControlMenuController::onGroundVisibleChanged(bool visible)
{
    if(graphicsSceneViewPtr_) {
        if(auto surfacePtr = graphicsSceneViewPtr_->getSurfaceViewPtr(); surfacePtr) {
            surfacePtr->setGroundVisible(visible);
        }
    }
}

void IsobathsViewControlMenuController::onUpdateIsobathsButtonClicked()
{
    //if (graphicsSceneViewPtr_) {
    //    if (dataProcessorPtr_) {
    //        QMetaObject::invokeMethod(dataProcessorPtr_, "onBottomTrackAdded", Qt::QueuedConnection,
    //                                  Q_ARG(QVector<int>, graphicsSceneViewPtr_->bottomTrack()->getAllIndxs()),
    //                                  Q_ARG(bool, false));
    //    }
    //}
}

void IsobathsViewControlMenuController::onTrianglesVisible(bool state)
{
    trianglesVisible_ = state;

    if (graphicsSceneViewPtr_) {
    }
    else {
        tryInitPendingLambda();
    }
}

void IsobathsViewControlMenuController::onEdgesVisible(bool state)
{
    edgesVisible_ = state;

    if (graphicsSceneViewPtr_) {
    }
    else {
        tryInitPendingLambda();
    }
}

void IsobathsViewControlMenuController::onSetSurfaceLevelCnt(int cnt)
{
    // qDebug() << "IsobathsViewControlMenuController::onSetSurfaceLi  val:  " << val;

    levelCnt_ = cnt;
    if (graphicsSceneViewPtr_) {
        if (dataProcessorPtr_) {
            QMetaObject::invokeMethod(dataProcessorPtr_, "setSurfaceIsobathsLevelCnt", Qt::QueuedConnection, Q_ARG(int, cnt));
        }
    }
    else {
        tryInitPendingLambda();
    }
}

void IsobathsViewControlMenuController::onThemeChanged(int val)
{
    themeId_ = val;

    if (graphicsSceneViewPtr_) {
        if (dataProcessorPtr_) {
            QMetaObject::invokeMethod(dataProcessorPtr_, "setSurfaceColorTableThemeById", Qt::QueuedConnection, Q_ARG(int, themeId_));
        }
    }
    else {
        tryInitPendingLambda();
    }
}

void IsobathsViewControlMenuController::onDebugModeView(bool state)
{
    debugModeView_ = state;

    if (graphicsSceneViewPtr_) {
    }
    else {
        tryInitPendingLambda();
    }
}

void IsobathsViewControlMenuController::onProcessStateChanged(bool state)
{
    // qDebug() << "onProcessStateChanged ......is...... " << state;
    processState_ = state;

    if (graphicsSceneViewPtr_) {
        if (dataProcessorPtr_) {
            QMetaObject::invokeMethod(dataProcessorPtr_, "setUpdateIsobaths", Qt::QueuedConnection, Q_ARG(bool, processState_));
        }
    }
    else {
        tryInitPendingLambda();
    }
}

void IsobathsViewControlMenuController::onResetIsobathsButtonClicked()
{
    if (graphicsSceneViewPtr_) {
        if (dataProcessorPtr_) {
            QMetaObject::invokeMethod(dataProcessorPtr_, "clearProcessing", Qt::QueuedConnection, Q_ARG(DataProcessorType , DataProcessorType::kIsobaths));
        }

        graphicsSceneViewPtr_->getIsobathsViewPtr()->clear();
    }
}

void IsobathsViewControlMenuController::setEdgeLimitChanged(int val)
{
    if(edgeLimit_ > val) {
        emit edgeLimitChanged(val);
    }
}

void IsobathsViewControlMenuController::onEdgeLimitChanged(int val)
{
    // qDebug() << "IsobathsViewControlMenuController::onEdgeLimitChanged............";
    edgeLimit_ = val;

    if (graphicsSceneViewPtr_) {
        if (dataProcessorPtr_) {
            QMetaObject::invokeMethod(dataProcessorPtr_, "setSurfaceEdgeLimit", Qt::QueuedConnection, Q_ARG(int, val));
        }
    }
    else {
        tryInitPendingLambda();
    }
}

void IsobathsViewControlMenuController::onSetExtraWidth(int val)
{
    extraWidth_ = val;

    if (graphicsSceneViewPtr_)  {
        if (dataProcessorPtr_) {
            QMetaObject::invokeMethod(dataProcessorPtr_, "setExtraWidth", Qt::QueuedConnection, Q_ARG(int, val));
        }
    }
    else {
        tryInitPendingLambda();
    }
}


// 在 onSetExtraWidth 实现之后添加
void IsobathsViewControlMenuController::onVerticalScaleSliderValueChanged(float value)
{
    if (!graphicsSceneViewPtr_) {
        return;
    }
    graphicsSceneViewPtr_->setVerticalScale(value);
}

float IsobathsViewControlMenuController::verticalScale() const
{
    if (!graphicsSceneViewPtr_) {
        return 1.0f;
    }
    return graphicsSceneViewPtr_->verticalScale();
}


void IsobathsViewControlMenuController::autoDrawTrackBoundary()
{
    if (!graphicsSceneViewPtr_) {
        return;
    }
    qDebug() << "IsobathsViewControlMenuController::autoDrawTrackBoundary().....";
    auto polygon = graphicsSceneViewPtr_->polygonOutline();
    // polygon->autoGenerateFromAlphaShape();
}
