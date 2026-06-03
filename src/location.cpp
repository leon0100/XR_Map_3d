#include "location.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QSettings>
#include <QFileDialog>
#include <QtMath>




Locations::Locations(QObject *parent) : QObject(parent)
{
    searchTimer_ = new QTimer(this);
    searchTimer_->setInterval(200);
    searchTimer_->setSingleShot(true);
    connect(searchTimer_, &QTimer::timeout, this, &Locations::slot_SearchTriggered);

    manager_ = new QNetworkAccessManager(this);
    connect(this,&Locations::signalLocationGoogle,this,&Locations::slot_locationGoogle);
    connect(AppController::instance(),&AppController::signalFileSelected,this,&Locations::slot_exportKmlPathSelected);
    connect(this,&Locations::signalLocationStyle, this, &Locations::slot_locationStyle);
}

typGpsCooDegree Locations::getCooDegree()
{
    return gpsCoor_;
}

void Locations::setDataset(Dataset* dataset)
{
    dataset_ = dataset;
}

void Locations::setLocationStyle(int locationStyle)
{
    locationStyle_ = locationStyle;
}

void Locations::setLatLonFormat(int format)
{
    m_latLonFormat_ = format;
}

void Locations::onLatLonChanged(const QString &latLon)
{
    parseLatLonFromPaste(latLon);
}

void Locations::setLatitude2(QString latitude)
{
    latitude_ = latitude;
}

void Locations::setLongitude2(QString longitude)
{
    longitude_ = longitude;
}

void Locations::setFuzzySearch2(QString fuzzySearch)
{
    if(fuzzySearch.isEmpty()) {
        fuzzyResult_.clear();
        emit signalFuzzyResults();
        return;
    }
    fuzzySearch_ = fuzzySearch;
    searchTimer_->start();
}

void Locations::clearFuzzySearch(QString fuzzySearch)
{
    searchTimer_->stop();
    fuzzySearch_ = fuzzySearch;
    fuzzyResult_.clear();
    emit signalFuzzySearch();
    emit signalFuzzyResults();
}

QString Locations::getLatitude()
{
    return latitude_;
}

void Locations::setLatitude(QString latitude)
{
    latitude_ = latitude;
    emit signalLatitude();
}

QString Locations::getLongitude()
{
    return longitude_;
}

void Locations::setLongitude(QString longitude)
{
    longitude_ = longitude;
    emit signalLongitude();
}

QStringList Locations::fuzzyResults()
{
    return fuzzyResult_;
}


QString Locations::getFuzzySearch()
{
    return fuzzySearch_;
}

QString Locations::street()
{
    return street_;
}


QString Locations::city()
{
    return city_;
}

QString Locations::province()
{
    return province_;
}

QString Locations::country()
{
    return country_;
}

QString Locations::importKml()
{
    return importKml_;
}

void Locations::setImportKml2(QString importKml)
{
    importKml_ = importKml;
    emit signalImportKml();
}

void Locations::clearAllText()
{
    setLatitude("");
    setLongitude("");
    clearFuzzySearch("");
    setStreet2("");
    setCity2("");
    setProvince2("");
    setCountry2("");
    setImportKml2("");
    emit signalShowLocation(false);
}
void Locations::cancelClicked()
{
    clearAllText();
}

void Locations::confirmClicked()
{
    confirmLocation();
    clearAllText();
}

void Locations::setStreet2(QString street)
{
    street_ = street;
    emit signalStreet();
}
void Locations::setCity2(QString city)
{
    city_ = city;
    emit signalCity();
}
void Locations::setProvince2(QString province)
{
    province_ = province;
    emit signalProvince();
}
void Locations::setCountry2(QString country)
{
    country_ = country;
    emit signalCountry();
}


void Locations::slot_locationGoogle()
{
    QString geocodingApiUrl = GEOCODING_API_URL;

    QUrlQuery query;
    if (!fuzzySearch_.isEmpty()) {
        // 使用模糊搜索
        query.addQueryItem("address", QUrl::toPercentEncoding(fuzzySearch_));
    } else {
        // 使用精确搜索
        QString fullAddress = QString("%1, %2, %3, %4") .arg(street_).arg(city_).arg(province_).arg(country_);
        query.addQueryItem("address", fullAddress);
    }

    QString apiKey = GOOGLE_API_KEY;
    query.addQueryItem("key", apiKey);

    QUrl requestUrl(geocodingApiUrl);
    requestUrl.setQuery(query);
    QNetworkRequest request(requestUrl);
    QNetworkReply *reply = manager_->get(request);
    connect(reply,&QNetworkReply::finished,this,&Locations::slot_locationResponseGoogle);
}
void Locations::slot_locationResponseGoogle()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject root = doc.object();
        QJsonArray results = root.value("results").toArray();
        if (!results.isEmpty()) {
            QJsonObject firstResult = results[0].toObject();
            QJsonObject geometry = firstResult.value("geometry").toObject();
            QJsonObject location = geometry.value("location").toObject();
            if (location.contains("lat") && location.contains("lng")) {
                double lat = firstResult.value("lat").toString().toDouble();
                double lon = firstResult.value("lon").toString().toDouble();
                gpsCoor_.latitude  = lat;
                gpsCoor_.longitude = lon;
                emit signalLocationStyle(1);
            }
            else {
                GIF->dialogInfo(Dialog_OK, tr("No search results found!"));
            }
        } else {
            GIF->dialogInfo(Dialog_OK, tr("No search results found!"));
        }
    }

    reply->deleteLater();
}


