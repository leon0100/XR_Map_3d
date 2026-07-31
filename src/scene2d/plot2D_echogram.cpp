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

void Plot2DEchogram::setKeelOffsetValue(int value)
{
    keelOffset_ = value;
    resetCash();
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

double Plot2DEchogram::KalmanFilter(double ResrcData,double ProcessNiose_Q,double MeasureNoise_R,double InitialPredict,int isFirst)
{
    static double x_last = ResrcData;
    if(-1 == isFirst)
    {
        x_last = ResrcData;
    }
    double x_mid = x_last;
    double x_now;
    static double p_last = InitialPredict;
    if(-1 == isFirst)
    {
        p_last = InitialPredict;
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
    int bottomLineIdx = wavePixel_.bottomLineIdx;
    if (depthCorrect_ && depthCorrectY_ >= 0 && depthCorrectY_ < height) {
        bottomLineIdx = depthCorrectY_;
        btStart = depthCorrectBtStart_;
    }
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
}

void Plot2DEchogram::applyDepthCorrect(Plot2D* parent, Dataset* dataset, int mouseX, int mouseY, int imageWidth, int height)
{
    if (dataset == nullptr || parent == nullptr || height <= 0) {
        return;
    }

    int wavePanelWidth = imageWidth / (WAVE_WIDTH_RATIO_DENOM - 1);
    if (wavePanelWidth < 1) wavePanelWidth = 1;

    if (mouseX < imageWidth || mouseX > (imageWidth + wavePanelWidth)) {
        return;
    }

    if (mouseY < 0) mouseY = 0;
    if (mouseY >= height) mouseY = height - 1;

    depthCorrectY_ = mouseY;
    float newDepth = currentUpRng_ + (currentLoRng_ - currentUpRng_) * ((float)mouseY / height);
    float temp = wavePixel_.btStart / ((1500.0/soundVelocity_) * wavePixel_.depth);
    depthCorrectBtStart_ = newDepth / temp - wavePixel_.startIdx;
    resetCash();
    parent->plotUpdate();
}

void Plot2DEchogram::setMarkDistTimeVisible(bool markVisible, int dist0time1, int distInterval, int timeInterval,
                                bool isFrame, bool isTime, bool isDepth, bool isCoordinate)
{
    isMarkVisible_ = markVisible;
    dist0time1Visible_ = dist0time1;
    distInterval_ = distInterval;
    timeInterval_ = timeInterval;
    isFrameVisible_ = isFrame;
    isTimeVisible_ = isTime;
    isDepthVisible_ = isDepth;
    isCoordinateVisible_ = isCoordinate;

    markDistList_.clear();
    markTimeList_.clear();

    getMarkAccordTimeDist();
}


void Plot2DEchogram::drawMarks(Plot2D* parent, int width, int height, int cash_position)
{
    if(!isMarkVisible_) {
        return;
    }

    if(_cash.size() != width || width <= 1) {
        return;
    }

    auto& canvas = parent->canvas();
    DatasetCursor& cursor = parent->cursor();
    QPainter* p  = canvas.painter();

    if(markDistList_.isEmpty() && markTimeList_.isEmpty()) {
        return;
    }

    QPen markPen(QColor(255, 0, 0));
    markPen.setWidth(1);
    markPen.setStyle(Qt::DashDotLine);
    p->setPen(markPen);

    QFont font = p->font();
    font.setPixelSize(qMax(9, height / 70));
    p->setFont(font);
    QFontMetrics fm(font);

    for(int x = 0; x < width; x++) {
        int col = (cash_position + x) % width;
        int poolIndex = _cash[col].poolIndex;
        if(poolIndex < 0) {
            continue;
        }
        if(dist0time1Visible_ == 0) {
            if(!markDistList_.contains(poolIndex)) {
                continue;
            }
        }
        else if(dist0time1Visible_ == 1) {
            if(!markTimeList_.contains(poolIndex)) {
                continue;
            }
        }

        Epoch* epoch = dataset_->fromIndex(poolIndex);
        ChartParameters params = epoch->getChartParameters(cursor.channel1);
        QString markLabel;
        if(isFrameVisible_) {
            p->drawLine(x, 0, x, height);
            markLabel.append(QString::number(poolIndex+1));
        }
        if(isTimeVisible_) {
            quint32 timeVal = params.time;
            quint8 hour     = GET_HOUR(timeVal);
            quint8 minute   = GET_MINUTE(timeVal);
            quint8 second   = GET_SECOND(timeVal);
            QString time = ", " + QString("%1").arg(hour, 2, 10, QLatin1Char('0'))+":"+QString("%1").
                    arg(minute, 2, 10, QLatin1Char('0'))+":"+QString("%1").arg(second, 2, 10, QLatin1Char('0'));
            markLabel.append(time);
        }
        if(isCoordinateVisible_) {
            double lat  = params.latitude;
            double lon  = params.longitude;
            QString string_ns, string_lat, string_ew, string_lon;
            if(lon >= 0) {
                string_ew = "E";
                string_lon = QString::number(lon,'f',6);
            }
            else {
                string_ew = "W";
                string_lon = QString::number(-lon,'f',6);
            }

            if(lat >= 0) {
                string_ns = "N";
                string_lat = QString::number(lat,'f',6);
            }
            else {
                string_ns = "S";
                string_lat = QString::number(-lat,'f',6);
            }
            QString coor = ", " + string_ew + string_lon + "° " +string_ns + string_lat +"°";
            markLabel.append(coor);
        }
        if(isDepthVisible_){
            QString depth = ", " + QString::number(params.depth/100.0f, 'f', 2) +"m";
            markLabel.append(depth);
        }

        p->save();
        p->translate(x, height);
        p->rotate(-90);
        p->drawText(5, -2, markLabel);
        p->restore();
    }
}

void Plot2DEchogram::getMarkAccordTimeDist()
{
    if(dataset_ == nullptr || plot2d_ == nullptr) {
        return;
    }
    DatasetCursor& cursor = plot2d_->cursor();

    int dataSize = dataset_->size();
    if(dataSize == 0) {
        return;
    }

    if(dist0time1Visible_ == 0) {
        markDistList_.clear();
        markDistList_.insert(0);
        Epoch* epochData0 = dataset_->fromIndex(0);
        if(epochData0 == nullptr) {
            return;
        }

        North_East_Down lastNed = epochData0->getPositionGNSS().ned;

        for(int i = 1; i < dataSize; i++) {
            Epoch* epochData = dataset_->fromIndex(i);
            if(epochData == nullptr) {
                continue;
            }

            North_East_Down currNed = epochData->getPositionGNSS().ned;
            double dn = currNed.n - lastNed.n;
            double de = currNed.e - lastNed.e;
            double dist = sqrt(dn * dn + de * de);
            if(dist > distInterval_) {
                lastNed = currNed;
                markDistList_.insert(i);
            }
        }
    }
    else if(dist0time1Visible_ == 1) {
        markTimeList_.clear();
        markTimeList_.insert(0);
        Epoch* epochData0 = dataset_->fromIndex(0);
        if(epochData0 == nullptr) {
            return;
        }
        ChartParameters param0 = epochData0->getChartParameters(cursor.channel1);
        quint32 timeVal0 = param0.time;
        quint8 hour0 = GET_HOUR(timeVal0);
        quint8 minute0 = GET_MINUTE(timeVal0);
        quint8 second0 = GET_SECOND(timeVal0);
        int currTime0 = hour0 * 3600 + minute0 * 60 + second0;
        int nextTime = currTime0 + timeInterval_;

        for(int i = 1; i < dataSize; i++) {
            Epoch* epochData = dataset_->fromIndex(i);
            if(epochData == nullptr) {
                continue;
            }

            ChartParameters param = epochData->getChartParameters(cursor.channel1);
            quint32 timeVal = param.time;
            quint8 hour = GET_HOUR(timeVal);
            quint8 minute = GET_MINUTE(timeVal);
            quint8 second = GET_SECOND(timeVal);
            int currTime = hour * 3600 + minute * 60 + second;
            if(currTime >= nextTime) {
                markTimeList_.insert(i);
                nextTime += timeInterval_;
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
                int sfEnd = 0, btStart = 0, draft = 0, btStart_filter;
                float upRng = params.upRng;
                float loRng = params.loRng;
                float depth = params.depth;
                depth += keelOffset_;
                if((upRng < 0) || (loRng < 0) || (pingSize <= 0) || (upRng == loRng)) {
                    draft = 0;
                    btStart = 0;
                    sfEnd = 0;
                }

                float depthFilter = 0.01;
                if(depthFilterList_.size() > pool_index_safe) {
                    depthFilter = depthFilterList_.at(pool_index_safe);
                }
                // qDebug() << "222222depth:" << depth << "  latitude:" << params.longitude << "  " << params.latitude;
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
                        int rgb = colorData[(int)(j*nowScaleY)+startIdx];
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

                    for(int j = 0; j<height && ((int)(j/nowScaleY)+ startIdx)<colorData.count(); j++) {
                        int rgb = colorData[(int)(j/nowScaleY)+ startIdx];
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

void Plot2DEchogram::setDeleteFrameMode(bool mode)
{
    deleteFrameMode_ = mode;
    if (mode) {
        if(dataset_ && dataset_->size() != 0) {
            deleteHint_ = 1;
        }
    }
    else {
        deleteHint_ = 0;
        deleteStartIdx_ = -1;
        deleteEndIdx_   = -1;
        deleteFrameMouseX_ = -1;
        deleteFrameMouseY_ = -1;
    }
}

void Plot2DEchogram::clearDeleteFrame()
{
    deleteHint_ = 1;
    deleteStartIdx_ = -1;
    deleteEndIdx_   = -1;
    deleteFrameMouseX_ = -1;
    deleteFrameMouseY_ = -1;
}

int Plot2DEchogram::getDeleteStartIdx() const
{
    int dataSize = dataset_->size();
    if(dataSize <= deleteStartIdx_) {
        return -1;
    }
    DatasetCursor& cursor = plot2d_->cursor();
    Epoch* epochData = dataset_->fromIndex(deleteStartIdx_);
    ChartParameters param = epochData->getChartParameters(cursor.channel1);
    double lat  = param.latitude;
    double lon  = param.longitude;
    QString string_ns, string_lat, string_ew, string_lon;
    if(lon >= 0) {
        string_ew = "E";
        string_lon = QString::number(lon,'f',6);
    } else {
        string_ew = "W";
        string_lon = QString::number(-lon,'f',6);
    }
    if(lat >= 0) {
        string_ns = "N";
        string_lat = QString::number(lat,'f',6);
    } else {
        string_ns = "S";
        string_lat = QString::number(-lat,'f',6);
    }
    plot2d_->fromLonStr_  = string_ew + string_lon + "°";
    plot2d_->fromLatiStr_ = string_ns + string_lat + "°";

    if(deleteEndIdx_ > 0) {
        epochData = dataset_->fromIndex(deleteEndIdx_);
        param = epochData->getChartParameters(cursor.channel1);
        lat  = param.latitude;
        lon  = param.longitude;
        // QString string_ns2, string_lat2, string_ew2, string_lon2;
        if(lon >= 0) {
            string_ew = "E";
            string_lon = QString::number(lon,'f',6);
        } else {
            string_ew = "W";
            string_lon = QString::number(-lon,'f',6);
        }
        if(lat >= 0) {
            string_ns = "N";
            string_lat = QString::number(lat,'f',6);
        } else {
            string_ns = "S";
            string_lat = QString::number(-lat,'f',6);
        }
        plot2d_->toLonStr_  = string_ew + string_lon + "°";
        plot2d_->toLatiStr_ = string_ns + string_lat + "°";
    }

    return deleteStartIdx_;
}

int Plot2DEchogram::getDeleteEndIdx() const
{
    return deleteEndIdx_;
}

bool Plot2DEchogram::deleteFrames(Plot2D* parent, Dataset* dataset)
{
    if(parent == nullptr || dataset == nullptr) return false;
    if((deleteStartIdx_ < 0) || (deleteEndIdx_ < 0)) return false;
    if((deleteStartIdx_ >= dataset->size()) || (deleteEndIdx_ >= dataset->size())) return false;
    dataset->removeFrames(deleteStartIdx_, deleteEndIdx_);
    clearDeleteFrame();
    resetCash();
    return true;
}

void Plot2DEchogram::updateDeleteFrameMousePos(int mouseX, int mouseY)
{
    deleteFrameMouseX_ = mouseX;
    deleteFrameMouseY_ = mouseY;
}

bool Plot2DEchogram::handleDeleteFrameDoubleClick(Plot2D* parent, Dataset* dataset, int mouseX, int mouseY, bool isHorizontal)
{
    if (!deleteFrameMode_ || parent == nullptr || dataset == nullptr) {
        return false;
    }

    int epochIdx = parent->getEpochIndxByMousePos(mouseX, mouseY, isHorizontal);
    if (epochIdx < 0 || epochIdx >= dataset->size()) {
        return false;
    }

    if (deleteHint_ == 1) {
        deleteStartIdx_ = epochIdx;
    }
    else if(deleteHint_ == 2) {
        deleteEndIdx_ = epochIdx;
        if (deleteEndIdx_ < deleteStartIdx_) {
            std::swap(deleteStartIdx_, deleteEndIdx_);
        }
    }
    return true;
}

void Plot2DEchogram::drawDeleteFrameHint(int width, int height)
{
    if (!deleteFrameMode_ ||  plot2d_ == nullptr || deleteHint_ == 0) return;
    auto& canvas = plot2d_->canvas();
    QPainter* p  = canvas.painter();
    if (p == nullptr) return;

    QFont font = p->font();
    font.setPixelSize(qMax(9, height / 60));
    p->setFont(font);
    QFontMetrics fm(font);
    QString text;
    if (deleteHint_ == 1) {
        text = QObject::tr("select start frame");
    }
    else if(deleteHint_ == 2) {
        text = QObject::tr("select end frame");
    }
    QRect textRect = fm.boundingRect(text);

    int mx = deleteFrameMouseX_;
    int my = deleteFrameMouseY_;
    if (mx < 0 || my < 0) {
        mx = width  * 0.5;
        my = height * 0.5;
    }
    int textX = mx + 10;
    int textY = my - textRect.height() - 5;
    if (textX + textRect.width() > width) {
        textX = mx - textRect.width() - 10;
    }
    if (textY < 0) {
        textY = my + 10;
    }
    p->fillRect(textX - 4, textY - 2, textRect.width() + 8, textRect.height() + 4, QColor(0, 0, 0, 180));
    p->drawText(textX, textY + fm.ascent(), text);

    QPen spen(QColor(0, 255, 0, 200));
    spen.setWidth(2);
    spen.setStyle(Qt::DashLine);
    p->setPen(spen);

    DatasetCursor& cursor = plot2d_->cursor();
    if (deleteStartIdx_ >= 0) {
        for (int col = 0; col < (int)cursor.indexes.size(); col++) {
            if (cursor.indexes[col] == deleteStartIdx_) {
                p->drawLine(col, 0, col, height);
                if (deleteHint_ == 1) {
                    deleteHint_ = 2;
                }
                break;
            }
        }
    }

    if (deleteEndIdx_ >= 0) {
        for (int col = 0; col < (int)cursor.indexes.size(); col++) {
            if (cursor.indexes[col] == deleteEndIdx_) {
                p->drawLine(col, 0, col, height);
                break;
            }
        }
    }

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

void Plot2DEchogram::clearPlotData()
{
    clearDeleteFrame();
    clearBatchCorrect();
    wavePixel_ = CashLine();
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
        drawMarks(parent, image_width, image_height, cash_position);
        drawDeleteFrameHint(image_width, image_height);
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
