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

void Plot2DEchogram::setBottomLineVisible(bool isVisible)
{
    bottomLineVisible_ = isVisible;
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
    currentLoRng_ = minLoRng;
    resetCash();
}

int  Plot2DEchogram::getSoundVelocity()
{
    return soundVelocity_;
}

void Plot2DEchogram::setSoundVelocity(int soundVelocity, int draftOffset)
{
    soundVelocity_ = soundVelocity;
    draftOffset_ = draftOffset;
    resetCash();
}

void Plot2DEchogram::setSensitivity(int sensitive)
{
    sensLevel_ = sensitive;
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
            dist[i] = (cnt > 0) ? (uint8_t)(val / cnt) : 0;
        }
    }
}


void Plot2DEchogram::drawLatestWavePixel(Plot2D* parent, int panelX, int panelW, int height)
{
    auto& canvas = parent->canvas();
    QPainter* p  = canvas.painter();

    int bgRgb = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
    p->fillRect(panelX, 0, panelW, height, QColor::fromRgb(bgRgb));

    if (wavePixel_.waveData.isEmpty()) return;

    // 绘制分隔线
    QPen sepPen(QColor(80, 80, 80, 180));
    sepPen.setWidth(1);
    p->setPen(sepPen);
    p->drawLine(panelX, 0, panelX, height);

    QVector<uint8_t> latestWave = wavePixel_.waveData;
    for (int j = 0; j < height; j++) {
        uint8_t dataValue = latestWave[j];
        int bgColor = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
        int rgb = bgColor;

        if (j >= 0 && j < wavePixel_.sfEnd) {
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
        else if (j >= wavePixel_.sfEnd && j < wavePixel_.btStart) {
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
        else if(j >= wavePixel_.btStart) {
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
        int halfWidth = (latestWave[j] * (panelW / 2)) / 255;
        int centerX   = panelX + panelW / 2;
        int xStart    = centerX - halfWidth;
        int xEnd      = centerX + halfWidth;
        if (xStart < panelX) xStart = panelX;
        if (xEnd > panelX + panelW) xEnd = panelX + panelW;
        if (xEnd > xStart) {
            p->fillRect(xStart, j, xEnd - xStart, 1, QColor::fromRgb(rgb));
        }

    }



    int bottomLineIdx = wavePixel_.bottomLineIdx;
    if (bottomLineIdx >= 0 && bottomLineIdx < height) {
        QPen linePen(Qt::red);
        linePen.setWidth(2);
        p->setPen(linePen);
        p->drawLine(panelX, bottomLineIdx, panelX + panelW, bottomLineIdx);
    }


    //==================== 顶部信息栏 ====================
    const int infoBarHeight = height / 16;
    p->fillRect(0, 0, panelX, infoBarHeight, QColor(0, 0, 0, 80));

    float depth   = wavePixel_.depth / 100.0f;
    float heading = wavePixel_.heading / 10.0f;
    float speed   = wavePixel_.speed / 100 * 0.514444f;
    float lat     = wavePixel_.latitude;
    float lon     = wavePixel_.longitude;
    float temp    = (wavePixel_.temperature / 10.0f - 32) / 1.8f;

    QString depthStr   = QString::number(depth, 'f', 2)  + "m";
    QString headingStr = QString::number(heading, 'f',1) + "°";
    QString speedStr   = QString::number(speed, 'f', 2)  + "m/s";
    QString latStr     = QString::number(lat, 'f', 6) + "°";
    QString lonStr     = QString::number(lon, 'f', 6) + "°";
    QString tempStr    = QString::number(temp, 'f', 1) + QString::fromUtf8("\xE2\x84\x83");

    QString line1 = QStringLiteral("Depth:%1  HDG:%2 Speed: %3") .arg(depthStr).arg(headingStr).arg(speedStr);
    QString line2 = QStringLiteral("Lat:%1  Lon:%2  Temp:%3") .arg(latStr).arg(lonStr).arg(tempStr);;

    QPen textPen(QColor(255, 255, 255, 230));
    p->setPen(textPen);

    QFont font = p->font();
    font.setPixelSize(infoBarHeight * 0.2);
    font.setBold(true);
    p->setFont(font);

    QFontMetrics fm(font);
    int lineHeight  = fm.height();
    int textWidth = fm.horizontalAdvance(line1) > fm.horizontalAdvance(line2) ? fm.horizontalAdvance(line1)
                                                    :fm.horizontalAdvance(line2);
    p->drawText(panelX - 32 - textWidth, infoBarHeight/2 - lineHeight + fm.ascent(), line1);
    p->drawText(panelX - 32 - textWidth, infoBarHeight/2 - lineHeight + lineHeight + fm.ascent(), line2);

}

void Plot2DEchogram::drawBottomLine(Canvas canvas, int width, int cash_position, bool isVisible)
{
    if(isVisible) {
        if(_cash.size() == width && width > 1) {
            QPainter* cp = canvas.painter();
            QPen profilePen(QColor(255, 0, 0));
            profilePen.setWidth(1);
            cp->setPen(profilePen);
            QPolygonF profilePts;
            profilePts.reserve(width);
            for(int x = 0; x < width; ++x) {
                int col = (cash_position + x) % width;
                int idx = _cash[col].bottomLineIdx;
                if (_cash[col].state == CashLine::CashState::CashStateValid) {
                    profilePts.append(QPointF(x, idx));
                }
                else if (profilePts.size() > 1) {
                    cp->drawPolyline(profilePts);
                    profilePts.clear();
                }
            }
            if(profilePts.size() > 1) {
                cp->drawPolyline(profilePts);
            }
        }
    }

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
            if (isCashNotvalid || pool_index_safe != cacheIndex || !wasValidlyRendered) {
                _cash[column].poolIndex = pool_index_safe;


                ChartParameters params = epochData->getChartParameters(cursor.channel1);
                const int pingSize = params.pingSize;

                int sfEnd = 0, btStart = 0, draft = 0;

                float upRng = params.upRng;
                float loRng = params.loRng;
                float depth = params.depth;
                if((upRng < 0) || (loRng < 0) || (pingSize <= 0) || (upRng == loRng)) {
                    draft = 0;
                    btStart = 0;
                    sfEnd = 0;
                }

                int startIdx = 0;

                if(loRng != 0) {
                    // if(loRng == upRng) {
                    //     draft   = 0;
                    //     btStart  = 0;
                    //     sfEnd    = 0;
                    //     startIdx = 0;
                    // }
                    draft = (1500.0/soundVelocity_) * (draftOffset_ / (loRng + upRng)) * pingSize;

                    btStart = (1500.0/soundVelocity_) * (depth / (loRng - upRng)) * pingSize;

                    float surfaceEnd;
                    if((depth < 100) && (depth > 30)) {
                        surfaceEnd = depth - 10 + draftOffset_;
                    } else {
                        surfaceEnd = 100 + draftOffset_;
                    }
                    sfEnd = (1500.0/soundVelocity_) * (surfaceEnd / (loRng - upRng)) * pingSize;
                }
                if((btStart < 0) || (sfEnd < 0) || (depth < 0)) {
                    draft   = 0;
                    btStart = 0;
                    sfEnd   = 0;
                }

                QVector<uint8_t> rawDataVec;
                rawDataVec.resize(PING_SIZE_MAX);
                if (cursor.channel2 == CHANNEL_NONE) {
                    epochData->getSonarFramePixel(cursor.channel1, cursor.subChannel1, rawDataVec);
                }

                /*- 灵敏度滤波 -*/
                int sens = 95 - sensLevel_ * 10;
                for(int i = 0; (i < btStart) && (i < pingSize) && (i < height); i++) {
                    if(rawDataVec[i] < sens) {
                        rawDataVec[i] = 0;
                    }
                }

                for(int i = btStart; (i < pingSize) && (i < height); i++) {
                    if(rawDataVec[i] < (sens * 0.5)) {
                        rawDataVec[i] = 0;
                    }
                }

                QVector<uint8_t> cacheData;
                for(int i = 0;((i < draft) && (i < height)); i++) {
                    cacheData.append(0);
                }
                for(int i = draft;((i < (pingSize + draft)) && (i < height)); i++) {
                    cacheData.append((quint8)rawDataVec[i-draft]);
                }
                for(int i = (pingSize+draft); i < height; i++) {
                    cacheData.append(0);
                }


                float scaleY = (float)height / pingSize * (loRng/(currentLoRng_ - currentUpRng_));
                startIdx = pingSize * currentUpRng_ / loRng;


                QList<int> colorData;
                colorData.clear();

                int colorNum = 1;
                if(colorNum == 1) {
                    /*-水表-*/
                    for(int j = 0; (j<sfEnd)&&(j<btStart)&&(j<height); j++) {
                        if(cacheData[j] == 0) {
                            colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
                        } else {
                            if((cacheData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
                                colorData.append(ZyColorScheme::colorScheme_surface[254]);
                            } else if((cacheData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
                                colorData.append(ZyColorScheme::colorScheme_surface[0]);
                            } else {
                                colorData.append(ZyColorScheme::colorScheme_surface[cacheData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
                            }
                        }
                    }
                    /*-水中-*/
                    for(int j = sfEnd; ((j<btStart)&&(j<height)); j++)
                    {
                        if(cacheData[j] == 0) {
                            colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
                        }
                        else
                        {
                            if((cacheData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254)
                            {
                                colorData.append(ZyColorScheme::colorScheme_fish[254]);
                            }
                            else if((cacheData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0)
                            {
                                colorData.append(ZyColorScheme::colorScheme_fish[0]);
                            }
                            else
                            {
                                colorData.append(ZyColorScheme::colorScheme_fish[cacheData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
                            }
                        }

                    }
                    /*-水底-*/
                    for(int j = btStart; j < height; j++)
                    {
                        if(cacheData[j] == 0)
                        {
                            colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
                        }
                        else
                        {
                            if((cacheData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254)
                            {
                                colorData.append(ZyColorScheme::colorScheme_bottom[254]);
                            }
                            else if((cacheData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0)
                            {
                                colorData.append(ZyColorScheme::colorScheme_bottom[0]);
                            }
                            else
                            {
                                colorData.append(ZyColorScheme::colorScheme_bottom[cacheData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
                            }
                        }
                    }
                }


                uint32_t* img_data = (uint32_t*)_image.bits();
                int bytesPerLine   = _image.bytesPerLine() / 4;

                if(scaleY < 1 && scaleY > 0) {
                    scaleY = 1 / scaleY;

                    int j = 0;
                    for(; (((int)(j*scaleY)+startIdx) < cacheData.count()); j++) {
                        int rgb = colorData[startIdx+(int)(j*scaleY)];
                        QRgb color = qRgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
                        img_data[j* bytesPerLine + column] = color;
                    }
                    /*-自动补齐-*/
                    for(; j<height; j++) {
                        /*-底层的部分设置成1不使用透明模式-*/
                        int rgb = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
                        img_data[j* bytesPerLine + column] = qRgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
                    }
                }
                else if(scaleY >= 1) {
                    for (int j = 0; j < height; j++) {
                        int rgb = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
                        QRgb color = qRgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
                        img_data[j* bytesPerLine + column] = color;
                    }

                    for(int j = 0; ((j<height)&&(btStart+(int)(j/scaleY)<colorData.count())); j++) {
                        int rgb = colorData[startIdx+(int)(j/scaleY)];
                        QRgb color = qRgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
                        img_data[j * bytesPerLine + column] = color;
                    }
                }




                // uint8_t* cacheData = new uint8_t[height];
                // memset(cacheData, 0, height * sizeof(uint8_t));
                // stretchCompressPixel(rawDataVec, cacheData, height, scaleY, startIdx);

                // _cash[column].waveData = QVector<uint8_t>(cacheData, cacheData + height);
                _cash[column].waveData = cacheData;
                _cash[column].sfEnd    = sfEnd;
                _cash[column].btStart  = btStart;
                _cash[column].bottomLineIdx = _cash[column].btStart * scaleY;
                _cash[column].state    = CashLine::CashState::CashStateValid;
                _cash[column].isNeedUpdate = true;
                _cash[column].heading  = params.heading;
                _cash[column].depth    = params.depth;
                _cash[column].speed    = params.speed;
                _cash[column].temperature = params.temperature;
                _cash[column].longitude = params.longitude;
                _cash[column].latitude = params.latitude;




                // uint32_t* img_data = (uint32_t*)_image.bits();
                // int bytesPerLine   = _image.bytesPerLine() / 4;
                // for (int j = 0; j < height; j++) {
                //     uint8_t dataValue = cacheData[j];
                //     int bgColor = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
                //     int rgb = bgColor;
                //     // qDebug() << "dataValue....." << dataValue;

                //     if (j >= 0 && j < frameSfEnd) {
                //         // 水表
                //         if(dataValue == 0) {
                //             rgb = bgColor;
                //         } else {
                //             if(dataValue + ZyColorScheme::colorLine * COLOR_LINE > 254) {
                //                 rgb = ZyColorScheme::colorScheme_surface[254];
                //             } else if(dataValue + ZyColorScheme::colorLine * COLOR_LINE < 0) {
                //                 rgb = ZyColorScheme::colorScheme_surface[0];
                //             } else {
                //                 rgb =  ZyColorScheme::colorScheme_surface[dataValue + ZyColorScheme::colorLine * COLOR_LINE];
                //             }
                //         }
                //     }
                //     else if (j >= frameSfEnd && j < frameBtStart) {
                //         // 水中
                //         if(dataValue == 0) {
                //             rgb = bgColor;
                //         } else {
                //             if(dataValue + ZyColorScheme::colorLine * COLOR_LINE > 254) {
                //                 rgb = ZyColorScheme::colorScheme_fish[254];
                //             } else if(dataValue + ZyColorScheme::colorLine * COLOR_LINE < 0) {
                //                 rgb = ZyColorScheme::colorScheme_fish[254];
                //             } else {
                //                 rgb = ZyColorScheme::colorScheme_fish[dataValue + ZyColorScheme::colorLine * COLOR_LINE];
                //             }
                //         }
                //     }
                //     else if(j >= frameBtStart) {
                //         // 水底
                //         if(dataValue == 0) {
                //             rgb = bgColor;
                //         } else {
                //             if(dataValue + ZyColorScheme::colorLine * COLOR_LINE > 254) {
                //                 rgb = ZyColorScheme::colorScheme_bottom[254];
                //             } else if(dataValue + ZyColorScheme::colorLine * COLOR_LINE < 0) {
                //                 rgb = ZyColorScheme::colorScheme_bottom[254];
                //             } else {
                //                 rgb = ZyColorScheme::colorScheme_bottom[dataValue + ZyColorScheme::colorLine * COLOR_LINE];
                //             }
                //         }
                //     }

                //     QRgb color = qRgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
                //     img_data[j* bytesPerLine + column] = color;
                // }

                // delete[] cacheData;
                // cacheData = nullptr;
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
        wavePixel_ = _cash[visualRightColumn];
    }

    _lastCursor = cursor;

    return wrapStartPos;
}

bool Plot2DEchogram::draw(Plot2D* parent, Dataset* dataset)
{
    Canvas& canvas = parent->canvas();
    DatasetCursor& cursor = parent->cursor();

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
                p.drawImage(cash_col_1, 0, _image, cash_col_1, 0, cashUpdateWidth, image_height, Qt::ThresholdDither);
            }
            else {
                cash_col++;
            }
        }

        _flagColorChanged = false;

        canvas.painter()->drawPixmap(0, 0, _pixmap, cash_position, 0, image_width - cash_position, image_height);
        canvas.painter()->drawPixmap(image_width - cash_position, 0, _pixmap, 0, 0, cash_position, image_height);

        int wavePanelWidth = image_width / (WAVE_WIDTH_RATIO_DENOM - 1);
        if(wavePanelWidth < 1) {
            wavePanelWidth = 1;
        }
        drawLatestWavePixel(parent, image_width, wavePanelWidth, image_height);

        drawBottomLine(canvas, image_width, cash_position, bottomLineVisible_);
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
