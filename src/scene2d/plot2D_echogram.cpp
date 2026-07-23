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

void Plot2DEchogram::setDepthFilterVisible(bool isVisible, int value)
{
    filterLevelVisible_ = isVisible;
    depthFilterList_.clear();
    depthFilterLevel_ = value;
    if(isVisible) {
        depthFilterList_ = getDepthListKF();
    }
}

void Plot2DEchogram::setBatchCorrect(bool batch)
{
    batchCorrect_ = batch;
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


double Plot2DEchogram::KalmanFilter(double ResrcData,double ProcessNiose_Q,double MeasureNoise_R,double InitialPrediction,int isFirst)
{
    static double x_last = ResrcData;
    if(-1 == isFirst)
    {
        x_last = ResrcData;
    }
    double x_mid = x_last;
    double x_now;
    static double p_last = InitialPrediction;
    if(-1 == isFirst)
    {
        p_last = InitialPrediction;
    }
    double p_mid;
    double p_now;
    double kg;

    x_mid = x_last;           //x_last=x(k-1|k-1),x_mid=x(k|k-1)
    p_mid = p_last + ProcessNiose_Q;       //p_mid=p(k|k-1),p_last=p(k-1|k-1),Q=噪声
    kg = p_mid / (p_mid + MeasureNoise_R); //kg为kalman filter，R为噪声
    x_now = x_mid + kg * (  ResrcData - x_mid); //估计出的最优值
    p_now = (1 - kg) * p_mid;                                 //最优值对应的covariance

    p_last = p_now; //更新covariance值
    x_last = x_now; //更新系统状态值

    return x_now;
}


void Plot2DEchogram::drawDepthFilter(Canvas canvas, int width, int cash_position, bool isVisible)
{
    if(!isVisible) {
        return;
    }

    if(_cash.size() != width || width <= 1 || depthFilterList_.isEmpty()) {
        return;
    }

    QPainter* cp = canvas.painter();
    QPen depthFilterPen(QColor(0,255,0));
    depthFilterPen.setWidth(2);
    cp->setPen(depthFilterPen);

    QPoint lastPoint;
    bool hasLast = false;
    for(int x = 0; x < width; x++) {
        int col = (cash_position + x) % width;
        if(_cash[col].state != CashLine::CashState::CashStateValid) {
            hasLast = false;
            continue;
        }

        int poolIndex = _cash[col].poolIndex;
        if(poolIndex < 0 || poolIndex >= depthFilterList_.size()) {
            hasLast = false;
            continue;
        }

        int depth = depthFilterList_.at(poolIndex);
        if(depth <= 0) {
            hasLast = false;
            continue;
        }

        float y = (float)depth / currentLoRng_ * canvas.height();
        QPoint currentPoint(x, (int)y);

        if(hasLast) {
            cp->drawLine(lastPoint, currentPoint);
        }

        lastPoint = currentPoint;
        hasLast = true;
    }

}

QList<int> Plot2DEchogram::getDepthListKF()
{
    QList<int> list_kf;
    int depth = 0;
    float mark_dif = 0;
    int mark_count = 0;
    int mark_last = depth;
    int isFirst = -1;
    if(dataset_ == nullptr || plot2d_ == nullptr) {
        return list_kf;
    }

    int diffCount = 20,diffValue = 30,R_small = 5,R_big = 20;
    DatasetCursor& cursor = plot2d_->cursor();
    int dataSize = dataset_->size();
    qDebug() << "dataSize....." << dataSize;
    for(int i = 0; i < dataSize; i++) {
        Epoch* epochData = dataset_->fromIndex(i);
        if(epochData == nullptr) {
            continue;
        }

        ChartParameters params = epochData->getChartParameters(cursor.channel1);
        float depth = params.depth;
        if(mark_count > diffCount) {
            mark_dif = (depth-mark_last)/diffValue;
            mark_count = 0;
            mark_last = depth;
        } else {
            mark_count++;
        }

        if(mark_dif != 0) {
            if(0 == depth) {
                if(list_kf.isEmpty()) {
                    list_kf.append(0);
                } else {
                    list_kf.append(list_kf.last());
                }
            }
            else {
                list_kf.append(KalmanFilter(depth, 1, R_small*depthFilterLevel_, 4000.0, isFirst));
            }
            //list_midCount.append(3);
        } else {
            //list_kfR.append(100);
            if(0 == depth) {
                if(list_kf.isEmpty()) {
                    list_kf.append(0);
                }
                else {
                    list_kf.append(list_kf.last());
                }
            } else {
                list_kf.append(KalmanFilter(depth, 1, R_big*depthFilterLevel_*depthFilterLevel_, 4000.0, isFirst));
            }
            //list_midCount.append(21);
        }

        isFirst = 0;
    }


    return list_kf;
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

    int sfEnd   = wavePixel_.sfEnd;
    int btStart = wavePixel_.btStart;
    QVector<uint8_t> cacheData = wavePixel_.waveData;

    QList<int> colorData;
    colorData.clear();

    /*-水表-*/
    for(int j = 0; (j<sfEnd)&&(j<btStart)&&(j<height); j++) {
        if(cacheData[j] == 0) {
            colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
        }
        else {
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
        else {
            if((cacheData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
                colorData.append(ZyColorScheme::colorScheme_fish[254]);
            }
            else if((cacheData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
                colorData.append(ZyColorScheme::colorScheme_fish[0]);
            }
            else {
                colorData.append(ZyColorScheme::colorScheme_fish[cacheData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
            }
        }

    }
    /*-水底-*/
    for(int j = btStart; j < height; j++)
    {
        if(cacheData[j] == 0) {
            colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
        }
        else
        {
            if((cacheData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
                colorData.append(ZyColorScheme::colorScheme_bottom[254]);
            }
            else if((cacheData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
                colorData.append(ZyColorScheme::colorScheme_bottom[0]);
            }
            else {
                colorData.append(ZyColorScheme::colorScheme_bottom[cacheData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
            }
        }
    }


    float scaleY = wavePixel_.nowScaleY;
    int startIdx = wavePixel_.startIdx;

    QImage sonarWave     = QImage(panelW, height, QImage::Format_RGB32);
    int *sonarWaveBuffer = reinterpret_cast<int *>(sonarWave.bits());
    std::fill(sonarWaveBuffer, sonarWaveBuffer+panelW*height, ZyColorScheme::background[ZyColorScheme::backgroundIndex]);

    int ratio = 10;
    if(scaleY < 1 && scaleY > 0) {
        scaleY = 1 / scaleY;
        /*-底色，防止拉伸在范围外-*/
        for(int i = 0; i < height; i++) {
            for(int j = 1; j < panelW; j++) {
                sonarWaveBuffer[(i)*panelW +j] = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
            }
        }

        for(int i = 0; (i < height) && ((startIdx+(int)(i*scaleY)) < colorData.count()); i++) {
            for(int j = (panelW/2-(cacheData[startIdx+(int)(i*scaleY)]/ratio));
                 j<(panelW/2+(cacheData[startIdx+(int)(i*scaleY)]/ratio)); j++) {
                sonarWaveBuffer[(i)*panelW +j] = colorData[startIdx+(int)(i*scaleY)];
            }
        }
    }
    else if(scaleY >= 1) {
        for(int i = 0; i < height; i++) {
            for(int j = 1; j < panelW; j++) {
                sonarWaveBuffer[i*panelW+j] = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
            }
        }
        for(int i = 0; (i < height) && ((startIdx+(int)(i/scaleY))<colorData.count()); i++) {
            for(int j = panelW/2-(cacheData[startIdx+(int)(i/scaleY)]/ratio);
                 j < (panelW/2+(cacheData[startIdx+(int)(i/scaleY)]/ratio)); j++) {
                sonarWaveBuffer[(i)*panelW +j] = colorData[startIdx+(int)(i/scaleY)];
            }
        }
    }

    p->drawImage(panelX, 0, sonarWave);

    int bottomLineIdx = wavePixel_.bottomLineIdx;
    if (depthCorrect_ && depthCorrectY_ >= 0 && depthCorrectY_ < height) {
        bottomLineIdx = depthCorrectY_;
    }
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

void Plot2DEchogram::setDepthCorrect(bool depthCorrect)
{
    depthCorrect_ = depthCorrect;
    if (!depthCorrect) {
        depthCorrectY_ = -1;
    }
}

void Plot2DEchogram::clearDepthCorrect()
{
    depthCorrectY_ = -1;
    resetCash();
}

void Plot2DEchogram::applyDepthCorrect(Plot2D* parent, Dataset* dataset, int mouseX, int mouseY, int imageWidth, int height)
{
    if (!depthCorrect_ || dataset == nullptr || parent == nullptr || height <= 0) {
        return;
    }

    int wavePanelWidth = imageWidth / (WAVE_WIDTH_RATIO_DENOM - 1);
    if (wavePanelWidth < 1) wavePanelWidth = 1;

    if (mouseX < imageWidth || mouseX > imageWidth + wavePanelWidth) {
        return;
    }

    int y = mouseY;
    if (y < 0) y = 0;
    if (y >= height) y = height - 1;

    depthCorrectY_ = y;

    float newDepth = currentUpRng_ + (currentLoRng_ - currentUpRng_) * ((float)y / height);

    DatasetCursor& cursor = parent->cursor();
    int latestIdx = dataset->endIndex() - 1;
    int latestSafe = dataset->validIndex(latestIdx);
    if (latestSafe >= 0) {
        Epoch* epoch = dataset->fromIndex(latestSafe);
        if (epoch) {
            ChartParameters params = epoch->getChartParameters(cursor.channel1);
            params.depth = newDepth;
            epoch->setChartParameters(cursor.channel1, params);
        }
    }

    resetCash();
    parent->plotUpdate();
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

    float currentViewMaxLoRng = -1.0f;
    for(int column = 0; column < width; column++) {
        int cursorPos = column - wrapStartPos;
        if(column < wrapStartPos) {
            cursorPos += width;
        }

        int poolIndex = cursor.getIndex(cursorPos);
        int pool_index_safe = dataset->validIndex(poolIndex);
        if(pool_index_safe >= 0) {
            // qDebug() << "pool_index_safe..........." << pool_index_safe;
            Epoch* epochData = dataset->fromIndex(pool_index_safe);
            if(epochData == nullptr) {
                return -1;
            }
            const int cacheIndex = _cash[column].poolIndex;
            if (isCashNotvalid || pool_index_safe != cacheIndex) {
                _cash[column].poolIndex = pool_index_safe;

                ChartParameters params = epochData->getChartParameters(cursor.channel1);
                const int pingSize = params.pingSize;
                // qDebug() << "=========================================================================================";

                int sfEnd = 0, btStart = 0, draft = 0, btStart_filter;
                float upRng = params.upRng;
                float loRng = params.loRng;
                float depth = params.depth;
                if((upRng < 0) || (loRng < 0) || (pingSize <= 0) || (upRng == loRng)) {
                    draft = 0;
                    btStart = 0;
                    sfEnd = 0;
                }

                float depthFilter = 0.01;
                if(depthFilterList_.size() > pool_index_safe) {
                    depthFilter = depthFilterList_.at(pool_index_safe);
                }
                // qDebug() << "depth..." << depth << "  loRng:" << loRng << "  upRng:" << upRng << "pingSize:" << pingSize;
                if(loRng != 0) {
                    draft   = (1500.0/soundVelocity_) * (draftOffset_ / (loRng-upRng)) * pingSize;
                    btStart = (1500.0/soundVelocity_) * (depth / (loRng-upRng)) * pingSize;
                    btStart_filter = (1500.0/soundVelocity_) * (depthFilter / (loRng-upRng)) * pingSize;

                    float surfaceEnd = 0.0;
                    if((depth < 100) && (depth > 30)) {
                        surfaceEnd = depth - 10 + draftOffset_;
                    } else {
                        surfaceEnd = 100 + draftOffset_;
                    }
                    sfEnd = (1500.0/soundVelocity_) * (surfaceEnd / (loRng-upRng)) * pingSize;
                }
                if((btStart < 0) || (sfEnd < 0) || (depth < 0)) {
                    draft   = 0;
                    btStart = 0;
                    sfEnd   = 0;
                }

                currentViewMaxLoRng = qMax(currentViewMaxLoRng, loRng);

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

                float nowScaleY = (float)height / pingSize * (loRng / (currentLoRng_-currentUpRng_));
                int startIdx = pingSize * currentUpRng_ / loRng;
                int btStartNow = btStart;
                btStartNow -= startIdx;
                // qDebug() << "btStart:" << btStart << "   btStartNow:" << btStartNow << "   nowScaleY:" << nowScaleY;
                int btStartFilter = btStart_filter;
                btStartFilter -= startIdx;
                QList<int> colorData;
                colorData.clear();
                int colorNum = 1;
                if(colorNum == 1) {
                    /*-水表-*/
                    for(int j = 0; (j<sfEnd)&&(j<btStart)&&(j<height); j++) {
                        if(cacheData[j] == 0) {
                            colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
                        }
                        else {
                            if((cacheData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
                                colorData.append(ZyColorScheme::colorScheme_surface[254]);
                            }
                            else if((cacheData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
                                colorData.append(ZyColorScheme::colorScheme_surface[0]);
                            }
                            else {
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
                        else {
                            if((cacheData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
                                colorData.append(ZyColorScheme::colorScheme_fish[254]);
                            }
                            else if((cacheData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
                                colorData.append(ZyColorScheme::colorScheme_fish[0]);
                            }
                            else {
                                colorData.append(ZyColorScheme::colorScheme_fish[cacheData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
                            }
                        }

                    }
                    /*-水底-*/
                    for(int j = btStart; j < height; j++)
                    {
                        if(cacheData[j] == 0) {
                            colorData.append(ZyColorScheme::background[ZyColorScheme::backgroundIndex]);
                        }
                        else {
                            if((cacheData[j]+ZyColorScheme::colorLine*COLOR_LINE) > 254) {
                                colorData.append(ZyColorScheme::colorScheme_bottom[254]);
                            }
                            else if((cacheData[j]+ZyColorScheme::colorLine*COLOR_LINE) < 0) {
                                colorData.append(ZyColorScheme::colorScheme_bottom[0]);
                            }
                            else {
                                colorData.append(ZyColorScheme::colorScheme_bottom[cacheData[j]+ZyColorScheme::colorLine*COLOR_LINE]);
                            }
                        }
                    }
                }


                uint32_t* img_data = (uint32_t*)_image.bits();
                int bytesPerLine   = _image.bytesPerLine() / 4;

                _cash[column].bottomLineIdx = btStartNow * nowScaleY;
                _cash[column].nowScaleY     = nowScaleY;

                if(nowScaleY < 1 && nowScaleY > 0) {
                    nowScaleY = 1 / nowScaleY;

                    int j = 0;
                    for(; ((int)(j*nowScaleY)+startIdx) < cacheData.count(); j++) {
                        int rgb = colorData[startIdx+(int)(j*nowScaleY)];
                        QRgb color = qRgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
                        img_data[j* bytesPerLine + column] = color;
                    }
                    /*-自动补齐-*/
                    for(; j < height; j++) {
                        /*-底层的部分设置成1不使用透明模式-*/
                        int rgb = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
                        img_data[j* bytesPerLine + column] = qRgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
                    }
                }
                else if(nowScaleY >= 1) {
                    for (int j = 0; j < height; j++) {
                        int rgb = ZyColorScheme::background[ZyColorScheme::backgroundIndex];
                        QRgb color = qRgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
                        img_data[j* bytesPerLine + column] = color;
                    }

                    for(int j = 0; (j<height) && (btStart+(int)(j/nowScaleY)<colorData.count()); j++) {
                        int rgb = colorData[startIdx+(int)(j/nowScaleY)];
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
                _cash[column].state    = CashLine::CashState::CashStateValid;
                _cash[column].isNeedUpdate = true;
                _cash[column].heading  = params.heading;
                _cash[column].depth    = params.depth;
                _cash[column].speed    = params.speed;
                _cash[column].temperature = params.temperature;
                _cash[column].longitude = params.longitude;
                _cash[column].latitude  = params.latitude;
                _cash[column].startIdx  = startIdx;



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
        if(currentViewMaxLoRng == -1.0f) {
            parent->currentViewMaxLoRng_ = 3200.0f;
        }
        else {
            parent->currentViewMaxLoRng_ = currentViewMaxLoRng;
        }
    }

    _lastCursor = cursor;

    return wrapStartPos;
}

void Plot2DEchogram::addBatchCorrect(QPoint pos)
{
    batchCorrectList_.append(pos);
}

void Plot2DEchogram::clearBatchCorrect()
{
    batchCorrectList_.clear();
}

void Plot2DEchogram::setUpdateBatchCorrect(bool updateBatchCorrect)
{
    updateBatchCorrect_ = updateBatchCorrect;
}

void Plot2DEchogram::drawBatchCorrect(Plot2D* parent, Dataset* dataset, int width, int height)
{
    if(batchCorrect_) {
        auto& canvas = parent->canvas();
        QPainter* p  = canvas.painter();
        QPen pen;
        pen.setWidth(3);
        pen.setColor(Qt::red);
        p->setPen(pen);

        for(int i = 0; i < batchCorrectList_.size() - 1; i++) {
            QPoint start = batchCorrectList_[i];
            QPoint end   = batchCorrectList_[i+1];
            p->drawLine(start, end);
        }
    }
}

void Plot2DEchogram::updateBatchCorrect(Plot2D* parent, Dataset* dataset, int width, int height)
{
    if(batchCorrect_ && updateBatchCorrect_) {
        QList<QPoint> correctPoints;
        for(int i = 0; i < batchCorrectList_.size() - 1; i++) {
            QPoint start = batchCorrectList_[i];
            QPoint end   = batchCorrectList_[i+1];
            int steps    = qMax(qAbs(end.x() - start.x()), qAbs(end.y() - start.y()));
            if(steps == 0) {
                continue;
            }

            for(int j = 0; j <= steps; j++) {
                QPoint p;
                p.setX(start.x() + (end.x() - start.x()) * j / steps);
                p.setY(start.y() + (end.y() - start.y()) * j / steps);
                correctPoints.append(p);
            }
        }

        DatasetCursor& cursor = parent->cursor();
        for(const QPoint& point: correctPoints) {
            int posX = point.x();
            int posY = point.y();
            if(posX < 0) posX = 0;
            if(posX >= width) posX = width - 1;
            int poolIndex = cursor.getIndex(posX);
            int pool_index_safe = dataset->validIndex(poolIndex);
            if(pool_index_safe >= 0) {
                Epoch* epochData = dataset->fromIndex(pool_index_safe);
                if(epochData) {
                    ChartParameters params = epochData->getChartParameters(cursor.channel1);
                    params.depth = currentUpRng_ + (currentLoRng_- currentUpRng_) * ((float)posY / height);
                    epochData->setChartParameters(cursor.channel1, params);
                }
            }
        }

        batchCorrectList_.clear();
        updateBatchCorrect_ = false;
        resetCash();
        parent->plotUpdate();
    }
}

bool Plot2DEchogram::draw(Plot2D* parent, Dataset* dataset)
{
    plot2d_ = parent;
    dataset_ = dataset;
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
        drawDepthFilter(canvas, image_width, cash_position, filterLevelVisible_);

        drawBatchCorrect(parent, dataset, image_width, image_height);
        updateBatchCorrect(parent, dataset, image_width, image_height);
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
