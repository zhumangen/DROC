#ifndef ECHOSCU_H
#define ECHOSCU_H

#ifdef ECHOSCU_CPP
#define ECHOSCU_EXTERN extern
#else
#define ECHOSCU_EXTERN
#endif

class QString;
ECHOSCU_EXTERN bool echoscu(const QString &peerTitle, const QString &ourTitle,
                               const QString &hostname, int port, QString &msg);

#endif // ECHOSCU_H
