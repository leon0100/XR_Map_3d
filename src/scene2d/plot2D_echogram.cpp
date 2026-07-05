#include "plot2D_echogram.h"
#include "plot2D.h"

#include <QApplication>
#include <QSettings>
#include <QtGlobal>



Plot2DEchogram::Plot2DEchogram()
{
    setThemeId(ClassicTheme);
    setLevels(10, 100);

    QString fileName = qApp->applicationDirPath() + "/dcs_caise.tcs";
    zyColorScheme_ = new ZyColorScheme;
    zyColorScheme_->loadColorScheme(fileName);
}

Plot2DEchogram::~Plot2DEchogram()
{
    if(zyColorScheme_) {
        delete zyColorScheme_;
        zyColorScheme_ = nullptr;
    }

    _lastCursor = DatasetCursor();
}

void Plot2DEchogram::setLowLevel(float low)
{
    setLevels(low, _levels.high);
}

void Plot2DEchogram::setHightLevel(float high)
{
    setLevels(_levels.low, high);
}

void Plot2DEchogram::setLevels(float low, float high)
{
    _levels.low  = low;
    _levels.high = high;
}

void Plot2DEchogram::setColorScheme(QVector<QColor> coloros, QVector<int> levels)
{
    if(coloros.length() != levels.length()) { return; }

    _colorTable.resize(256);
    _colorLevels.resize(256);

    int nbr_levels = coloros.length() - 1;
    int i_level = 0;

    for(int i = 0; i < nbr_levels; i++) {
        while(levels[i + 1] >= i_level) {
            float b_koef = (float)(i_level - levels[i]) / (float)(levels[i + 1] - levels[i]);
            float a_koef = 1.0f - b_koef;

            int red   = qRound(coloros[i].red()*a_koef + coloros[i + 1].red()*b_koef);
            int green = qRound(coloros[i].green()*a_koef + coloros[i + 1].green()*b_koef);
            int blue  = qRound(coloros[i].blue()*a_koef + coloros[i + 1].blue()*b_koef);
            _colorHashMap[i_level] = ((red / 8) << 10) | ((green / 8) << 5) | ((blue / 8));

            _colorTable[i_level] = qRgb(red, green, blue);
            i_level++;
        }
    }

}

int Plot2DEchogram::getThemeId() const
{
    return static_cast<int>(themeId_);
}

void Plot2DEchogram::setThemeId(int theme_id)
{
    if (theme_id >= ClassicTheme && theme_id <= BWTheme) {
        themeId_ = static_cast<ThemeId>(theme_id);
    }
    else {
        themeId_ = ClassicTheme;
    }

    QVector<QColor> coloros;
    QVector<int> levels;

    if(theme_id == ClassicTheme) {
        coloros = { QColor::fromRgb(0, 0, 0), QColor::fromRgb(20, 5, 80),
                   QColor::fromRgb(50, 180, 230), QColor::fromRgb(190, 240, 250), QColor::fromRgb(255, 255, 255)};
        levels = {0, 30, 130, 220, 255};
    }
    else if(theme_id == SepiaTheme) {
        coloros = { QColor::fromRgb(0, 0, 0), QColor::fromRgb(50, 50, 10),
                   QColor::fromRgb(230, 200, 100), QColor::fromRgb(255, 255, 220)};
        levels = {0, 30, 130, 255};
    }
    else if(theme_id == WRGBDTheme) {
        coloros = {
            QColor::fromRgb(0, 0, 0),
            QColor::fromRgb(40, 0, 80),
            QColor::fromRgb(0, 30, 150),
            QColor::fromRgb(20, 230, 30),
            QColor::fromRgb(255, 50, 20),
            QColor::fromRgb(255, 255, 255),
        };

        levels = {0, 30, 80, 120, 150, 255};
    }
    else if(theme_id == WBTheme) {
        coloros = { QColor::fromRgb(0, 0, 0), QColor::fromRgb(190, 200, 200), QColor::fromRgb(230, 255, 255)};
        levels = {0, 150, 255};
    }
    else if(theme_id == BWTheme) {
        coloros = {QColor::fromRgb(230, 255, 255), QColor::fromRgb(70, 70, 70), QColor::fromRgb(0, 0, 0)};
        levels = {0, 150, 255};
    }

    setColorScheme(coloros, levels);
}

void Plot2DEchogram::setCompensation(int compensation_id)
{
    _compensation_id = compensation_id;
    resetCash();
}

