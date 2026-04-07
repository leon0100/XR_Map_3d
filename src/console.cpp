#include "console.h"
#include "QTime"


GetInterface* GetInterface::getInterface()
{
    static GetInterface instance;
    return &instance;
}

GetInterface::GetInterface(QObject* parent) : QObject(parent)
{
}

double GetInterface::getDistance_Haversine(double currLon, double currLati, double goalLon, double goalLati)
{
    double dLat = (goalLati - currLati) * _PI_180;
    double dLon = (goalLon - currLon) * _PI_180;
    double a = pow(sin(dLat/2), 2) + cos(currLati * _PI_180) * cos(goalLati * _PI_180) * pow(sin(dLon/2), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));

    return EARTH_RADIUS * c;
}



/*------------------------------------Console-----------------------------------------------*/
Console::Console()
    :m_list(new ConsoleListModel())
{
    m_list->init();
}

ConsoleListModel *Console::listModel() const {
    return m_list;
}

void Console::put(QtMsgType type, const QString &msg) {
    const QString time = QTime::currentTime().toString(QStringLiteral("hh:mm:ss:zzz"));
    m_list->appendEvent(time, type, msg);
}
