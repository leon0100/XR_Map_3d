#pragma once

#include <functional>
#include <QThread>

#include "qml_component_controller.h"
#include "data_processor.h"


class GraphicsScene3dView;
class IsobathsViewControlMenuController : public QmlComponentController
{
    Q_OBJECT

public:
    explicit IsobathsViewControlMenuController(QObject* parent = nullptr);

    void setGraphicsSceneView(GraphicsScene3dView* sceneView);
    void setDataProcessorPtr(DataProcessor *dataProcessorPtr);
    void setEdgeLimitChanged(int val);

    Q_INVOKABLE void onIsobathsVisibilityCheckBoxCheckedChanged(bool checked);
    Q_INVOKABLE void onContoursVisibilityCheckBoxCheckedChanged(bool checked);
    Q_INVOKABLE void onVertexVisibilityCheckBoxCheckedChanged(bool checked);
    Q_INVOKABLE void onOutlineVisibleChanged(bool visible);
    Q_INVOKABLE void onGroundVisibleChanged(bool visible);
    Q_INVOKABLE void onUpdateIsobathsButtonClicked();
    Q_INVOKABLE void onTrianglesVisible(bool state);
    Q_INVOKABLE void onEdgesVisible(bool state);
    Q_INVOKABLE void onSetSurfaceLevelCnt(int cnt);
    Q_INVOKABLE void onThemeChanged(int val);
    Q_INVOKABLE void onDebugModeView(bool state);
    Q_INVOKABLE void onProcessStateChanged(bool state);
    Q_INVOKABLE void onEdgeLimitChanged(int val);

    Q_INVOKABLE void onVerticalScaleSliderValueChanged(float value);
    Q_INVOKABLE float verticalScale() const;

    Q_INVOKABLE void autoDrawTrackBoundary();

protected:
    virtual void findComponent() override;

private:
    void tryInitPendingLambda();


signals:
    void edgeLimitChanged(int val);


private:
    GraphicsScene3dView* graphicsSceneViewPtr_;
    DataProcessor* dataProcessorPtr_;
    std::function<void()> pendingLambda_;
    QThread thread_;
    int themeId_;
    int levelCnt_ = 8;
    int edgeLimit_;
    // int extraWidth_;
    bool visibility_;
    bool edgesVisible_;
    bool trianglesVisible_;
    bool debugModeView_;
    bool processState_;
};