void Plot2DEchogram::resetCash()
{
    _cashFlags.resetCash = true;
}

void Plot2DEchogram::setUpperRng(int maxUpRng)
{
    currentUpRng_ = maxUpRng;
    resetCash();
}

void Plot2DEchogram::setLowerRng(int minLoRng)
{
    qDebug() << " Plot2DEchogram::setLowerRng(int minLoRng)...." << minLoRng;

    currentLoRng_ = minLoRng;
    resetCash();
}

void Plot2DEchogram::addReRenderPlotIndxs(const QSet<int> &indxs)
{
    reRenderPlotIndxs_.unite(indxs);
}

void Plot2DEchogram::stretchCompressPixel(QVector<uint8_t> &rawDataVec, uint8_t* dist, int distLen, float scale, int startIndx)
{
    if(dist == nullptr || distLen <= 0 || scale < 0) {
        return;
    }

    int srcLen = rawDataVec.size();

    if(scale > 1.0f) {
        for (int i = 0; i < distLen; i++) {
            float srcPos = i / scale + startIndx;
            int srcIdx = (int)srcPos;
            if (srcIdx >= 0 && srcIdx < srcLen) {
                if (srcIdx + 1 < srcLen) {
                    float coef = srcPos - floorf(srcPos);
                    dist[i] = (uint8_t)(rawDataVec[srcIdx] * (1.0f - coef) + rawDataVec[srcIdx + 1] * coef);
                }
                else {
                    dist[i] = rawDataVec[srcIdx];
                }
            }
            else {
                dist[i] = 0;
            }
        }
    }
    else {
        scale = 1.0 / scale;
        for (int i = 0; i < distLen; i++) {
            int srcStart = (int)(i * scale) + startIndx;
            int srcEnd = (int)((i + 1) * scale) + startIndx;
            if (srcStart < 0) srcStart = 0;
            if (srcEnd > srcLen) srcEnd = srcLen;
            if (srcEnd <= srcStart) srcEnd = srcStart + 1;
            int32_t val = 0;
            int cnt = 0;
            for (int j = srcStart; j < srcEnd && j < srcLen; j++) {
                val += rawDataVec[j];
                cnt++;
            }
            dist[i] = (cnt > 0) ?  (uint8_t)(val / cnt) : 0;
        }
    }
}


