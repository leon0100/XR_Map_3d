#pragma once

#include "scene_object.h"
#include "dataset_defs.h"
#include "dataset.h"


class GraphicsScene3dView;
class PolygonOutline : public SceneObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(PolygonOutline)

public:
    explicit PolygonOutline(GraphicsScene3dView* view = nullptr, QObject* parent = nullptr);
    virtual ~PolygonOutline();
    virtual SceneObjectType type() const override final;

    void setDatasetPtr(Dataset* datasetPtr);
    void polygonAddPoint(double latitude, double longitude);
    void modifyPolygonVertex(int index, LLA lla);
    int getNearestVertexIndex(LLA lla) const;

    void setQmlRootObject(QObject* object);

    void setOutlineMode(bool outline);
    bool getOutlineMode() const;

    void setDraggingPoint(bool dragging);
    bool getDraggingPoint();

    void selectVertex(int index, bool isDraggingPt);



public:
    class PolygonOutlineRenderImplementation : public SceneObject::RenderImplementation
    {
    public:
        PolygonOutlineRenderImplementation();
        virtual ~PolygonOutlineRenderImplementation();

        virtual void render(QOpenGLFunctions* ctx,
                        const QMatrix4x4& mvp,
                        const QMap<QString, std::shared_ptr<QOpenGLShaderProgram>>& shaderProgramMap) const override final;

        virtual void render(QOpenGLFunctions* ctx,
                        const QMatrix4x4& model, const QMatrix4x4& view, const QMatrix4x4& projection,
                        const QMap<QString, std::shared_ptr<QOpenGLShaderProgram>>& shaderProgramMap) const override final;


    private:
        friend class PolygonOutline;
        int draggingPtIndex_ = -1;
        bool isDraggingPoint_ = false;

    };


public Q_SLOTS:
    virtual void clearData() override final;

public slots:
    void autoGenerateFromAlphaShape();


protected:
    friend class GraphicsScene3dView;

private:
    Dataset* datasetPtr_;
    QObject* qmlRootObject_;

    int lastIndx_ = -1;

    bool isDrawOutlineMode_ = false;
    bool isDraggingPoint_ = false;

    bool m_vertexEditable = false;
    int m_selectedVertexIndex = -1;


public:
    int draggingPtIndex_ = -1;        //当前拖动的点索引，-1表示无



};
