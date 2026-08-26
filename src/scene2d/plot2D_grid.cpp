#include "plot2D_grid.h"
#include "plot2D.h"

Plot2DGrid::Plot2DGrid() : angleVisibility_(false)
{}

bool Plot2DGrid::draw(Plot2D* parent, Dataset* dataset)
{
    Canvas &canvas        = parent->canvas();
    DatasetCursor &cursor = parent->cursor();

    if (!isVisible()) {
       return false;
    }

    const int imageWidth    = canvas.width();
    const int imageHeight   = canvas.height();
    const int linesCount    = 5;
    const int minorPerMajor = 5;

    QPen pen(Qt::white);
    QPainter* p = canvas.painter();
    p->setPen(pen);
    QFontMetrics fm(p->font());

    float distFrom = 0.0f, distTo = 0.0f;
    bool rangeValid = false;
    if(cursor.distance.isValid()) {
        distFrom = loRngMin_ / 100.0;
        distTo   = loRngMax_ / 100.0;
        rangeValid = distTo > distFrom;
    }

    // ========== 小刻度（先画，避免被大刻度覆盖） ============
    if (rangeValid) {
        pen.setWidth(2);
        const int minorLen = imageHeight * 0.01;   //小刻度线长度
        for(int i = 0; i < linesCount; ++i) {
            int majorY1 = i * imageHeight / linesCount;
            int majorY2 = (i + 1) * imageHeight / linesCount;
            for (int j = 1; j < minorPerMajor; ++j) {
                int posY = majorY1 + (majorY2 - majorY1) * j / minorPerMajor;
                if (posY <= 0 || posY >= imageHeight) {
                    continue;
                }
                if (invert_) {
                    p->drawLine(0, posY, minorLen, posY);
                }
                else {
                    p->drawLine(imageWidth - minorLen, posY, imageWidth, posY);
                }
            }
        }
    }

    pen.setWidth(3);
    p->setPen(pen);
    for(int i = 0; i <= linesCount; ++i) {
        const int posY = i * imageHeight / linesCount;

        QString lineText;
        if(rangeValid) {
            const float distRange{distTo - distFrom};
            const float rangeVal{distRange * i / linesCount + distFrom};
            if(i == linesCount) {
                lineText = QString::number(rangeVal, 'f', 0) + "m";
            } else {
                lineText = QString::number(rangeVal, 'f', 1) + "m";
            }
        }

        const int textW = fm.horizontalAdvance(lineText);
        if(invert_) {
            p->drawLine(0, posY, textW, posY);
        }
        else {
            p->drawLine(imageWidth-textW, posY, imageWidth, posY);
        }

        if(!lineText.isEmpty()) {
            int textY = posY - 4;
            if(i == 0) {
               textY = posY + fm.ascent() + 2;   //顶部刻度文本往下偏移
            }

            if(i == linesCount) {
               textY = posY - fm.descent() - 2;  //底部刻度文本往上偏移
            }
            const int textX = invert_ ? textW * 0.15 : (imageWidth - textW * 1.2);
            p->drawText(textX, textY, lineText);
        }
    }

    if (!invert_) {
        p->drawLine(imageWidth, 0, imageWidth, imageHeight);
    }

    return true;
}

void Plot2DGrid::setAngleVisibility(bool state)
{
    angleVisibility_ = state;
}

void Plot2DGrid::setLoRngRange(int minLoRng, int maxLoRng)
{
    loRngMin_ = minLoRng;
    loRngMax_ = maxLoRng;
}

int Plot2DGrid::getLoRngMax()
{
    return loRngMax_;
}