void Plot2DEchogram::drawLatestWavePixel(Plot2D* parent, int panelX, int panelW, int height)
{
     if (latestWave_.isEmpty() || panelW <= 0 || height <= 0) return;

    auto& canvas = parent->canvas();
    QPainter* p  = canvas.painter();

    int bgRgb = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
    p->fillRect(panelX, 0, panelW, height, QColor::fromRgb(bgRgb));

    // 绘制分隔线
    QPen sepPen(QColor(80, 80, 80, 180));
    sepPen.setWidth(1);
    p->setPen(sepPen);
    p->drawLine(panelX, 0, panelX, height);

    for (int j = 0; j < height; j++) {
        uint8_t dataValue = latestWave_[j];
        int bgColor = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
        int rgb = bgColor;

        if (j >= 0 && j < latestWaveSfEnd_) {
            // 水表
            if(dataValue == 0) {
                rgb = bgColor;
            } else {
                if(dataValue + ZyColorScheme::colorLine * COLOR_LINE > 254) {
                    rgb = ZyColorScheme::colorScheme_surface[254];
                } else if(dataValue + ZyColorScheme::colorLine * COLOR_LINE < 0) {
                    rgb = ZyColorScheme::colorScheme_surface[0];
                } else {
                    rgb =  ZyColorScheme::colorScheme_surface[dataValue + ZyColorScheme::colorLine * COLOR_LINE];
                }
            }

        }
        else if (j >= latestWaveSfEnd_ && j < latestWaveBtStart_) {
            // 水中
            if(dataValue == 0) {
                rgb = bgColor;
            } else {
                if(dataValue + ZyColorScheme::colorLine * COLOR_LINE > 254) {
                    rgb = ZyColorScheme::colorScheme_fish[254];
                } else if(dataValue + ZyColorScheme::colorLine * COLOR_LINE < 0) {
                    rgb = ZyColorScheme::colorScheme_fish[254];
                } else {
                    rgb = ZyColorScheme::colorScheme_fish[dataValue + ZyColorScheme::colorLine * COLOR_LINE];
                }
            }
        }
        else if(j >= latestWaveBtStart_) {
            // 水底
            if(dataValue == 0) {
                rgb = bgColor;
            } else {
                if(dataValue + ZyColorScheme::colorLine * COLOR_LINE > 254) {
                    rgb = ZyColorScheme::colorScheme_bottom[254];
                } else if(dataValue + ZyColorScheme::colorLine * COLOR_LINE < 0) {
                    rgb = ZyColorScheme::colorScheme_bottom[254];
                } else {
                    rgb = ZyColorScheme::colorScheme_bottom[dataValue + ZyColorScheme::colorLine * COLOR_LINE];
                }
            }
        }

        // 以面板中线为中心，根据振幅向两侧扩展
        int halfWidth = (latestWave_[j] * (panelW / 2)) / 255;
        int centerX   = panelX + panelW / 2;
        int xStart    = centerX - halfWidth;
        int xEnd      = centerX + halfWidth;
        if (xStart < panelX) xStart = panelX;
        if (xEnd > panelX + panelW) xEnd = panelX + panelW;
        if (xEnd > xStart) {
            p->fillRect(xStart, j, xEnd - xStart, 1, QColor::fromRgb(rgb));
        }

    }


    if (bottomLineIdx_ >= 0 && bottomLineIdx_ < height) {
        QPen linePen(Qt::red);
        linePen.setWidth(2);
        p->setPen(linePen);
        p->drawLine(panelX, bottomLineIdx_, panelX + panelW, bottomLineIdx_);
    }



    //==================== 顶部信息栏 ====================
    const int infoBarHeight = height / 16;

    // 半透明背景
    p->fillRect(0, 0, panelX, infoBarHeight, QColor(0, 0, 0, 120));

    float speed = bottomLineIdx_;
    float lat   = 1111.123456;
    float lon   = 22222.654321;
    float depth = 2222.56;

    QString speedStr = !qFuzzyIsNull(speed) ? QString::number(speed, 'f', 2) + " km/h" : QStringLiteral("-- km/h");
    QString depthStr = !qFuzzyIsNull(depth) ? QString::number(depth, 'f', 2) + " m" : QStringLiteral("-- m");
    QString latStr = !qFuzzyIsNull(lat) ? QString::number(lat, 'f', 6) + QStringLiteral("°") : QStringLiteral("--");
    QString lonStr = !qFuzzyIsNull(lon) ? QString::number(lon, 'f', 6) + QStringLiteral("°") : QStringLiteral("--");

    QString line1 = QStringLiteral("Speed : %1    Depth : %2") .arg(speedStr) .arg(depthStr);
    QString line2 = QStringLiteral("Lat : %1    Lon : %2") .arg(latStr) .arg(lonStr);

    QPen textPen(QColor(255, 255, 255, 230));
    p->setPen(textPen);

    QFont font = p->font();
    font.setPixelSize(15);
    font.setBold(true);
    p->setFont(font);

    QFontMetrics fm(font);
    int lineHeight  = fm.height();
    int textWidth = fm.horizontalAdvance(line1) > fm.horizontalAdvance(line2) ? fm.horizontalAdvance(line1)
                                                    :fm.horizontalAdvance(line2);
    p->drawText(panelX - 20 - textWidth, infoBarHeight/2 - lineHeight + fm.ascent(), line1);
    p->drawText(panelX - 20 - textWidth, infoBarHeight/2 - lineHeight + lineHeight + fm.ascent(), line2);

}


