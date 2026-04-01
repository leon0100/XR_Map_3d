#pragma once

#include "scene_object.h"
#include "dataset_defs.h"

class GraphicsScene3dView;

class CustomTrack : public SceneObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CustomTrack)

    NED llaToNed(const LLA& lla, LLARef* ref);
    QVector3D llaToVector3D(const LLA& lla, LLARef* ref);

public:
    class CustomTrackRenderImplementation : public SceneObject::RenderImplementation
    {
    public:
        CustomTrackRenderImplementation();
        virtual ~CustomTrackRenderImplementation();

        virtual void render(QOpenGLFunctions* ctx,
                            const QMatrix4x4& mvp,
                            const QMap<QString, std::shared_ptr<QOpenGLShaderProgram>>& shaderProgramMap) const override final;

        virtual void render(QOpenGLFunctions* ctx,
                            const QMatrix4x4& model,
                            const QMatrix4x4& view,
                            const QMatrix4x4& projection,
                            const QMap<QString, std::shared_ptr<QOpenGLShaderProgram>>& shaderProgramMap) const override final;

    private:
        friend class CustomTrack;
    };

    explicit CustomTrack(GraphicsScene3dView* view = nullptr, QObject* parent = nullptr);
    virtual ~CustomTrack();
    virtual SceneObjectType type() const override final;

    void addLLAPoints(const QVector<LLA>& llaPoints);
    void clearCustomTrack();
    void setTrackColor(const QColor& color);
    void setTrackWidth(float width);

public Q_SLOTS:
    virtual void clearData() override final;

private:
    QColor trackColor_;
    float trackWidth_;
};