void Locations::getCoordinateOpenstreet()
{
    QString openStreetMapUrl = OPENSTREET_MAP_URL;
    QUrlQuery query;

    if (!fuzzySearch_.isEmpty()) {
        query.addQueryItem("q", fuzzySearch_);
    }
    else {
        QString fullAddress = QString("%1, %2, %3, %4").arg(street_).arg(city_).arg(province_).arg(country_);
        query.addQueryItem("q", fullAddress);
    }
    query.addQueryItem("format","json");
    query.addQueryItem("limit", "10");

    QUrl requestUrl(openStreetMapUrl);
    requestUrl.setQuery(query);
    QNetworkRequest request(requestUrl);
    request.setHeader(QNetworkRequest::UserAgentHeader, "XR-Map/1.10 (Contact: 2376963887@qq.com)");
    request.setRawHeader("Accept", "application/json");
    QNetworkReply *reply = manager_->get(request);
    connect(reply,&QNetworkReply::finished,this,&Locations::slot_locationResponseOpenStreet);
}
void Locations::slot_locationResponseOpenStreet()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply->error() != QNetworkReply::NoError) {
        emit signalLocationGoogle();
    }
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isArray()) {
        QJsonArray results = doc.array();
        if (!results.isEmpty()) {
            QJsonObject firstResult = results[0].toObject();
            if (firstResult.contains("lat") && firstResult.contains("lon")) {
                double lat = firstResult.value("lat").toString().toDouble();
                double lon = firstResult.value("lon").toString().toDouble();
                gpsCoor_.latitude  = lat;
                gpsCoor_.longitude = lon;
                emit signalLocationStyle(1);
            }
        }
        else
        {
            emit signalLocationGoogle();
        }
    }
    else
    {
        emit signalLocationGoogle();
    }

    reply->deleteLater();
}

void Locations::importKmlClicked()
{
#ifdef Q_OS_WIN
    QSettings settings("Toslon Company", "XR-Map");
    QString lastPath = settings.value("lastOpenPath", "./").toString();
    if (!lastPath.endsWith("/")) {
        lastPath.append("/");
    }

    QString fileName = QFileDialog::getOpenFileName(nullptr, tr("Open File"), lastPath, tr("Files (*.kmz *.kml);;"));
    if (!fileName.isEmpty()) {
        settings.setValue("lastOpenPath", QFileInfo(fileName).absolutePath());
        setImportKml2(fileName);
    }
#elif defined(Q_OS_ANDROID)
    // 调用 Java 方法打开 SAF 文件选择器
    QAndroidJniObject::callStaticMethod<void>("com/nqc/FileQtActivity","openFileFromQt","()V");

#endif
}


void Locations::confirmLocation()
{
    if(locationStyle_ == 0)  //经纬度值定位
    {
        gpsCoor_= locationUseLatiLon();
        if((gpsCoor_.latitude == -1000) && (gpsCoor_.longitude == -1000)) {
            return;
        }
        emit signalLocationStyle(0);
    }
    else if(locationStyle_ == 1)  //地名定位
    {
        getCoordinateOpenstreet();
    }
    else if(locationStyle_ == 2) {
        emit signalLocationStyle(2);
    }


}