int Plot2DEchogram::updateCache(Plot2D* parent, Dataset* dataset, int width, int height)
{
    // qDebug() << "::updateCash.......width:" << width << " height:" << height;
    DatasetCursor& cursor = parent->cursor();
    if (_cash.size() != width) {
        _cash.resize(width);
        resetCash();
    }

    bool isCashNotvalid = getTriggerCashReset();
    isCashNotvalid |= !_lastCursor.isChannelsEqual(cursor);
    isCashNotvalid |= !_lastCursor.isDistanceEqual(cursor);

    int wrapStartPos = qAbs(cursor.getIndex(0) % width);
    for (int i = 0; i < cursor.indexes.size(); i++) {
        if (cursor.indexes[i] > 0) {
            wrapStartPos = qAbs((cursor.indexes[i] + (width - i)) % width);
            break;
        }
    }

    for(int column = 0; column < width; column++) {
        int cursorPos = column - wrapStartPos;
        if(column < wrapStartPos) {
            cursorPos += width;
        }

        int poolIndex = cursor.getIndex(cursorPos);
        int pool_index_safe = dataset->validIndex(poolIndex);
        if(pool_index_safe >= 0) {
            bool wasValidlyRendered = true;
            if (reRenderPlotIndxs_.contains(pool_index_safe)) {
                reRenderPlotIndxs_.remove(pool_index_safe);
                wasValidlyRendered = false;
            }

            // qDebug() << "pool_index_safe..........." << pool_index_safe;
            Epoch* epochData = dataset->fromIndex(pool_index_safe);
            if(epochData == nullptr) {
                return -1;
            }
            const int cacheIndex = _cash[column].poolIndex;

            ChartParameters params = epochData->getChartParameters(cursor.channel1);
            const int pingSize = 240;
            int frameSfEnd     = params.sfEnd;
            int frameBtStart   = params.btStart;

            if (isCashNotvalid || pool_index_safe != cacheIndex || !wasValidlyRendered) {
                _cash[column].poolIndex = pool_index_safe;

                // 先获取原始长度的声呐数据
                QVector<uint8_t> rawDataVec;
                rawDataVec.resize(PING_SIZE_MAX);
                if (cursor.channel2 == CHANNEL_NONE) {
                    epochData->getSonarFramePixel(cursor.channel1, cursor.subChannel1, rawDataVec);
                }

                /*- 灵敏度滤波 -*/
                int sens = 5;
                for(int i = 0; (i < frameBtStart) && (i < pingSize) && (i < height); i++)
                {
                    if(rawDataVec[i] < sens) {
                        rawDataVec[i] = 0;
                    }
                }

                for(int i = frameBtStart; (i < pingSize) && (i < height); i++)
                {
                    if(rawDataVec[i]<(sens/2)) {
                        rawDataVec[i] = 0;
                    }
                }

                uint8_t* cacheData = new uint8_t[height];
                memset(cacheData, 0, height * sizeof(uint8_t));

                float scaleY = (float)height / pingSize * (params.loRng/(currentLoRng_ - currentUpRng_));
                int startIdx = pingSize * currentUpRng_ / params.loRng;
                stretchCompressPixel(rawDataVec, cacheData, height, scaleY, startIdx);

                _cash[column].waveData = QVector<uint8_t>(cacheData, cacheData + height);
                _cash[column].sfEnd    = frameSfEnd;
                _cash[column].btStart  = frameBtStart;
                _cash[column].bottomLineIdx = _cash[column].btStart * scaleY;
                _cash[column].state    = CashLine::CashState::CashStateValid;
                _cash[column].isNeedUpdate = true;

                uint32_t* img_data = (uint32_t*)_image.bits();
                int bytesPerLine = _image.bytesPerLine() / 4;
                for (int j = 0; j < height; j++) {
                    uint8_t dataValue = cacheData[j];
                    int bgColor = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
                    int rgb = bgColor;
                    // qDebug() << "dataValue....." << dataValue;

                     if (j >= 0 && j < frameSfEnd) {
                        // 水表
                        if(dataValue == 0) {
                            rgb = bgColor;
                        } else {
                            if(dataValue + ZyColorScheme::colorLine * COLOR_LINE > 254) {
                                rgb = ZyColorScheme::colorScheme_surface[254];
                            } else if(dataValue + ZyColorScheme::colorLine * COLOR_LINE < 0) {
                                rgb = ZyColorScheme::colorScheme_surface[0];
                            } else {
                                rgb =  ZyColorScheme::colorScheme_surface[dataValue + ZyColorScheme::colorLine * COLOR_LINE];
                            }
                        }

                    }
                      else if (j >= frameSfEnd && j < frameBtStart) {
                        // 水中
                        if(dataValue == 0) {
                            rgb = bgColor;
                        } else {
                            if(dataValue + ZyColorScheme::colorLine * COLOR_LINE > 254) {
                                rgb = ZyColorScheme::colorScheme_fish[254];
                            } else if(dataValue + ZyColorScheme::colorLine * COLOR_LINE < 0) {
                                rgb = ZyColorScheme::colorScheme_fish[254];
                            } else {
                                rgb = ZyColorScheme::colorScheme_fish[dataValue + ZyColorScheme::colorLine * COLOR_LINE];
                            }
                        }
                    }
                     else if(j >= frameBtStart) {
                        // 水底
                        if(dataValue == 0) {
                            rgb = bgColor;
                        } else {
                            if(dataValue + ZyColorScheme::colorLine * COLOR_LINE > 254) {
                                rgb = ZyColorScheme::colorScheme_bottom[254];
                            } else if(dataValue + ZyColorScheme::colorLine * COLOR_LINE < 0) {
                                rgb = ZyColorScheme::colorScheme_bottom[254];
                            } else {
                                rgb = ZyColorScheme::colorScheme_bottom[dataValue + ZyColorScheme::colorLine * COLOR_LINE];
                            }
                        }
                    }

                    QRgb color = qRgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
                    img_data[j* bytesPerLine + column] = color;
                }

                delete[] cacheData;
                cacheData = nullptr;
            }
        }

        else {
            if(isCashNotvalid || _cash[column].state != CashLine::CashState::CashStateEraced) {
                _cash[column].poolIndex = -1;
                _cash[column].state = CashLine::CashState::CashStateEraced;
                _cash[column].isNeedUpdate = true;

                uint32_t* img_data = (uint32_t*)_image.bits();
                int bytesPerLine = _image.bytesPerLine() / 4;
                int bgColor = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
                QRgb bgRgb  = qRgb((bgColor >> 16) & 0xFF, (bgColor >> 8) & 0xFF, bgColor & 0xFF);

                for (int j = 0; j < height; j++) {
                    img_data[j * bytesPerLine + column] = bgRgb;
                }
            }
        }
    }


    int visualRightColumn = (wrapStartPos == 0) ? (width - 1) : (wrapStartPos - 1);
    if (visualRightColumn >= 0 && visualRightColumn < _cash.size()) {
        const CashLine& line = _cash[visualRightColumn];
        if (!line.waveData.isEmpty()) {
            latestWave_        = line.waveData;
            latestWaveSfEnd_   = line.sfEnd;
            latestWaveBtStart_ = line.btStart;
            bottomLineIdx_     = line.bottomLineIdx;
        }
    }

    _lastCursor = cursor;

    return wrapStartPos;
}

