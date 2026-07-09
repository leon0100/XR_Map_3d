#include "plot2D_grid.h"
#include "plot2D.h"

Plot2DGrid::Plot2DGrid() : angleVisibility_(false)
{}

bool Plot2DGrid::draw(Plot2D* parent, Dataset* dataset)
{
    auto &canvas = parent->canvas();
    auto &cursor = parent->cursor();

    if (!isVisible())  return false;

    const int imageHeight = canvas.height();
    const int imageWidth  = canvas.width();
    const int linesCount  = 6;
    const int minorPerMajor = 5;

    QPen pen(Qt::white);
    QPainter* p = canvas.painter();
    p->setPen(pen);
    QFont font = p->font();
    font.setPixelSize(imageHeight * 0.02);
    QFontMetrics fm(font);

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
                if (posY <= 0 || posY >= imageHeight) continue;
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
            lineText = QString::number(rangeVal, 'f', 1) + "m";
        }

        const int textW = fm.horizontalAdvance(lineText) * 0.8;

        if(invert_) {
            p->drawLine(0, posY, textW, posY);
        }
        else {
            p->drawLine(imageWidth-textW, posY, imageWidth, posY);
        }

        if(!lineText.isEmpty()) {
            int textY = posY - 4;
            if(i == 0)           textY = posY + fm.ascent() + 2;        //顶部刻度文本往下偏移
            if(i == linesCount)  textY = posY - fm.descent() - 2;       //底部刻度文本往上偏移
            const int textX = invert_ ? textW * 0.1 : (imageWidth-textW);
            p->drawText(textX, textY, lineText);
        }
    }

    if (!invert_) {
        p->drawLine(imageWidth, 0, imageWidth, imageHeight);
    }

    // // 图像深度
    // if (cursor.distance.isValid()) {
    //     p->setFont(QFont("Asap", 26, QFont::Normal));
    //     QFontMetrics fm2(p->font());
    //     float val{ cursor.distance.to };
    //     bool isInteger = std::abs(val - std::round(val)) < kmath::fltEps;
    //     QString rangeText = QString::number(val, 'f', isInteger ? 0 : 2) + QObject::tr(" m");
    //     const int w = fm2.horizontalAdvance(rangeText);
    //     const int x = invert_ ? (textXOffset * 2) : (imageWidth - textXOffset * 0.5 - w);
    //     p->drawText(x, imageHeight - 10, rangeText);
    // }

    // rangefinder
    // if (_rangeFinderLastVisible && cursor.distance.isValid()) {
    //     Epoch* lastEpoch = dataset->last();
    //     Epoch* preLastEpoch = dataset->lastlast();
    //     if (!lastEpoch || !preLastEpoch) {
    //         return false;
    //     }
    //     float distance = NAN;

    //     if (lastEpoch != NULL && qIsFinite(lastEpoch->rangeFinder())) {
    //         distance = lastEpoch->rangeFinder();
    //     }
    //     else if (preLastEpoch != NULL && qIsFinite(preLastEpoch->rangeFinder())) {
    //         distance = preLastEpoch->rangeFinder();
    //     }

    //     if (qIsFinite(distance)) {
    //         pen.setColor(QColor(250, 100, 0));
    //         p->setPen(pen);
    //         p->setFont(QFont("Asap", 40, QFont::Normal));
    //         float val{ round(distance * 100.f) / 100.f };
    //         bool isInteger = std::abs(val - std::round(val)) < kmath::fltEps;
    //         QString rangeText = QString::number(val, 'f', isInteger ? 0 : 2) + QObject::tr(" m");
    //         p->drawText(imageWidth / 2 - rangeText.size() * 32, imageHeight - 15, rangeText);
    //     }
    // }

    // if(true) {
    //     Epoch* lastEpoch = dataset->last();
    //     Epoch* preLastEpoch = dataset->lastlast();
    //     if (!lastEpoch || !preLastEpoch) {
    //         return false;
    //     }

    //     Q_UNUSED(lastEpoch)
    //     Q_UNUSED(preLastEpoch)

    //     float temp = NAN;
    //     temp = dataset->getLastTemp();

    //     if (temperatureVisible_ && qIsFinite(temp)) {
    //         pen.setColor(QColor(80, 200, 0));
    //         p->setPen(pen);
    //         p->setFont(QFont("Asap", 40, QFont::Normal));
    //         float val{ round(temp * 100.f) / 100.f };
    //         bool isInteger = std::abs(val - std::round(val)) < kmath::fltEps;
    //         QString rangeText = QString::number(val, 'f', isInteger ? 0 : 1) + QObject::tr("°");
    //         p->drawText(imageWidth / 2 - 300, imageHeight - 15, rangeText);
    //     }
    // }

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
