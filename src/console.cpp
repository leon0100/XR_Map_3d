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



/*------------------------------------Console--------------------------------------------*/
// Console::Console() : m_list(new ConsoleListModel())
// {
//     m_list->init();
// }

// ConsoleListModel *Console::listModel() const
// {
//     return m_list;
// }

// void Console::put(QtMsgType type, const QString &msg)
// {
//     const QString time = QTime::currentTime().toString(QStringLiteral("hh:mm:ss:zzz"));
//     m_list->appendEvent(time, type, msg);
// }
