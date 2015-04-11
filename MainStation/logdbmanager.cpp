#include "logdbmanager.h"
#include "../share/global.h"
#include "mainwindow.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

QSqlError LogDbManager::lastError;
const char *LogDbManager::EventStringTable[] = {
    "Info",
    "Warn",
    "Error",
};

bool LogDbManager::createLogDb(bool recreate)
{
    QSqlDatabase logdb = QSqlDatabase::addDatabase("QSQLITE", LOG_DB_CONNECTION_NAME);
    logdb.setDatabaseName(LOG_DB_NAME);
    if (logdb.open()) {
        if (recreate) {
            logdb.exec("DROP INDEX IX_LogTable_LogTime ON LogTable");
            logdb.exec("DROP TABLE LogTable");
        }
        logdb.exec("CREATE TABLE IF NOT EXISTS LogTable(LogTime TimeStamp NOT NULL DEFAULT(datetime(\'now\',\'localtime\')),"
                   "UserName VARCHAR(64), EventType VARCHAR(16), EventContent TEXT)");
        logdb.exec("CREATE INDEX IF NOT EXISTS IX_LogTable_LogTime ON LogTable(LogTime)");
    }

    lastError = logdb.lastError();
    return lastError.type()==QSqlError::NoError;
}

bool LogDbManager::insertMessageToDb(EventType type, const QString &msg)
{
    QSqlDatabase logdb = QSqlDatabase::database(LOG_DB_CONNECTION_NAME);
    QSqlQuery query(logdb);
    query.prepare("INSERT INTO LogTable(UserName, EventType, EventContent) VALUES(?,?,?)");
    query.addBindValue(mainWindow->getCurrentUser().name);
    query.addBindValue(QString::fromLatin1(EventStringTable[type]));
    query.addBindValue(msg);
    query.exec();
    lastError = query.lastError();
    return lastError.type()==QSqlError::NoError;
}
