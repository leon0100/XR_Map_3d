#ifndef LOCATION_H
#define LOCATION_H

#include <QObject>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QTimer>
#include <QNetworkAccessManager>



#include "console.h"


#ifdef Q_OS_ANDROID
#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>
#include <QtAndroid>
#endif


#define  OPENSTREET_MAP_URL  "https://nominatim.openstreetmap.org/search";
#define  GEOCODING_API_URL   "https://maps.googleapis.com/maps/api/geocode/json";
#define  GOOGLE_API_KEY      "AIzaSyCTp-KwFso1wsfdEv732pidoddep0jbZ-k";


struct PlaceNameStruct {
    QString fuzzyPlace; //模糊地址
    QString street;
    QString city;
    QString state;
    QString country;
};

typedef struct _typGpsCooDegree_
{
    double latitude;
    double longitude;
}typGpsCooDegree;







class Locations : public QObject
{
    Q_OBJECT

public: //发送信号signalLatitude会改变属性的latitude
    Q_PROPERTY(QString latitude   READ getLatitude   NOTIFY signalLatitude)
    QString getLatitude();
    Q_PROPERTY(QString longitude  READ getLongitude   NOTIFY signalLongitude)
    QString getLongitude();

    Q_PROPERTY(QStringList fuzzyResults  READ fuzzyResults NOTIFY signalFuzzyResults)
    QStringList fuzzyResults();
    Q_PROPERTY(QString fuzzySearch       READ getFuzzySearch   NOTIFY signalFuzzySearch)
    QString getFuzzySearch();
    Q_PROPERTY(QString street            READ street       NOTIFY signalStreet)
    QString street();
    Q_PROPERTY(QString city              READ city         NOTIFY signalCity)
    QString city();
    Q_PROPERTY(QString province          READ province     NOTIFY signalProvince)
    QString province();
    Q_PROPERTY(QString country           READ country      NOTIFY signalCountry)
    QString country();
    Q_PROPERTY(QString importKml         READ importKml    NOTIFY signalImportKml)
    QString importKml();


public:
    explicit Locations(QObject *parent = nullptr);
    typGpsCooDegree getCooDegree();
    void getCoordinateOpenstreet();

    Q_INVOKABLE void setLocationStyle(int locationStyle);

    Q_INVOKABLE void setLatLonFormat(int format);
    Q_INVOKABLE void onLatLonChanged(const QString &latLon);
    void setLatitude(QString latitude);
    void setLongitude(QString longitude);
    Q_INVOKABLE void setLatitude2(QString latitude);
    Q_INVOKABLE void setLongitude2(QString longitude);

    Q_INVOKABLE void setFuzzySearch2(QString fuzzySearch);
    Q_INVOKABLE void clearFuzzySearch(QString fuzzySearch);

    Q_INVOKABLE void setStreet2(QString street);
    Q_INVOKABLE void setCity2(QString street);
    Q_INVOKABLE void setProvince2(QString street);
    Q_INVOKABLE void setCountry2(QString street);

    Q_INVOKABLE void setImportKml2(QString importKml);

    Q_INVOKABLE void confirmClicked();
    Q_INVOKABLE void cancelClicked();
    Q_INVOKABLE void importKmlClicked();



private:
    void confirmLocation();
    void parseLatLonFromPaste(QString str);

    void clearAllText();

    typGpsCooDegree locationUseLatiLon();
    double DDToDecimalDegrees(const QString &str,  const QRegularExpression &regex, bool &success);
    double DMSToDecimalDegrees(const QString &str, const QRegularExpression &regex, bool &success);
    double DDMToDecimalDegrees(const QString &str, const QRegularExpression &regex, bool &success);


public slots:
    void slot_SearchTriggered();
    void slot_locationResponseOpenStreet();
    void slot_locationResponseOpenStreetVague();
    void slot_locationGoogle();
    void slot_locationResponseGoogle();
    void slot_exportKmlPathSelected(QString path);


signals:
    void signalLatitude();
    void signalLongitude();
    void signalFuzzySearch();
    void signalFuzzyResults();
    void signalStreet();
    void signalCity();
    void signalProvince();
    void signalCountry();
    void signalImportKml();

    void signalLocationGoogle();

    void signalLocationStyle(int style_);

    void signalCloseLocation();

private:
    int locationStyle_ = 0;      // 0表示输入经纬度   1表示地名   2表示kmz/l导入
    int m_latLonFormat_ = 0;     // DD = 0,  DMS = 1,  DDM = 2
    QString latitude_;
    QString longitude_;
    typGpsCooDegree gpsCoor_;

    QTimer *searchTimer_,*checkTimer_;
    QStringList fuzzyResult_;
    QString fuzzySearch_,street_,city_, province_, country_;
    QString importKml_;
    QNetworkAccessManager *manager_;

};












/*-------------------------------------------------------AndroidFileHelper-----------------------------------------------*/
class AppController : public QObject
{
    Q_OBJECT
public:
    static AppController* instance() {
        static AppController instance;
        return &instance;
    }

signals:
    void safResultReceived(const QString& uri,const QString& operationType);
    void signalFileSelected(QString path);
    void signalSaveKmlFile(const QString& uri,const QString& operationType);


};















#endif // LOCATION_H
