#ifndef LOGDBMANAGER_H
#define LOGDBMANAGER_H

#include <QSqlError>
class QString;

class LogDbManager
{
public:
    enum EventType {
        ET_Info,
        ET_Warn,
        ET_Error,
    };

    LogDbManager() {}

    static bool createLogDb(bool recreate = false);
    static bool insertMessageToDb(EventType type, const QString &msg);

    static const char *EventStringTable[];
    static QSqlError lastError;
};

#endif // LOGDBMANAGER_H
