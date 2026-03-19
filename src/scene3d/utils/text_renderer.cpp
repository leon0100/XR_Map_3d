#include "text_renderer.h"
#include "draw_utils.h"
#include <QDebug>
#include <QFile>

// #include <ft2build.h> // NOLINT
// #include FT_FREETYPE_H


TextRenderer& TextRenderer::instance()
{
    static TextRenderer instance;
    return instance;
}

void TextRenderer::setFontPixelSize(int size)
{
    if(m_fontPixelSize != size){
        m_fontPixelSize = size;
        initFont();
    }
}

void TextRenderer::setColor(const QColor &color)
{
    if(m_color != color)
        m_color = color;
}

QColor TextRenderer::getColor() const
{
    return m_color;
}

void TextRenderer::setBackgroundColor(const QColor &color)
{
    if (m_backgroundColor != color)
        m_backgroundColor = color;
}

int TextRenderer::getCharPixelHeight() const
{
    if (m_chars.contains('0')) {
        return m_chars['0'].size.y();
    }
    return m_fontPixelSize;
}

void TextRenderer::render(const QString &text, float scale, QVector2D pos, bool drawBackground,
                          QOpenGLFunctions *ctx, const QMatrix4x4 &projection, const QMap <QString, std::shared_ptr <QOpenGLShaderProgram>>& shaderProgramMap)
{
    const float padding = 5.0f;

    // text_back
    if (drawBackground) {
        auto backgroundShader = shaderProgramMap.value("text_back", nullptr);

        if (!backgroundShader) {
            qWarning() << "Shader program 'text_back' not found!";
            return;
        }

        if (!backgroundShader->bind()) {
            qCritical() << "Error binding background shader program.";
            return;
        }

        if (!m_arrayBuffer.bind()) {
            qCritical() << "Error binding vertex array buffer!";
            return;
        }

        QVector2D bgTopLeft = pos - QVector2D(padding, -padding);
        QVector2D bgBottomRight = pos;
        float maxHeight = 0.0f;

        for (auto it = text.begin(); it != text.end(); ++it) {
            uint16_t c = it->unicode();
            if (!m_chars.contains(c))
                continue;

            auto ch = m_chars.value(c);
            bgBottomRight.setX(bgBottomRight.x() + (ch.advance >> 6) * scale);
            maxHeight = qMax(maxHeight, ch.size.y() * scale);
        }
        bgBottomRight.setY(pos.y() - maxHeight);
        bgBottomRight += QVector2D(padding, -padding);

        float bgVertices[6][3] = {
            { bgTopLeft.x(), bgTopLeft.y(), 0.0f },
            { bgTopLeft.x(), bgBottomRight.y(), 0.0f },
            { bgBottomRight.x(), bgBottomRight.y(), 0.0f },

            { bgTopLeft.x(), bgTopLeft.y(), 0.0f },
            { bgBottomRight.x(), bgBottomRight.y(), 0.0f },
            { bgBottomRight.x(), bgTopLeft.y(), 0.0f }
        };

        backgroundShader->setUniformValue("mvp_matrix", projection);
        backgroundShader->setUniformValue("color", QVector4D(0.18f, 0.18f, 0.18f, 1.0f));

        m_arrayBuffer.write(0, bgVertices, 6 * 3 * sizeof(float));

        int vertexLocation = backgroundShader->attributeLocation("a_position");
        backgroundShader->enableAttributeArray(vertexLocation);
        backgroundShader->setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3);

        ctx->glDrawArrays(GL_TRIANGLES, 0, 6);

        backgroundShader->release();
        m_arrayBuffer.release();
    }

    // text
    {
        auto textShader = shaderProgramMap.value("text", nullptr);

        if (!textShader) {
            qWarning() << "Shader program 'text' not found!";
            return;
        }

        if (!textShader->bind()) {
            qCritical() << "Error binding text shader program.";
            return;
        }

        if (!m_arrayBuffer.bind()) {
            qCritical() << "Error binding vertex array buffer!";
            return;
        }

        ctx->glEnable(GL_BLEND);
        ctx->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        ctx->glActiveTexture(GL_TEXTURE0);

        textShader->setUniformValue("mvp_matrix", projection);

        auto it = text.begin();
        while (it != text.end()) {
            uint16_t c = it->unicode();
            if (!m_chars.contains(c)) {
                ++it;
                continue;
            }

            auto ch = m_chars.value(c);

            float pen_x = pos.x() + ch.bearing.x() * scale;
            float pen_y = pos.y() - (ch.bearing.y() * scale);

            const float w = ch.size.x() * scale;
            const float h = ch.size.y() * scale;

            float vertices[6][4] = {
                { pen_x,     pen_y,     0.0, 0.0 },
                { pen_x,     pen_y + h, 0.0, 1.0 },
                { pen_x + w, pen_y + h, 1.0, 1.0 },

                { pen_x,     pen_y,     0.0, 0.0 },
                { pen_x + w, pen_y + h, 1.0, 1.0 },
                { pen_x + w, pen_y,     1.0, 0.0 }
            };

            if (ch.texture) {
                ch.texture->bind();
                textShader->setUniformValue("texture", ch.texture->textureId());
                textShader->setUniformValue("textColor", DrawUtils::colorToVector4d(m_color));

                m_arrayBuffer.write(0, vertices, 6 * 4 * sizeof(float));

                int vertexLocation = textShader->attributeLocation("a_position");
                textShader->enableAttributeArray(vertexLocation);
                textShader->setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 2, stride2d);

                int texcoordLocation = textShader->attributeLocation("a_texcoord");
                textShader->enableAttributeArray(texcoordLocation);
                textShader->setAttributeBuffer(texcoordLocation, GL_FLOAT, 2 * sizeof(float), 2, stride2d);

                ctx->glDrawArrays(GL_TRIANGLES, 0, 6);

                ch.texture->release();
            }

            pos.setX(pos.x() + (ch.advance >> 6) * scale);
            ++it;
        }

        ctx->glDisable(GL_BLEND);

        textShader->release();
        m_arrayBuffer.release();
    }
}

