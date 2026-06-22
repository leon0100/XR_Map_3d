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
    
    // 同时也加载 Plot2DEchogram 自己的配色数组（兼容性）
    readColorToColorList(defaultPath);
    getColorFromColorList();
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

void Plot2DEchogram::readColorToColorList(QString fileName)
{
    /*-用于存取自定义色表-*/
    QSettings selfColor(fileName, QSettings::IniFormat);
    StructColorList tmp;

    /*-自定义配色表读取-*/
    colorList_surface.clear();
    selfColor.beginGroup("surface");
    for(int i=0;i<selfColor.value("count",0).toInt();i++) {
        tmp.colorPosition = selfColor.value(("position_"+QString::number(i)), 0).toFloat();
        tmp.colorValue = selfColor.value("value_"+QString::number(i), 0).toInt();
        colorList_surface.append(tmp);
    }
    selfColor.endGroup();

    colorList_fish.clear();
    selfColor.beginGroup("fish");
    for(int i=0;i<selfColor.value("count",0).toInt();i++) {
        tmp.colorPosition = selfColor.value(("position_"+QString::number(i)), 0).toFloat();
        tmp.colorValue = selfColor.value("value_"+QString::number(i), 0).toInt();
        colorList_fish.append(tmp);
    }
    selfColor.endGroup();

    colorList_bottom.clear();
    selfColor.beginGroup("bottom");
    for(int i=0;i<selfColor.value("count",0).toInt();i++) {
        tmp.colorPosition = selfColor.value(("position_"+QString::number(i)), 0).toFloat();
        tmp.colorValue = selfColor.value("value_"+QString::number(i), 0).toInt();
        colorList_bottom.append(tmp);
    }
    selfColor.endGroup();
}

void Plot2DEchogram::getColorFromColorList()
{
    /*----表层渐变显示信号颜色图----*/
    QLinearGradient linearGradient_Surface(QPointF(0, 0), QPointF(0, 255));
    QLinearGradient linearGradient_Fish(QPointF(0, 0), QPointF(0, 255));
    QLinearGradient linearGradient_Bottom(QPointF(0, 0), QPointF(0, 255));
    QPixmap pixmap(100,256);
    QPainter painter;
    QImage GradientImage;

    /*-颜色链表中按顺序进行渐变-*/
    for(int i = 0; i<colorList_surface.count(); i++)
    {
        linearGradient_Surface.setColorAt(colorList_surface.at(i).colorPosition/255.0, colorList_surface.at(i).colorValue);
    }
    painter.begin(&pixmap);
    painter.setBrush(linearGradient_Surface);
    painter.drawRect(0, 0, 100, 256);
    painter.end();
    GradientImage = pixmap.toImage();
    for(int i=0;i<255;i++)
    {
        colorScheme_surface[i] = GradientImage.pixel(99,i+1)&0x00FFFFFF;
    }
    /*-颜色链表中按顺序进行渐变-*/
    for(int i = 0; i<colorList_fish.count(); i++)
    {
        linearGradient_Fish.setColorAt(colorList_fish.at(i).colorPosition/255.0, colorList_fish.at(i).colorValue);
    }
    painter.begin(&pixmap);
    painter.setBrush(linearGradient_Fish);
    painter.drawRect(0, 0, 100, 256);
    painter.end();
    GradientImage = pixmap.toImage();
    for(int i=0;i<255;i++)
    {
        colorScheme_fish[i] = GradientImage.pixel(99,i+1)&0x00FFFFFF;
    }
    /*-颜色链表中按顺序进行渐变-*/
    for(int i = 0; i<colorList_bottom.count(); i++)
    {
        linearGradient_Bottom.setColorAt(colorList_bottom.at(i).colorPosition/255.0, colorList_bottom.at(i).colorValue);
    }
    painter.begin(&pixmap);
    painter.setBrush(linearGradient_Bottom);
    painter.drawRect(0, 0, 100, 256);
    painter.end();
    GradientImage = pixmap.toImage();
    for(int i=0;i<255;i++)
    {
        colorScheme_bottom[i] = GradientImage.pixel(99,i+1)&0x00FFFFFF;
    }
}

