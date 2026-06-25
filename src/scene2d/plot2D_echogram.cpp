#include "plot2D_echogram.h"
#include "plot2D.h"

#include <QApplication>
#include <QSettings>


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

    QString defaultPath = qApp->applicationDirPath() + "/dcs_caise.tcs";

    // 加载 ZyColorScheme 的静态配色数组
    ZyColorScheme::loadColorScheme(defaultPath);
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

            int red = qRound(coloros[i].red()*a_koef + coloros[i + 1].red()*b_koef);
            int green = qRound(coloros[i].green()*a_koef + coloros[i + 1].green()*b_koef);
            int blue = qRound(coloros[i].blue()*a_koef + coloros[i + 1].blue()*b_koef);
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

// int Plot2DEchogram::updateCash(Plot2D* parent, Dataset* dataset, int width, int height)
// {
//     qDebug() << "int Plot2DEchogram::updateCash......." << width << " " << height;
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
int Plot2DEchogram::updateCash(Plot2D* parent, Dataset* dataset, int width, int height, int sfEnd, int btStart)
{
    // qDebug() << "int Plot2DEchogram::updateCash......." << width << " " << height;
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

    // isCashNotvalid = true;

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
        if(_cash[column].data.size() != height) {
            _cash[column].data.resize(height);
            _cash[column].data.fill(0);
            _cash[column].poolIndex = -1;
            _cash[column].state = CashLine::CashState::CashStateEraced;
            _cash[column].isNeedUpdate = true;

            int16_t cashDataSize = _cash[column].data.size();
            int16_t* cash_data   = _cash[column].data.data();
            uint32_t* img_data   = (uint32_t*)_image.bits();
            int bytesPerLine     = _image.bytesPerLine() / 4;
            for (int image_row = 0; image_row < cashDataSize; image_row++) {
                uint8_t dataValue = static_cast<uint8_t>(*cash_data);
                int bgColor = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
                QRgb color = qRgb((bgColor >> 16) & 0xFF, (bgColor >> 8) & 0xFF, bgColor & 0xFF);
                // qDebug() << "dataValue....." << dataValue;

                if(dataValue == 0) {
                    int bgColor = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
                    color = qRgb((bgColor >> 16) & 0xFF, (bgColor >> 8) & 0xFF, bgColor & 0xFF);
                }
                else {
                    // qDebug() << "choose color depend on region of depth....";
                    // 根据深度区域选择配色
                    if (image_row >= 0 && image_row < sfEnd) {
                        // 水表区域 - 使用 surface 配色
                        int colorIndex = dataValue + ZyColorScheme::colorLine * COLOR_LINE;
                        colorIndex = qBound(0, colorIndex, 254);
                        int rgb = ZyColorScheme::colorScheme_surface[colorIndex];
                        color = qRgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
                    }
                    else if (image_row >= sfEnd && image_row < btStart) {
                        // 水中区域 - 使用 fish 配色
                        int colorIndex = dataValue + ZyColorScheme::colorLine * COLOR_LINE;
                        colorIndex = qBound(0, colorIndex, 254);
                        int rgb = ZyColorScheme::colorScheme_fish[colorIndex];
                        color = qRgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
                    }
                    else if(image_row >= btStart){
                        // 水底区域 - 使用 bottom 配色
                        int colorIndex = dataValue + ZyColorScheme::colorLine * COLOR_LINE;
                        colorIndex = qBound(0, colorIndex, 254);
                        int rgb = ZyColorScheme::colorScheme_bottom[colorIndex];
                        color = qRgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
                    }
                }

                img_data[image_row * bytesPerLine + column] = color;
                cash_data++;
            }
        }

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
            const int cash_index = _cash[column].poolIndex;

            if (isCashNotvalid || pool_index_safe != cash_index || !wasValidlyRendered) {
                _cash[column].poolIndex = pool_index_safe;

                if(epochData != NULL) {
                    int16_t* cash_data     = _cash[column].data.data();
                    int16_t cash_data_size = _cash[column].data.size();

                    if (cursor.channel2 == CHANNEL_NONE) {
                        // qDebug() << "cash_data_size............" << cash_data_size;
                        epochData->chartTo(cursor.channel1, cursor.subChannel1, from, to, cash_data, cash_data_size, _compensation_id);
                    }
                    else {
                        int cash_data_size_part1 = cash_data_size*(range1/fullrange);
                        qDebug() << "cash_data_size_part1........." << cash_data_size_part1;

                        if(cash_data_size_part1 > 0) {
                            epochData->chartTo(cursor.channel1, cursor.subChannel1, from1, to1, cash_data, cash_data_size_part1, _compensation_id, true);
                        }

                        if(cash_data_size_part1 < 0) {
                            cash_data_size_part1 = 0;
                        }

                        const int cash_data_size_part2 = cash_data_size - cash_data_size_part1;
                        if(cash_data_size_part2 > 0) {
                            epochData->chartTo(cursor.channel2, cursor.subChannel2, from2, to2, &cash_data[cash_data_size_part1], cash_data_size_part2, _compensation_id, false);
                        }
                    }

                    _cash[column].state = CashLine::CashState::CashStateValid;
                    _cash[column].isNeedUpdate = true;
                    uint32_t* img_data = (uint32_t*)_image.bits();
                    int bytesPerLine = _image.bytesPerLine() / 4;
                    for (int image_row = 0; image_row < cash_data_size; image_row++) {
                        uint8_t dataValue  =static_cast<uint8_t>(*cash_data);
                        int bgColor = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
                        QRgb color = qRgb((bgColor >> 16) & 0xFF, (bgColor >> 8) & 0xFF, bgColor & 0xFF);

                        // 根据深度区域选择配色,只有数据值不为0时才应用配色
                        if (dataValue > 0) {
                            // 根据深度区域选择配色
                            if (image_row >= 0 && image_row < sfEnd) {
                                // 水表区域 - 使用 surface 配色
                                int colorIndex = dataValue + ZyColorScheme::colorLine * COLOR_LINE;
                                colorIndex = qBound(0, colorIndex, 254);
                                int rgb = ZyColorScheme::colorScheme_surface[colorIndex];
                                color = qRgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
                            }
                            else if (image_row >= sfEnd && image_row < btStart) {
                                // 水中区域 - 使用 fish 配色
                                int colorIndex = dataValue + ZyColorScheme::colorLine * COLOR_LINE;
                                colorIndex = qBound(0, colorIndex, 254);
                                int rgb = ZyColorScheme::colorScheme_fish[colorIndex];
                                color = qRgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
                            }
                            else if(image_row >= btStart){
                                // 水底区域 - 使用 bottom 配色
                                int colorIndex = dataValue + ZyColorScheme::colorLine * COLOR_LINE;
                                colorIndex = qBound(0, colorIndex, 254);
                                int rgb = ZyColorScheme::colorScheme_bottom[colorIndex];
                                color = qRgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
                            }
                        }

                        img_data[image_row * bytesPerLine + column] = color;
                        cash_data++;
                    }
                }
                else {
                    if(_cash[column].state != CashLine::CashState::CashStateEraced) {
                        _cash[column].data.fill(0);
                        _cash[column].poolIndex = -1;
                        _cash[column].state = CashLine::CashState::CashStateEraced;
                        _cash[column].isNeedUpdate = true;

                        int16_t cash_data_size = _cash[column].data.size();
                        int16_t* cash_data = _cash[column].data.data();
                        uint32_t* img_data = (uint32_t*)_image.bits();
                        int bytesPerLine = _image.bytesPerLine() / 4;
                        qDebug() << "bytesPerLine......" << bytesPerLine;
                        for (int image_row = 0; image_row < cash_data_size; image_row++) {
                            uint8_t dataValue = static_cast<uint8_t>(*cash_data);
                            QRgb color;

                            // 根据深度区域选择配色
                            if (sfEnd >= 0 && image_row <= sfEnd) {
                                int colorIndex = dataValue + ZyColorScheme::colorLine * COLOR_LINE;
                                colorIndex = qBound(0, colorIndex, 254);
                                int rgb = ZyColorScheme::colorScheme_surface[colorIndex];
                                color = qRgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
                            } else if (btStart >= 0 && image_row < btStart) {
                                int colorIndex = dataValue + ZyColorScheme::colorLine * COLOR_LINE;
                                colorIndex = qBound(0, colorIndex, 254);
                                int rgb = ZyColorScheme::colorScheme_fish[colorIndex];
                                color = qRgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
                            } else {
                                int colorIndex = dataValue + ZyColorScheme::colorLine * COLOR_LINE;
                                colorIndex = qBound(0, colorIndex, 254);
                                int rgb = ZyColorScheme::colorScheme_bottom[colorIndex];
                                color = qRgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
                            }

                            img_data[image_row * bytesPerLine + column] = color;
                            cash_data++;
                        }
                    }
                }

            }else {
                // qDebug() << "isCashNotvalid:" << isCashNotvalid << "  wasValidlyRendered" <<wasValidlyRendered;
            }
        } else {
            if(_cash[column].state != CashLine::CashState::CashStateEraced) {
                _cash[column].data.fill(0);
                _cash[column].poolIndex = -1;
                _cash[column].state = CashLine::CashState::CashStateEraced;
                _cash[column].isNeedUpdate = true;

                int16_t* cash_data = _cash[column].data.data();
                int16_t cash_data_size = _cash[column].data.size();
                uint32_t* img_data = (uint32_t*)_image.bits();
                int bytesPerLine = _image.bytesPerLine() / 4;
                qDebug() << "int 222222bytesPerLine...." << bytesPerLine;
                for (int image_row = 0; image_row < cash_data_size; image_row++) {
                    uint8_t dataValue = static_cast<uint8_t>(*cash_data);
                    int bgColor = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
                    QRgb color = qRgb((bgColor >> 16) & 0xFF, (bgColor >> 8) & 0xFF, bgColor & 0xFF);
                    if(dataValue > 0) {
                        if (image_row >= 0 && image_row < sfEnd) {
                            // 水表区域 - 使用 surface 配色
                            int colorIndex = dataValue + ZyColorScheme::colorLine * COLOR_LINE;
                            colorIndex = qBound(0, colorIndex, 254);
                            int rgb = ZyColorScheme::colorScheme_surface[colorIndex];
                            color = qRgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
                        }
                        else if (image_row >= sfEnd && image_row < btStart) {
                            // 水中区域 - 使用 fish 配色
                            int colorIndex = dataValue + ZyColorScheme::colorLine * COLOR_LINE;
                            colorIndex = qBound(0, colorIndex, 254);
                            int rgb = ZyColorScheme::colorScheme_fish[colorIndex];
                            color = qRgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
                        }
                        else if(image_row >= btStart){
                            // 水底区域 - 使用 bottom 配色
                            int colorIndex = dataValue + ZyColorScheme::colorLine * COLOR_LINE;
                            colorIndex = qBound(0, colorIndex, 254);
                            int rgb = ZyColorScheme::colorScheme_bottom[colorIndex];
                            color = qRgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
                        }
                    }

                    img_data[image_row * bytesPerLine + column] = color;
                    cash_data++;
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
    // qDebug() << "Plot2DEchogram::draw(............";
    auto& canvas = parent->canvas();
    auto& cursor = parent->cursor();

    _colorLevels.clear();

    updateColors();


    if (isVisible() && dataset != nullptr && cursor.distance.isValid()) {
        const int image_width  = canvas.width();
        const int image_height = canvas.height();

        if(_image.width() != image_width || _image.height() != image_height) {
            _image  = QImage(image_width, image_height, QImage::Format_RGB32);
            _pixmap = QPixmap(image_width, image_height);
        }

        // 设置颜色表
        // _image.setColorTable(_colorLevels);
        float depth = 0.0f;
        float loRng = 0.0f;
        float upRng = 0.0f;
        float sspd  = 1500.0f;
        int pingSize = 240;

        // int currentEpochIndex = cursor.getIndex(cursor.indexes.size() / 2);
        int currentEpochIndex = cursor.getIndex(cursor.indexes.size()-1);
        int validIndex = dataset->validIndex(currentEpochIndex);
        if (validIndex >= 0) {
            auto* currentEpoch = dataset->fromIndex(validIndex);
            if (currentEpoch) {
                ChartParameters params = currentEpoch->getChartParameters(cursor.channel1);
                depth    = params.depth;
                loRng    = params.loRng;
                upRng    = params.upRng;
                sspd     = params.sspd;
                pingSize = params.pingSize;
            }
        }


        if((upRng<0) || (loRng<0) || (pingSize<=0) || (upRng == loRng)) {
            StructSonarInfo sonarInfo;
            sonarInfo.draft = 0;
            sonarInfo.btStart = 0;
            sonarInfo.sfEnd = 0;
            return 0;
        }

        int sfEnd = 0, btStart = 0;
        if(loRng != 0) {
            if(loRng == upRng) {
                StructSonarInfo sonarInfo;
                sonarInfo.draft = 0;
                sonarInfo.btStart = 0;
                sonarInfo.sfEnd = 0;
                sonarInfo.startIdx = 0;
                sonarInfo.endIdx = 0;
                return 0;
            }

            // if(loRng > upRng) {
            //     cursor.distance.set(0, loRng * 0.1);
            // }

            btStart = (1500/sspd) *(depth /(loRng-upRng)) *pingSize;

            float surfaceEnd;
            if((depth < 100) && (depth > 30)) {
                surfaceEnd = depth-10;
            } else {
                surfaceEnd = 100;
            }
            sfEnd = (1500/sspd) *(surfaceEnd /(loRng-upRng)) *pingSize;
        }
        if((btStart < 0) || (sfEnd < 0) || (depth < 0)) {
            StructSonarInfo sonarInfo;
            sonarInfo.draft = 0;
            sonarInfo.btStart = 0;
            sonarInfo.sfEnd = 0;
            return 0;
        }

        qDebug() << "........upRng:" << upRng << " loRng:" << loRng << "currentEpochIndex:" << currentEpochIndex << "  sfEnd:" << sfEnd << " btStart:" << btStart;
        const int cash_width = canvas.width();
        // const int cash_position = updateCash(parent, dataset, cash_width, image_height);
        const int cash_position = updateCash(parent, dataset, cash_width, image_height, sfEnd, btStart);

        QPainter p(&_pixmap);

        int cash_col = 0;
        while(cash_col < cash_width) {
            int cash_col_1 = cash_col;
            while(cash_col < cash_width && (_cash[cash_col].isNeedUpdate || _flagColorChanged)) {
                _cash[cash_col].isNeedUpdate = false;
                cash_col++;
            }

            int cash_update_width = cash_col - cash_col_1;
            if(cash_update_width > 0) {
                p.drawImage(cash_col_1, 0, _image, cash_col_1, 0 , cash_update_width, image_height, Qt::ThresholdDither);
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