bool Plot2DEchogram::draw(Plot2D* parent, Dataset* dataset)
{
    auto& canvas = parent->canvas();
    auto& cursor = parent->cursor();

    _colorLevels.clear();

    if (isVisible() && dataset != nullptr && cursor.distance.isValid()) {
        int image_width  = canvas.width();
        const int image_height = canvas.height();

        if(_image.width() != image_width || _image.height() != image_height) {
            _image  = QImage(image_width, image_height, QImage::Format_RGB32);
            _pixmap = QPixmap(image_width, image_height);

            int bgRgb = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
            _image.fill(QColor::fromRgb(bgRgb));

            for (int i = 0; i < _cash.size(); i++) {
                _cash[i].isNeedUpdate = true;
                if (_cash[i].state == CashLine::CashState::CashStateEraced) {
                    _cash[i].state = CashLine::CashState::CashStateNotValid;
                }
            }
        }

        int cash_position = updateCache(parent, dataset, image_width, image_height);

        QPainter p(&_pixmap);

        int cash_col = 0;
        while(cash_col < image_width) {
            int cash_col_1 = cash_col;
            while(cash_col < image_width && (_cash[cash_col].isNeedUpdate || _flagColorChanged)) {
                _cash[cash_col].isNeedUpdate = false;
                cash_col++;
            }

            int cashUpdateWidth = cash_col - cash_col_1;
            if(cashUpdateWidth > 0) {
                p.drawImage(cash_col_1, 0, _image, cash_col_1, 0 , cashUpdateWidth, image_height, Qt::ThresholdDither);
            }
            else {
                cash_col++;
            }
        }

        _flagColorChanged = false;

        canvas.painter()->drawPixmap(0, 0, _pixmap, cash_position, 0, image_width - cash_position, image_height);
        canvas.painter()->drawPixmap(image_width - cash_position, 0, _pixmap, 0, 0, cash_position, image_height);

        drawLatestWavePixel(parent, image_width, WAVE_PIXEL_WIDTH, image_height);
    }

    return true;
}

float Plot2DEchogram::getLowLevel() const
{
    return _levels.low;
}

float Plot2DEchogram::getHighLevel() const
{
    return _levels.high;
}
