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
    North_East_Down llaToNed(const LLA& lla, LLARef* ref);
    QVector3D llaToVector3D(const LLA& lla, LLARef* ref);

    void drawPolygonOutline(double latitide, double longitude);

    void setOutlineMode(bool outline);
    bool getOutlineMode() const;

    void setDraggingPoint(bool dragging);
    bool getDraggingPoint();

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
        QVector<int> selectedVertexIndices_;

    };

    explicit PolygonOutline(GraphicsScene3dView* view = nullptr, QObject* parent = nullptr);
    virtual ~PolygonOutline();
    virtual SceneObjectType type() const override final;

    void setDatasetPtr(Dataset* datasetPtr);
    void polygonAddPoint(double latitude, double longitude);

    void addLLAPoints(const QVector<LLA>& llaPoints);
    void clearPolygonOutline();
    void setTrackColor(const QColor& color);
    void setTrackWidth(float width);


    void setVertexEditable(bool editable);
    bool isVertexEditable() const;
    int getNearestVertexIndex(const QVector3D& point, float threshold) const;
    void moveVertex(int index, const QVector3D& newPosition);
    QVector<LLA> getVertices() const;
    QVector3D getVertexLocal(int index) const;


public Q_SLOTS:
    virtual void clearData() override final;


public slots:
    void slot_setDrawOutlineMode(bool outlineMode);


protected:
    friend class GraphicsScene3dView;

private:
    Dataset* datasetPtr_;


    QVector<LLA> polygonOutlineVec_;
    int lastIndx_;

    QColor trackColor_;
    float trackWidth_;

    bool isDrawOutlineMode_ = false;
    bool isDraggingPoint_ = false;


    bool m_vertexEditable = false;
    int m_selectedVertexIndex = -1;


public:
    int draggingPointIndex_ = -1;        //当前拖动的点索引，-1表示无
    int hoverPointIndex_ = -1;           //当前悬停的点索引
};