typGpsCooDegree Locations::locationUseLatiLon()
{
    QString latiStr = latitude_;
    QString lonStr = longitude_;
    qDebug() << "latiStr....." << latiStr << "   " << lonStr;

    QRegularExpression ddRegex(R"(^([-+]?\d{1,3})(?:\.(\d+))?[°]?[NSWEnswe]?$)");
    QRegularExpression dmsRegex(R"(^([-+]?\d{1,3})°([0-5]?[0-9])[\'′]([0-5]?[0-9](?:\.\d+)?)[\"″]?[NSWEnswe]?$)");
    QRegularExpression ddmRegex(R"(^([-+]?\d{1,3})°([0-5]?[0-9](?:\.\d+)?)[\'′][NSWEnswe]?$)");

    typGpsCooDegree gpsCoor;
    double lat = 0.0, lon = 0.0;
    bool latSuccess = false, lonSuccess = false;

    switch (m_latLonFormat_)
    {
    case 0:  // Decimal Degrees (DD)
        lat = DDToDecimalDegrees(latiStr, ddRegex, latSuccess);
        lon = DDToDecimalDegrees(lonStr, ddRegex, lonSuccess);
        break;

    case 1:  // Degrees, Minutes, Seconds (DMS)
        lat = DMSToDecimalDegrees(latiStr, dmsRegex, latSuccess);
        lon = DMSToDecimalDegrees(lonStr, dmsRegex, lonSuccess);
        break;

    case 2:  // Degrees, Decimal Minutes (DDM)
        lat = DDMToDecimalDegrees(latiStr, ddmRegex, latSuccess);
        lon = DDMToDecimalDegrees(lonStr, ddmRegex, lonSuccess);
        break;

    default:
        break;
    }

    if (!latSuccess || !lonSuccess) {
        qDebug() << "latSuccess: " << latSuccess << "  lonSuccess:" << lonSuccess;
        GIF->dialogInfo(Dialog_OK,  tr("Invalid latitude or longitude for the selected format."));
        return {-1000,-1000};
    }

    gpsCoor.latitude = lat;
    gpsCoor.longitude = lon;
    return gpsCoor;
}

void Locations::parseLatLonFromPaste(QString str)
{
    QStringList parts = str.split(QRegularExpression("[,，\\s;]+"), Qt::SkipEmptyParts);
    if(parts.size() == 2) {
        // QRegularExpression latiRegex(R"(.*([北南](?:纬)?|N|S|n|s).*)", QRegularExpression::CaseInsensitiveOption);
        // QRegularExpression lonRegex(R"(.*([东西](?:经)?|E|W|e|w).*)", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression latiRegex(R"(.*(北|南|N|S|n|s).*)", QRegularExpression::CaseInsensitiveOption);
        QRegularExpression lonRegex(R"(.*(东|西|E|W|e|w).*)",  QRegularExpression::CaseInsensitiveOption);
        QString part1 = parts[0], part2 = parts[1];
        QString latStr, lonStr;
        QString latSymbol, lonSymbol;

        if(latiRegex.match(part1).hasMatch() && lonRegex.match(part2).hasMatch()) {
            latStr = part1;
            lonStr = part2;
        }
        else if(latiRegex.match(part2).hasMatch() && lonRegex.match(part1).hasMatch()) {
            latStr = part2;
            lonStr = part1;
        }
        else {
            QRegularExpression numberRegex(R"(^[-+]?\d+(\.\d+)?$)");
            if (numberRegex.match(part1).hasMatch() && numberRegex.match(part2).hasMatch()) {
                latStr = part1;
                lonStr = part2;
                latSymbol = latStr.startsWith('-') ? "S" : "N";
                lonSymbol = lonStr.startsWith('-') ? "W" : "E";
            }
        }

        if (latStr.contains(QRegularExpression("[北Nn]"))) {
            latSymbol = "N";
        }
        else if (latStr.contains(QRegularExpression("[南Ss]"))) {
            latSymbol = "S";
        }

        if (lonStr.contains(QRegularExpression("[东Ee]"))) {
            lonSymbol = "E";
        }
        else if (lonStr.contains(QRegularExpression("[西Ww]"))) {
            lonSymbol = "W";
        }

        // latStr.remove(QRegularExpression("[北南](?:纬)?|N|S|n|s"));
        // lonStr.remove(QRegularExpression("[东西](?:经)?|E|W|e|w"));
        latStr.remove(QRegularExpression("北|南|N|S|n|s"));
        lonStr.remove(QRegularExpression("东|西|E|W|e|w"));

        latStr = latStr.trimmed() + latSymbol;
        lonStr = lonStr.trimmed() + lonSymbol;

        if(part1.contains(QRegularExpression("[Gg]"))) {
            latSymbol = "N";
            lonSymbol = "E";
            lonStr = part1.remove(0, 1) + lonSymbol;
            latStr = part2 + latSymbol;
        }

        if(latSymbol.isEmpty() && lonSymbol.isEmpty()) {
            GIF->dialogInfo(Dialog_OK, tr("Invalid latitude or longitude for the selected format."));
        }

        setLatitude(latStr);
        setLongitude(lonStr);
    }
}

