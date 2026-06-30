#include "plot2D_echogram.h"
#include "plot2D.h"

#include <QApplication>
#include <QSettings>
#include <QtGlobal>


int Plot2DEchogram::colorScheme_surface[255] = {0};
int Plot2DEchogram::colorScheme_fish[255]    = {0};
int Plot2DEchogram::colorScheme_bottom[255]  = {0};

Plot2DEchogram::Plot2DEchogram()
{
    setThemeId(ClassicTheme);
    setLevels(10, 100);
    _colorSchemeType  = 0;
    _useCustomScheme  = false;
    _customSchemePath = QString();


    // 加载 ZyColorScheme 的静态配色数组
    QString fileName = qApp->applicationDirPath() + "/dcs_caise.tcs";
    zyColorScheme_ = new ZyColorScheme;
    zyColorScheme_->loadColorScheme(fileName);
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
    updateColors();
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

    updateColors();
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



void Plot2DEchogram::updateColors()
{
    cntt_++;
    // qDebug() << "void Plot2DEchogram::updateColors()..........." << cntt_;
    _colorLevels.resize(256);

    int colorOffset = ZyColorScheme::colorLine * COLOR_LINE;

    for(int i = 0; i < 256; i++) {
        if(i == 0) {
            _colorLevels[i] = qRgb((ZyColorScheme::background[ZyColorScheme::backgroundIndex] >> 16) & 0xFF,
                            (ZyColorScheme::background[ZyColorScheme::backgroundIndex] >> 8) & 0xFF,
                            ZyColorScheme::background[ZyColorScheme::backgroundIndex] & 0xFF);
        } else {
            int colorIndex = i + colorOffset;

            if(colorIndex > 254) {
                colorIndex = 254;
            }
            else if(colorIndex < 0) {
                colorIndex = 0;
            }

            if(colorIndex > 0 && colorIndex <= 30) {
                _colorSchemeType = 0;
            }
            else if(colorIndex >100 && colorIndex <= 180) {
                _colorSchemeType = 1;
            }
            else {
                _colorSchemeType = 2;
            }

            int colorValue = 0;
            // qDebug() << "_colorSchemeType........" << _colorSchemeType;
            switch(_colorSchemeType) {
                case 0:
                    colorValue = ZyColorScheme::colorScheme_surface[colorIndex];
                    break;
                case 1:
                    colorValue = ZyColorScheme::colorScheme_fish[colorIndex];
                    break;
                case 2:
                    colorValue = ZyColorScheme::colorScheme_bottom[colorIndex];
                    break;
                default:
                    break;
            }

            _colorLevels[i] = qRgb((colorValue >> 16) & 0xFF,(colorValue >> 8) & 0xFF, colorValue & 0xFF);
        }
    }

    _flagColorChanged = true;

    if(!_image.isNull()) {
        _image.setColorTable(_colorLevels);
    }
}

void Plot2DEchogram::resetCash()
{
    _cashFlags.resetCash = true;
}

void Plot2DEchogram::addReRenderPlotIndxs(const QSet<int> &indxs)
{
    reRenderPlotIndxs_.unite(indxs);
}


void Plot2DEchogram::stretchCompressPixel(QVector<uint8_t> &rawDataVec, uint8_t* dist, int distLen, float scale)
{
    if(dist == nullptr || distLen <= 0 || scale < 0) {
        return;
    }

    int srcLen = rawDataVec.size();

    if(scale > 1.0f) {
        for (int i = 0; i < distLen; i++) {
            float srcPos = i / scale;
            int srcIdx = (int)srcPos;
            if (srcIdx >= 0 && srcIdx < srcLen) {
                if (srcIdx + 1 < srcLen) {
                    float coef = srcPos - floorf(srcPos);
                    dist[i] = (uint8_t)(rawDataVec[srcIdx] * (1.0f - coef) + rawDataVec[srcIdx + 1] * coef);
                } else {
                    dist[i] = rawDataVec[srcIdx];
                }
            } else {
                dist[i] = 0;
            }
        }
    }
    else {
        scale = 1.0 / scale;
        for (int i = 0; i < distLen; i++) {
            int srcStart = (int)(i * scale);
            int srcEnd = (int)((i + 1) * scale);
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


// int Plot2DEchogram::updateCash(Plot2D* parent, Dataset* dataset, int width, int height)
// {
//     auto& cursor = parent->cursor();

//     if (_cash.size() != width) {
//         _cash.resize(width);
//         resetCash();
//     }

//     uint8_t* image_data = (uint8_t*)_image.constBits();
//     const int b_scanline = _image.bytesPerLine();

//     bool is_cash_notvalid = getTriggerCashReset();
//     is_cash_notvalid |= !_lastCursor.isChannelsEqual(cursor);
//     is_cash_notvalid |= !_lastCursor.isDistanceEqual(cursor);
//     is_cash_notvalid |=  _lastWidth != width;
//     is_cash_notvalid |=  _lastHeight != height;

//     float from = cursor.distance.from;
//     float to = cursor.distance.to;
//     float fullrange = to - from;

//     float range1 = 0;
//     float from1 = 0;
//     float to1 = 0;

//     float from2 = 0;
//     float to2 = 0;

//     if (to >= 0) {
//         range1 = 0 - from;
//         from1 = 0;
//         to1 = -from;

//         if (from >= 0) {
//             from2 = from;
//         }
//         else {
//             from2 = 0;
//         }
//         to2 = to;
//     }
//     else {
//         range1 = to - from;
//         from1 = -to;
//         to1 = -from;
//     }

//     int wrap_start_pos = qAbs(cursor.getIndex(0) % width);

//     for (unsigned int i = 0; i < cursor.indexes.size(); i++) {
//         if (cursor.indexes[i] > 0) {
//             wrap_start_pos = qAbs((cursor.indexes[i] + (width - i)) % width);
//             break;
//         }
//     }


//     for(int column = 0; column < width; column++) {
//         if(_cash[column].data.size() != height) {
//             _cash[column].state = CashLine::CashState::CashStateNotValid;
//             _cash[column].data.resize(height);
//             _cash[column].poolIndex = -1;
//             _cash[column].state = CashLine::CashState::CashStateEraced;
//             _cash[column].isNeedUpdate = true;

//             int16_t cash_data_size = _cash[column].data.size();
//             int16_t* cash_data = _cash[column].data.data();
//             uint8_t * img_data = image_data + column;
//             for (int image_row = 0; image_row < cash_data_size; image_row++) {
//                 *img_data = *cash_data;
//                 img_data += b_scanline;
//                 cash_data++;
//             }
//         }

//         int cursor_pos = column - wrap_start_pos;
//         if(column < wrap_start_pos) {
//             cursor_pos += width;
//         }

//         int pool_index = cursor.getIndex(cursor_pos);
//         int pool_index_safe = dataset->validIndex(pool_index);
//         if(pool_index_safe >= 0) {

//             bool wasValidlyRendered = true;
//             if (reRenderPlotIndxs_.contains(pool_index_safe)) {
//                 reRenderPlotIndxs_.remove(pool_index_safe);
//                 wasValidlyRendered = false;
//             }

//             auto* datasource = dataset->fromIndex(pool_index_safe);
//             const int cash_index = _cash[column].poolIndex;

//             if (is_cash_notvalid || pool_index_safe != cash_index || !wasValidlyRendered) {
//                 _cash[column].poolIndex = pool_index_safe;

//                 if(datasource != NULL) {
//                     _cash[column].state = CashLine::CashState::CashStateNotValid;
//                     int16_t* cash_data = _cash[column].data.data();
//                     int16_t cash_data_size = _cash[column].data.size();

//                     if (cursor.channel2 == CHANNEL_NONE) {
//                         datasource->chartTo(cursor.channel1, cursor.subChannel1, from, to, cash_data, cash_data_size, _compensation_id);
//                     }
//                     else {
//                         int cash_data_size_part1 = cash_data_size*(range1/fullrange);

//                         if(cash_data_size_part1 > 0) {
//                             datasource->chartTo(cursor.channel1, cursor.subChannel1, from1, to1, cash_data, cash_data_size_part1, _compensation_id, true);
//                         }

//                         if(cash_data_size_part1 < 0) {
//                             cash_data_size_part1 = 0;
//                         }

//                         const int cash_data_size_part2 = cash_data_size - cash_data_size_part1;
//                         if(cash_data_size_part2 > 0) {
//                             datasource->chartTo(cursor.channel2, cursor.subChannel2, from2, to2, &cash_data[cash_data_size_part1], cash_data_size_part2, _compensation_id, false);
//                         }
//                     }

//                     _cash[column].state = CashLine::CashState::CashStateValid;
//                     _cash[column].isNeedUpdate = true;
//                     uint8_t * img_data = image_data + column;
//                     for (int image_row = 0; image_row < cash_data_size; image_row++) {
//                         *img_data = *cash_data;
//                         img_data += b_scanline;
//                         cash_data++;
//                     }
//                 }
//                 else {
//                     if(_cash[column].state != CashLine::CashState::CashStateEraced) {
//                         _cash[column].state = CashLine::CashState::CashStateNotValid;
//                         _cash[column].data.fill(0);
//                         _cash[column].poolIndex = -1;
//                         _cash[column].state = CashLine::CashState::CashStateEraced;
//                         _cash[column].isNeedUpdate = true;

//                         int16_t cash_data_size = _cash[column].data.size();
//                         int16_t* cash_data = _cash[column].data.data();
//                         uint8_t * img_data = image_data + column;
//                         for (int image_row = 0; image_row < cash_data_size; image_row++) {
//                             *img_data = *cash_data;
//                             img_data += b_scanline;
//                             cash_data++;
//                         }
//                     }
//                 }

//             }
//         } else {
//             if(_cash[column].state != CashLine::CashState::CashStateEraced) {
//                 _cash[column].state = CashLine::CashState::CashStateNotValid;
//                 _cash[column].data.fill(0);
//                 _cash[column].poolIndex = -1;
//                 _cash[column].state = CashLine::CashState::CashStateEraced;
//                 _cash[column].isNeedUpdate = true;

//                 int16_t* cash_data = _cash[column].data.data();
//                 int16_t cash_data_size = _cash[column].data.size();
//                 uint8_t * img_data = image_data + column;
//                 for (int image_row = 0; image_row < cash_data_size; image_row++) {
//                     *img_data = *cash_data;
//                     img_data += b_scanline;
//                     cash_data++;
//                 }
//             }
//         }
//     }

//     _lastCursor = cursor;
//     _lastWidth = width;
//     _lastHeight = height;

//     return wrap_start_pos;
// }
int Plot2DEchogram::updateCache(Plot2D* parent, Dataset* dataset, int width, int height)
{
    int sfEnd, btStart;
    // qDebug() << "::updateCash.......width:" << width << " height:" << height << " sfEnd:" << sfEnd << "  btStart:" << btStart;
    DatasetCursor& cursor = parent->cursor();
    if (_cash.size() != width) {
        _cash.resize(width);
        resetCash();
    }

    bool isCashNotvalid = getTriggerCashReset();
    isCashNotvalid |= !_lastCursor.isChannelsEqual(cursor);
    isCashNotvalid |= !_lastCursor.isDistanceEqual(cursor);
    isCashNotvalid |= _lastWidth != width;
    isCashNotvalid |= _lastHeight != height;


    float from = cursor.distance.from;
    float to   = cursor.distance.to;
    float fullrange = to - from;

    float range1 = 0;
    float from1 = 0;
    float to1 = 0;

    float from2 = 0;
    float to2 = 0;

    if (to >= 0) {
        range1 = 0 - from;
        from1 = 0;
        to1 = -from;

        if (from >= 0) {
            from2 = from;
        }
        else {
            from2 = 0;
        }
        to2 = to;
    }
    else {
        range1 = to - from;
        from1 = -to;
        to1 = -from;
    }

    int wrapStartPos = qAbs(cursor.getIndex(0) % width);

    for (unsigned int i = 0; i < cursor.indexes.size(); i++) {
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

            Epoch* epochData = dataset->fromIndex(pool_index_safe);
            if(epochData == nullptr) {
                return -1;
            }
            const int cacheIndex = _cash[column].poolIndex;

            ChartParameters params = epochData->getChartParameters(cursor.channel1);
            float frameLoRng  = params.loRng;
            const int  pingSize   =  240;
            int frameSfEnd = params.sfEnd;
            int  frameBtStart = params.btStart;

            if (isCashNotvalid || pool_index_safe != cacheIndex || !wasValidlyRendered) {
                _cash[column].poolIndex = pool_index_safe;

                // 先获取原始长度的声呐数据
                QVector<uint8_t> rawDataVec;
                rawDataVec.resize(PING_SIZE_MAX);
                if (cursor.channel2 == CHANNEL_NONE) {
                    // epochData->getSonarFrameData(cursor.channel1, cursor.subChannel1, rawData, PING_SIZE_MAX);
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
                float scaleY = (float)height / pingSize * (frameLoRng/(1500.0 - 0.0));
                // qDebug() << "scaleY......" << scaleY;
                scaleY *= 0.4;
                stretchCompressPixel(rawDataVec, cacheData, height, scaleY);

                _cash[column].state = CashLine::CashState::CashStateValid;
                _cash[column].isNeedUpdate = true;

                uint32_t* img_data = (uint32_t*)_image.bits();
                int bytesPerLine = _image.bytesPerLine() / 4;
                for (int image_row = 0; image_row < height; image_row++) {
                    uint8_t dataValue = cacheData[image_row];
                    int bgColor = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
                    int rgb = bgColor;
                    // qDebug() << "dataValue....." << dataValue;

                     if (image_row >= 0 && image_row < frameSfEnd) {
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
                      else if (image_row >= frameSfEnd && image_row < frameBtStart) {
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
                     else if(image_row >= frameBtStart){
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
                    img_data[image_row* bytesPerLine + column] = color;
                }

                delete[] cacheData;
                cacheData = nullptr;

            }
        }

        else {
            if(_cash[column].state != CashLine::CashState::CashStateEraced) {
                _cash[column].data.fill(0);
                _cash[column].poolIndex = -1;
                _cash[column].state = CashLine::CashState::CashStateEraced;
                _cash[column].isNeedUpdate = true;

                // int16_t* cash_data = _cash[column].data.data();
            //     // int16_t cash_data_size = _cash[column].data.size();
            //     int cashDataSize = height;
            //     uint8_t* cashData = new uint8_t[cashDataSize];
            //     memset(cashData, 0, cashDataSize * sizeof(uint8_t));
            //     uint32_t* img_data = (uint32_t*)_image.bits();
            //     int bytesPerLine = _image.bytesPerLine() / 4;
            //     qDebug() << "int 222222bytesPerLine...." << bytesPerLine;

            //     for (int image_row = 0; image_row < cashDataSize; image_row++) {
            //         uint8_t dataValue = cashData[image_row];
            //         int bgColor = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
            //         int rgb = bgColor;

            //         // 根据深度区域选择配色
            //         if (image_row >= 0 && image_row < sfEnd) {
            //             // 水表
            //             if(dataValue == 0) {
            //                 rgb = bgColor;
            //             }
            //             else {
            //                 if(dataValue + ZyColorScheme::colorLine * COLOR_LINE > 254) {
            //                     rgb = ZyColorScheme::colorScheme_surface[254];
            //                 } else if(dataValue + ZyColorScheme::colorLine * COLOR_LINE < 0) {
            //                     rgb = ZyColorScheme::colorScheme_surface[0];
            //                 } else {
            //                     rgb =  ZyColorScheme::colorScheme_surface[dataValue + ZyColorScheme::colorLine * COLOR_LINE];
            //                 }
            //             }

            //         }
            //         else if (image_row >= sfEnd && image_row < btStart) {
            //             // 水中
            //             if(dataValue == 0) {
            //                 rgb = bgColor;
            //             }
            //             else {
            //                 if(dataValue + ZyColorScheme::colorLine * COLOR_LINE > 254) {
            //                     rgb = ZyColorScheme::colorScheme_fish[254];
            //                 } else if(dataValue + ZyColorScheme::colorLine * COLOR_LINE < 0) {
            //                     rgb = ZyColorScheme::colorScheme_fish[254];
            //                 } else {
            //                     rgb = ZyColorScheme::colorScheme_fish[dataValue + ZyColorScheme::colorLine * COLOR_LINE];
            //                 }
            //             }
            //         }
            //         else if(image_row >= btStart) {
            //             // 水底
            //             if(dataValue == 0) {
            //                 rgb = bgColor;
            //             }
            //             else {
            //                 if(dataValue + ZyColorScheme::colorLine * COLOR_LINE > 254) {
            //                     rgb = ZyColorScheme::colorScheme_bottom[254];
            //                 } else if(dataValue + ZyColorScheme::colorLine * COLOR_LINE < 0) {
            //                     rgb = ZyColorScheme::colorScheme_bottom[254];
            //                 } else {
            //                     rgb = ZyColorScheme::colorScheme_bottom[dataValue + ZyColorScheme::colorLine * COLOR_LINE];
            //                 }
            //             }
            //         }

            //         QRgb color = qRgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
            //         img_data[image_row * bytesPerLine + column] = color;
            //     }

            //     delete[] cashData;
            //     cashData = nullptr;


                uint32_t* img_data = (uint32_t*)_image.bits();
                int bytesPerLine = _image.bytesPerLine() / 4;
                int bgColor = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
                QRgb bgRgb = qRgb((bgColor >> 16) & 0xFF, (bgColor >> 8) & 0xFF, bgColor & 0xFF);

                // 直接用背景色填充整列，不依赖未初始化的 sfEnd/btStart
                for (int image_row = 0; image_row < height; image_row++) {
                    img_data[image_row * bytesPerLine + column] = bgRgb;
                }
            }
        }
    }

    _lastCursor = cursor;
    _lastWidth = width;
    _lastHeight = height;

    return wrapStartPos;
}

bool Plot2DEchogram::draw(Plot2D* parent, Dataset* dataset)
{
    auto& canvas = parent->canvas();
    auto& cursor = parent->cursor();

    _colorLevels.clear();

    if (isVisible() && dataset != nullptr && cursor.distance.isValid()) {
        const int image_width  = canvas.width();
        const int image_height = canvas.height();

        if(_image.width() != image_width || _image.height() != image_height) {
            _image  = QImage(image_width, image_height, QImage::Format_RGB32);
            _pixmap = QPixmap(image_width, image_height);
        }

        // qDebug() << "........upRng:" << upRng << " loRng:" << loRng << "currentEpochIndex:" << currentEpochIndex << "  sfEnd:" << sfEnd << " btStart:" << btStart;
        const int cash_width = canvas.width();
        // const int cash_position = updateCash(parent, dataset, cash_width, image_height, sfEnd, btStart);

        // 先初始化缓存尺寸/状态（不传 sfEnd/btStart，交给每列计算）
        const int cash_position = updateCache(parent, dataset, cash_width, image_height);

        QPainter p(&_pixmap);

        int cash_col = 0;
        while(cash_col < cash_width) {
            int cash_col_1 = cash_col;
            while(cash_col < cash_width && (_cash[cash_col].isNeedUpdate || _flagColorChanged)) {
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

        canvas.painter()->drawPixmap(0, 0, _pixmap, cash_position, 0, cash_width - cash_position, image_height);
        canvas.painter()->drawPixmap(cash_width - cash_position, 0, _pixmap, 0, 0, cash_position, image_height);
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
