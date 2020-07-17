#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlRecord>
#include "tcpclientsocket.h"

class Server : public QTcpServer
{
    Q_OBJECT
public:
    Server(const QSqlDatabase &database, QObject *parent=0, int port = 0);
    QList<TcpClientSocket*> tcpClientSocketList;

    void updateClients(QString msg, int descriptor);

private:
    void signIn(int descriptor);
    void changePsd(int descriptor);
    void updateAddBook(int descriptor);
    void updateAddType(int descriptor);
    void updateDelBook(int descriptor);
    void updateDelType(int descriptor);
    void userInformation(QString userName, int descriptor);
    void clientQuery(int descriptor);
    void clientBorrow(int descriptor);
    void clientReturn(int descriptor);

    int generateBookId();
    int generateTypeId();

    QStringList order;
    QSqlDatabase db;

signals:
    void updateServer_msg(QString);

public slots:
    void updateServer_server(QString, int);
    void slotDisconnected(int);

protected:
    void incomingConnection(int socketDescriptor);
};

#endif // SERVER_H