double Locations::DDToDecimalDegrees(const QString &str, const QRegularExpression &regex, bool &success)
{
    QString trimmedStr = str.trimmed();
    QRegularExpressionMatch match = regex.match(trimmedStr);
    if (match.hasMatch()) {
        double degrees = match.captured(1).toDouble();
        double decimal = match.captured(2).isEmpty() ? 0.0 : match.captured(2).toDouble();
        double decimalDegrees = degrees + decimal/qPow(10, match.captured(2).size());
        bool isNegative = trimmedStr.startsWith('-') || trimmedStr.endsWith("S") || trimmedStr.endsWith("s")
                          || trimmedStr.endsWith("W") || trimmedStr.endsWith("w") || trimmedStr.startsWith("S")
                          || trimmedStr.startsWith("s") || trimmedStr.startsWith("W") || trimmedStr.startsWith("w");
        if (isNegative) {
            decimalDegrees = -qAbs(decimalDegrees);
        }

        success = true;
        return decimalDegrees;
    }

    success = false;
    return 0.0;
}


double Locations::DMSToDecimalDegrees(const QString &str, const QRegularExpression &regex, bool &success)
{
    QString trimmedStr = str.trimmed();
    QRegularExpressionMatch match = regex.match(trimmedStr);
    if (match.hasMatch()) {
        double degrees = match.captured(1).toDouble();
        double minutes = match.captured(2).toDouble();
        double seconds = match.captured(3).isEmpty() ? 0.0 : match.captured(3).toDouble();

        double decimalDegrees = 0.0;
        if (trimmedStr.startsWith('-') || trimmedStr.endsWith('S') || trimmedStr.endsWith('s') || trimmedStr.endsWith('W')
            || trimmedStr.endsWith('w')|| trimmedStr.startsWith('S') || trimmedStr.startsWith('s') || trimmedStr.startsWith('W')
            || trimmedStr.startsWith('w')) {
            decimalDegrees = qAbs(degrees) + minutes/60.0 + seconds/3600.0;
            decimalDegrees = -decimalDegrees;
        } else {
            decimalDegrees = degrees + minutes/60.0 + seconds/3600.0;
        }

        success = true;
        return decimalDegrees;
    }

    success = false;
    return 0.0;
}

double Locations::DDMToDecimalDegrees(const QString &str, const QRegularExpression &regex, bool &success)
{
    QString trimmedStr = str.trimmed();
    QRegularExpressionMatch match = regex.match(trimmedStr);
    match = regex.match(str);
    if (match.hasMatch()) {
        double degrees = match.captured(1).toDouble();
        double decimalMinutes = match.captured(2).toDouble();

        double decimalDegrees = 0.0;
        if (trimmedStr.startsWith('-') || trimmedStr.endsWith('S') || trimmedStr.endsWith('s')
            || trimmedStr.endsWith('W') ||trimmedStr.endsWith('w')|| trimmedStr.startsWith('S') || trimmedStr.startsWith('s')
            || trimmedStr.startsWith('W') ||trimmedStr.startsWith('w')) {
            decimalDegrees = qAbs(degrees) + decimalMinutes / 60.0;
            decimalDegrees = -decimalDegrees;
        } else {
            decimalDegrees = degrees + decimalMinutes / 60.0;
        }

        success = true;
        return decimalDegrees;
    }

    success = false;
    return 0.0;
}

void Locations::slot_SearchTriggered()
{
    QString openStreetMapUrl = OPENSTREET_MAP_URL;
    QUrlQuery query;
    query.addQueryItem("q", fuzzySearch_);
    query.addQueryItem("format","json");
    query.addQueryItem("limit", "10");

    QUrl requestUrl(openStreetMapUrl);
    requestUrl.setQuery(query);

    QNetworkRequest request(requestUrl);
    request.setHeader(QNetworkRequest::UserAgentHeader, "XR-Map/1.10 (Contact: 2376963887@qq.com)");
    request.setRawHeader("Accept", "application/json");
    QNetworkReply *reply = manager_->get(request);
    connect(reply,&QNetworkReply::finished,this,&Locations::slot_locationResponseOpenStreetVague);
}
void Locations::slot_locationResponseOpenStreetVague()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isArray())
        {
            QJsonArray results = doc.array();
            if (!results.isEmpty())
            {
                fuzzyResult_.clear();
                for (const QJsonValue &value : results)
                {
                    QJsonObject result = value.toObject();
                    if (result.contains("lat") && result.contains("lon")) {
                        if (result.contains("display_name")) {
                            fuzzyResult_.append(result["display_name"].toString());
                        }
                    }
                }
                emit signalFuzzyResults();
            }
            else {
                // qDebug() <<  "results.isEmpty()) ";
            }
        }
        else {
            //qDebug() <<  "doc.isArray()) ";
        }
    }

    reply->deleteLater();
}


void Locations::slot_exportKmlPathSelected(QString path)
{
    setImportKml2(path);
}

void Locations::slot_locationStyle(int index)
{
    // qDebug() << "slot_locationsDStyle....." << index;
    if(index == 0 || index == 1) {
        dataset_->location(gpsCoor_.latitude, gpsCoor_.longitude);
    }
    else if(index == 2) {

    }
}