QList<int> colorData;
void Plot2DEchogram::drawImagePixsel(int column, StructSonarInfo sonarInfo, float scale, int colorNum)
{
    // if(scale < 1) {
    //     scale = 1/scale;
    //     compressImagePixsel(column, sonarInfo, scale, colorNum, is2nd);
    // }
    // else {
    //     stretchImagePixsel(column, sonarInfo, scale, colorNum, is2nd);
    // }

    if(scale <= 0) {
        return;
    }

    colorData.clear();
    if(colorNum == 1) {
        /*-水表-*/
        for(int j = 0; (j<sonarInfo.sfEnd)&&(j<sonarInfo.btStart)&&(j<sonarImageHeight); j++) {
            if(sonarInfo.rawData[j] == 0) {
                colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
            } else {
                if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
                    colorData.append(ZyColorScheme::colorScheme_surface[254]);
                }
                else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
                    colorData.append(ZyColorScheme::colorScheme_surface[0]);
                }
                else {
                    colorData.append(ZyColorScheme::colorScheme_surface[sonarInfo.rawData[j]+ ZyColorScheme::colorLine*COLOR_LINE]);
                }
            }
        }
        /*-水中-*/
        for(int j = sonarInfo.sfEnd; ((j<sonarInfo.btStart)&&(j<sonarImageHeight)); j++)
        {
            if(sonarInfo.rawData[j] == 0) {
                colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
            }
            else
            {
                if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254)
                {
                    colorData.append(ZyColorScheme::colorScheme_fish[254]);
                }
                else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0)
                {
                    colorData.append(ZyColorScheme::colorScheme_fish[0]);
                }
                else
                {
                    colorData.append(ZyColorScheme::colorScheme_fish[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
                }
            }
        }
        /*-水底-*/
        for(int j = sonarInfo.btStart; j < sonarImageHeight; j++)
        {
            if(sonarInfo.rawData[j] == 0)
            {
                colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
            }
            else
            {
                if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254)
                {
                    colorData.append(ZyColorScheme::colorScheme_bottom[254]);
                }
                else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0)
                {
                    colorData.append(ZyColorScheme::colorScheme_bottom[0]);
                }
                else
                {
                    colorData.append(ZyColorScheme::colorScheme_bottom[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
                }
            }
        }
    }
    else if(colorNum == 2)
    {
        /*-水表-*/
        for(int j = 0; (j<sonarInfo.sfEnd)&&(j<sonarInfo.btStart)&&(j<sonarImageHeight); j++)
        {
            if(sonarInfo.rawData[j] == 0)
            {
                colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
            }
            else
            {
                if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254)
                {
                    colorData.append(ZyColorScheme::colorScheme_surface_2[254]);
                }
                else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0)
                {
                    colorData.append(ZyColorScheme::colorScheme_surface_2[0]);
                }
                else
                {
                    colorData.append(ZyColorScheme::colorScheme_surface_2[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
                }
            }
        }
        /*-水中-*/
        for(int j = sonarInfo.sfEnd; ((j<sonarInfo.btStart)&&(j<sonarImageHeight)); j++)
        {
            if(sonarInfo.rawData[j] == 0)
            {
                colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
            }
            else
            {
                if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254)
                {
                    colorData.append(ZyColorScheme::colorScheme_fish_2[254]);
                }
                else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0)
                {
                    colorData.append(ZyColorScheme::colorScheme_fish_2[0]);
                }
                else
                {
                    colorData.append(ZyColorScheme::colorScheme_fish_2[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
                }
            }
        }
        /*-水底-*/
        for(int j = sonarInfo.btStart; j < sonarImageHeight; j++)
        {
            if(sonarInfo.rawData[j] <5) {
                colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
            }
            else {
                if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254)
                {
                    colorData.append(ZyColorScheme::colorScheme_bottom_2[254]);
                }
                else if((sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0)
                {
                    colorData.append(ZyColorScheme::colorScheme_bottom_2[0]);
                }
                else
                {
                    colorData.append(ZyColorScheme::colorScheme_bottom_2[sonarInfo.rawData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
                }
            }
        }

    }

}


void Plot2DEchogram::updateColors()
{
    _colorLevels.resize(256);

    int colorOffset = ZyColorScheme::colorLine * COLOR_LINE;

    for(int i = 0; i < 256; i++) {
        if(i == 0) {
            _colorLevels[i] = qRgb((ZyColorScheme::background[ZyColorScheme::backgroundIndex] >> 16) & 0xFF,
                            (ZyColorScheme::background[ZyColorScheme::backgroundIndex] >> 8) & 0xFF,
                            ZyColorScheme::background[ZyColorScheme::backgroundIndex] & 0xFF);
        } else {
            // 计算颜色索引（参考 drawImagePixsel 的逻辑）
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
            qDebug() << "_colorSchemeType........" << _colorSchemeType;
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

int Plot2DEchogram::updateCash(Plot2D* parent, Dataset* dataset, int width, int height)
{
    auto& cursor = parent->cursor();

    if (_cash.size() != width) {
        _cash.resize(width);
        resetCash();
    }

    uint8_t* image_data = (uint8_t*)_image.constBits();
    const int b_scanline = _image.bytesPerLine();

    bool is_cash_notvalid = getTriggerCashReset();
    is_cash_notvalid |= !_lastCursor.isChannelsEqual(cursor);
    is_cash_notvalid |= !_lastCursor.isDistanceEqual(cursor);
    is_cash_notvalid |=  _lastWidth != width;
    is_cash_notvalid |=  _lastHeight != height;


    float from = cursor.distance.from;
    float to = cursor.distance.to;
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

    int wrap_start_pos = qAbs(cursor.getIndex(0) % width);

    for (unsigned int i = 0; i < cursor.indexes.size(); i++) {
        if (cursor.indexes[i] > 0) {
            wrap_start_pos = qAbs((cursor.indexes[i] + (width - i)) % width);
            break;
        }
    }


//    _cashPosition = wrap_start_pos;
    for(int column = 0; column < width; column++) {
        if(_cash[column].data.size() != height) {
//            _cash[column].stateColor = CashLine::CashState::CashStateNotValid;
            _cash[column].state = CashLine::CashState::CashStateNotValid;
            _cash[column].data.resize(height);
//            _cash[column].data.fill(0);
            _cash[column].poolIndex = -1;
            _cash[column].state = CashLine::CashState::CashStateEraced;
            _cash[column].isNeedUpdate = true;

            int16_t cash_data_size = _cash[column].data.size();
            int16_t* cash_data = _cash[column].data.data();
            uint8_t * img_data = image_data + column;
            for (int image_row = 0; image_row < cash_data_size; image_row++) {
                *img_data = *cash_data;
                img_data += b_scanline;
                cash_data++;
            }
        }

        int cursor_pos = column - wrap_start_pos;
        if(column < wrap_start_pos) {
            cursor_pos += width;
        }

        int pool_index = cursor.getIndex(cursor_pos);
        int pool_index_safe = dataset->validIndex(pool_index);
        if(pool_index_safe >= 0) {

            bool wasValidlyRendered = true;
            if (reRenderPlotIndxs_.contains(pool_index_safe)) {
                reRenderPlotIndxs_.remove(pool_index_safe);
                wasValidlyRendered = false;
            }

            auto* datasource = dataset->fromIndex(pool_index_safe);
            const int cash_index = _cash[column].poolIndex;

            if (is_cash_notvalid || pool_index_safe != cash_index || !wasValidlyRendered) {
                _cash[column].poolIndex = pool_index_safe;

                if(datasource != NULL) {
                    _cash[column].state = CashLine::CashState::CashStateNotValid;
                    int16_t* cash_data = _cash[column].data.data();
                    int16_t cash_data_size = _cash[column].data.size();

                    if (cursor.channel2 == CHANNEL_NONE) {
                        datasource->chartTo(cursor.channel1, cursor.subChannel1, from, to, cash_data, cash_data_size, _compensation_id);
                    }
                    else {
                        int cash_data_size_part1 = cash_data_size*(range1/fullrange);

                        if(cash_data_size_part1 > 0) {
                            datasource->chartTo(cursor.channel1, cursor.subChannel1, from1, to1, cash_data, cash_data_size_part1, _compensation_id, true);
                        }

                        if(cash_data_size_part1 < 0) {
                            cash_data_size_part1 = 0;
                        }

                        const int cash_data_size_part2 = cash_data_size - cash_data_size_part1;
                        if(cash_data_size_part2 > 0) {
                            datasource->chartTo(cursor.channel2, cursor.subChannel2, from2, to2, &cash_data[cash_data_size_part1], cash_data_size_part2, _compensation_id, false);
                        }
                    }

                    _cash[column].state = CashLine::CashState::CashStateValid;
                    _cash[column].isNeedUpdate = true;
                    uint8_t * img_data = image_data + column;
                    for (int image_row = 0; image_row < cash_data_size; image_row++) {
                        *img_data = *cash_data;
                        img_data += b_scanline;
                        cash_data++;
                    }
//                    _cash[column].stateColor = CashLine::CashState::CashStateNotValid;
                } else {
                    if(_cash[column].state != CashLine::CashState::CashStateEraced) {
//                        _cash[column].stateColor = CashLine::CashState::CashStateNotValid;
                        _cash[column].state = CashLine::CashState::CashStateNotValid;
                        _cash[column].data.fill(0);
                        _cash[column].poolIndex = -1;
                        _cash[column].state = CashLine::CashState::CashStateEraced;
                        _cash[column].isNeedUpdate = true;

                        int16_t cash_data_size = _cash[column].data.size();
                        int16_t* cash_data = _cash[column].data.data();
                        uint8_t * img_data = image_data + column;
                        for (int image_row = 0; image_row < cash_data_size; image_row++) {
                            *img_data = *cash_data;
                            img_data += b_scanline;
                            cash_data++;
                        }
                    }
                }

            }
        } else {
            if(_cash[column].state != CashLine::CashState::CashStateEraced) {
                _cash[column].state = CashLine::CashState::CashStateNotValid;
                _cash[column].data.fill(0);
                _cash[column].poolIndex = -1;
                _cash[column].state = CashLine::CashState::CashStateEraced;
                _cash[column].isNeedUpdate = true;

                int16_t* cash_data = _cash[column].data.data();
                int16_t cash_data_size = _cash[column].data.size();
                uint8_t * img_data = image_data + column;
                for (int image_row = 0; image_row < cash_data_size; image_row++) {
                    *img_data = *cash_data;
                    img_data += b_scanline;
                    cash_data++;
                }
            }
        }
    }

    //qInfo("Cash validate %u", cash_validate);

    _lastCursor = cursor;
    _lastWidth = width;
    _lastHeight = height;

    return wrap_start_pos;
}

bool Plot2DEchogram::draw(Plot2D* parent, Dataset* dataset)
{
    auto& canvas = parent->canvas();
    auto& cursor = parent->cursor();

    _colorLevels.clear();

    // ----------------- 测试用：创建简单的颜色表 ------------------
    // _colorLevels.resize(256);
    // for(int i = 0; i < 256; i++) {
    //     if(i < 20) {
    //         _colorLevels[i] = qRgb(0, 0, 20 + i);
    //     }
    //     else if(i < 50) {
    //         _colorLevels[i] = qRgb(0, i - 20, 50);
    //     }
    //     else if(i < 75) {
    //         _colorLevels[i] = qRgb(i - 50, 20 + (i - 50), 20);
    //     }
    //     else if(i < 100) {
    //         _colorLevels[i] = qRgb(20 + (i - 75), 20, 0);
    //     }
    //     else {
    //         int red = 75 + (i - 100);
    //         red = qBound(0, red, 255);
    //         _colorLevels[i] = qRgb(red, 20, 0);
    //     }
    // }


    updateColors();


    if (isVisible() && dataset != nullptr && cursor.distance.isValid()) {
        const int image_width  = canvas.width();
        const int image_height = canvas.height();

        if(_image.width() != image_width || _image.height() != image_height) {
            _image = QImage(image_width, image_height, QImage::Format_Indexed8);
            _image.setColorTable(_colorLevels);
            _pixmap = QPixmap(image_width, image_height);
        }

        // 设置颜色表
        _image.setColorTable(_colorLevels);

        const int cash_width    = canvas.width();
        const int cash_position = updateCash(parent, dataset, cash_width, image_height);

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
                 p.drawImage(cash_col_1, 0, _image, cash_col_1, 0 , cash_update_width, image_height, Qt::ThresholdDither); // Qt::NoOpaqueDetection |
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
