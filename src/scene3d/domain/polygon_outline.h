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
    NED llaToNed(const LLA& lla, LLARef* ref);
    QVector3D llaToVector3D(const LLA& lla, LLARef* ref);


    void drawPolygonOutline(double latitide, double longitude);
    bool getOutlineMode() const;

    LLARef getLlaRef();
    void setLlaRef(const LLARef &val);

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
                            const QMatrix4x4& model,
                            const QMatrix4x4& view,
                            const QMatrix4x4& projection,
                            const QMap<QString, std::shared_ptr<QOpenGLShaderProgram>>& shaderProgramMap) const override final;

    private:
        friend class PolygonOutline;
        QVector<int> selectedVertexIndices_;

    };

    explicit PolygonOutline(GraphicsScene3dView* view = nullptr, QObject* parent = nullptr);
    virtual ~PolygonOutline();
    virtual SceneObjectType type() const override final;

    void setDatasetPtr(Dataset* datasetPtr);
    void polygonAddPoint();

    void addLLAPoints(const QVector<LLA>& llaPoints);
    void clearPolygonOutline();
    void setTrackColor(const QColor& color);
    void setTrackWidth(float width);

public Q_SLOTS:
    virtual void clearData() override final;


protected:
    friend class GraphicsScene3dView;

private:
    Dataset* datasetPtr_;
    int lastIndx_;
    LLARef llaRef_;

    QColor trackColor_;
    float trackWidth_;

    bool isDrawOutlineMode_ = true;
};