void TextRenderer::render3D(const QString &text, float scale, QVector3D pos, const QVector3D &dir, QOpenGLFunctions *ctx, const QMatrix4x4 &pvm, const QMap <QString, std::shared_ptr <QOpenGLShaderProgram>>& shaderProgramMap)
{
    auto shaderProgram = shaderProgramMap.value("text", nullptr);

    if (!shaderProgram) {
        qWarning() << "Shader program 'static' not found!";
        return;
    }

    if (!shaderProgram->bind()) {
        qCritical() << "Error binding text shader program.";
        return;
    }

    if(!m_arrayBuffer.bind()){
        qCritical() << "Error binding vertex array buffer!";
        return;
    }

    ctx->glEnable(GL_BLEND);
    ctx->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shaderProgram->setUniformValue("mvp_matrix", pvm);

    float cs = dir.normalized().x();
    float sn = dir.normalized().y();

    auto it = text.begin();
    while(it != text.end()){
        uint16_t c = it->unicode();

        if (!m_chars.contains(c)) {
            ++it;
            continue;
        }

        auto ch = m_chars.value(c);

        float pen_x = ch.bearing.x() * scale;
        float pen_y = -ch.bearing.y() * scale;
        float pen_z = 0.0f;

        const float w = ch.size.x() * scale;
        const float h = ch.size.y() * scale;

        float vertices[6][5] = {
            { pen_x,     pen_y,     pen_z, 0.0, 0.0 },
            { pen_x,     pen_y + h, pen_z, 0.0, 1.0 },
            { pen_x + w, pen_y + h, pen_z, 1.0, 1.0 },

            { pen_x,     pen_y ,    pen_z, 0.0, 0.0 },
            { pen_x + w, pen_y + h, pen_z, 1.0, 1.0 },
            { pen_x + w, pen_y,     pen_z, 1.0, 0.0 }
        };

        for (int i = 0; i < 6; ++i) {
            float vx = vertices[i][0];
            float vy = vertices[i][1];
            float rotated_x = vx * cs - vy * sn;
            float rotated_y = vx * sn + vy * cs;
            vertices[i][0] = rotated_x + pos.x();
            vertices[i][1] = rotated_y + pos.y();
            vertices[i][2] = pos.z();
        }

        if (ch.texture) {
            ch.texture->bind();
            shaderProgram->setUniformValue("texture", m_chars[c].texture->textureId());
            shaderProgram->setUniformValue("textColor", DrawUtils::colorToVector4d(m_color));

            m_arrayBuffer.write(0,vertices, 6 * 5 * sizeof(float));

            int vertexLocation = shaderProgram->attributeLocation("a_position");
            shaderProgram->enableAttributeArray(vertexLocation);
            shaderProgram->setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 3, stride3d);

            int texcoordLocation = shaderProgram->attributeLocation("a_texcoord");
            shaderProgram->enableAttributeArray(texcoordLocation);
            shaderProgram->setAttributeBuffer(texcoordLocation, GL_FLOAT, 3 * sizeof(float), 2, stride3d);

            ctx->glDrawArrays(GL_TRIANGLES, 0, 6);
            ch.texture->release();
        }

        float adv = (ch.advance >> 6) * scale;
        pos.setX(pos.x() + adv * cs);
        pos.setY(pos.y() + adv * sn);

        it++;
    }

    ctx->glDisable(GL_BLEND);

    shaderProgram->release();
    m_arrayBuffer.release();
}

void TextRenderer::cleanup()
{
    if (QOpenGLContext::currentContext()) {
        for (auto& ch : m_chars) {
            if (ch.texture) {
                ch.texture->destroy();
            }
        }
    }
    m_chars.clear();

    m_arrayBuffer.destroy();
}

TextRenderer::TextRenderer()
{
    initBuffers();
    initFont();
}

TextRenderer::~TextRenderer()
{
    // cleanup
}

void TextRenderer::initBuffers()
{
    m_arrayBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_arrayBuffer.create();
    m_arrayBuffer.bind();
    m_arrayBuffer.allocate(6 * 5 * sizeof(float));
}


void TextRenderer::initFont()
{
    m_chars.clear();

    QFont font;
    font.setPixelSize(m_fontPixelSize);

    QFontMetrics metrics(font);

    auto loadGlyph = [&](ushort c)
    {
        QChar ch(c);

        int w = metrics.horizontalAdvance(ch);
        int h = metrics.height();

        if (w <= 0 || h <= 0)
            return;

        QString str(ch);

        QImage image(w, h, QImage::Format_ARGB32);
        image.fill(Qt::transparent);

        QPainter painter(&image);
        painter.setFont(font);
        painter.setPen(Qt::white);
        painter.drawText(0, metrics.ascent(), str);
        painter.end();

        auto tex = std::make_shared<QOpenGLTexture>(image);
        tex->setMinificationFilter(QOpenGLTexture::Linear);
        tex->setMagnificationFilter(QOpenGLTexture::Linear);
        tex->setWrapMode(QOpenGLTexture::ClampToEdge);

        Character character;
        character.texture = tex;
        character.num = c;
        character.advance = metrics.horizontalAdvance(ch) << 6;
        character.size = QVector2D(w, h);
        character.bearing = QVector2D(0, metrics.ascent());

        m_chars.insert(c, character);
    };

    auto loadRange = [&](ushort start, ushort end)
    {
        for (ushort c = start; c <= end; ++c)
            loadGlyph(c);
    };

    loadRange(0x20, 0x7F);   // ASCII
}
