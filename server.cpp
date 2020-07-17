#include "server.h"

#include <QHostAddress>
#include <QDebug>
#include <QMessageBox>

#include "adddialog.h"
#include "addtypedialog.h"
#include "adduserdialog.h"
extern int uniqueUserId;
extern int uniqueBookId;
extern int uniqueTypeId;

Server::Server(const QSqlDatabase &database, QObject *parent, int port)
    :QTcpServer(parent)
{
    db = database;

    QHostAddress serverIP;
    QString ip = "127.0.0.1";
    if(!serverIP.setAddress(ip))
    {
        QMessageBox::information(NULL, tr("错误"), tr("无效的IP地址"));
    }
    listen(serverIP, port);
//    listen(QHostAddress::Any, port);
}

//如果有新的客户端连接
void Server::incomingConnection(int socketDescriptor)
{
    TcpClientSocket *tcpClientSocket = new TcpClientSocket(this);
    connect(tcpClientSocket, SIGNAL(updateClients(QString, int)), this, SLOT(updateServer_server(QString, int)));
    connect(tcpClientSocket, SIGNAL(disconnected(int)), this, SLOT(slotDisconnected(int)));

    tcpClientSocket->setSocketDescriptor(socketDescriptor);

    tcpClientSocketList.append(tcpClientSocket);
}

//处理客户端descriptor传来的命令msg
void Server::updateServer_server(QString msg, int descriptor)
{
    order.clear();
    order = msg.split(";", QString::SkipEmptyParts);
    if (order.at(0) == "confirmSend")
    {
        signIn(descriptor);
    }
    else if (order.at(0) == "changePsdSend")
    {
        changePsd(descriptor);
    }
    else if (order.at(0) == "informationSend")
    {
        userInformation(order.at(1), descriptor);
    }
    else if (order.at(0) == "querySend")
    {
        clientQuery(descriptor);
    }
    else if (order.at(0) == "borrowSend")
    {
        clientBorrow(descriptor);
    }
    else if (order.at(0) == "returnSend")
    {
        clientReturn(descriptor);
    }
    else if (order.at(0) == "addBookSend")
    {
        updateAddBook(descriptor);
    }
    else if (order.at(0) == "addTypeSend")
    {
        updateAddType(descriptor);
    }
    else if (order.at(0) == "delBookSend")
    {
        updateDelBook(descriptor);
    }
    else if (order.at(0) == "delTypeSend")
    {
        updateDelType(descriptor);
    }
    else
    {
        qDebug()<<"Undo Order: "<<msg;
        emit updateServer_msg(msg);
    }
}

//对指定的客户端descriptor发送命令
void Server::updateClients(QString msg,int descriptor)
{
    for(int i = 0; i < tcpClientSocketList.count(); i++)
    {
        QTcpSocket *item = tcpClientSocketList.at(i);
        if(item->socketDescriptor() == descriptor)
        {
            item->write(msg.toUtf8().data());
            return;
        }
    }
}

void Server::slotDisconnected(int descriptor)
{
    for(int i = 0; i < tcpClientSocketList.count(); i++)
    {
        QTcpSocket *item = tcpClientSocketList.at(i);
        if(item->socketDescriptor() == descriptor)
        {
            tcpClientSocketList.removeAt(i);
            return;
        }
    }
    return;
}


void Server::signIn(int descriptor)
{
    QSqlQuery query(db);
    QString msg;

    query.exec(tr("select * from userTable where username = '%1' and password = '%2'").arg(order.at(1)).arg(order.at(2)));
    if(query.next())
    {
        msg = tr("confirmReceive;accept;%1").arg(query.value(3).toString());
        emit updateServer_msg(tr("%1: Enter").arg(order.at(1)));
    }
    else
    {
        msg = tr("confirmReceive;reject");
        emit updateServer_msg(tr("%1: EnterRejct").arg(order.at(1)));
    }
    updateClients(msg, descriptor);
}

void Server::changePsd(int descriptor)
{
    QSqlQuery query(db);
    QString msg;

    query.exec(tr("select * from userTable where username = '%1'").arg(order.at(1)));
    if(query.next())
    {
        query.exec(tr("update userTable set password = '%1' where username = '%2'").arg(order.at(2)).arg(order.at(1)));
        msg = tr("changePsdReceive;accept");
        emit updateServer_msg(tr("%1: ChangePsd").arg(order.at(1)));
    }
    else
    {
        msg = tr("changePsdReceive;reject");
        emit updateServer_msg(tr("%1: ChangePsdReject").arg(order.at(1)));
    }
    updateClients(msg, descriptor);
}

void Server::updateAddBook(int descriptor)
{
    QSqlQuery query(db);
    query.exec(tr("select * from bookTable where bookname = '%1' and press = '%2' and type = '%3' and writer = '%4' and year = %5").arg(order.at(1)).arg(order.at(2)).arg(order.at(3)).arg(order.at(4)).arg(order.at(5)));

    QString msg;
    if (query.next())
    {
        //update
        int restnum = query.value(5).toInt();
        QString numStr = order.at(6);
        restnum = restnum + numStr.toInt();
        msg = "addBookReceive;accept";

        query.exec(tr("update bookTable set restnum = %1 where bookname = '%2' and press = '%3' and type = '%4' and writer = '%5' and year = %6").arg(restnum).arg(order.at(1)).arg(order.at(2)).arg(order.at(3)).arg(order.at(4)).arg(order.at(5)));
        query.exec(tr("update bookTable set details = '%1' where bookname = '%2' and press = '%3' and type = '%4' and writer = '%5' and year = %6").arg(order.at(7)).arg(order.at(1)).arg(order.at(2)).arg(order.at(3)).arg(order.at(4)).arg(order.at(5)));

        emit updateServer_msg(tr("addBook[update];%1;%2").arg(order.at(1)).arg(restnum));
    }
    else
    {
        //insert
        query.exec(tr("select * from bookTypeTable where typename = '%1'").arg(order.at(3)));
        if(query.next())
        {
            int id = generateBookId();
            int typeId = query.value(0).toInt();
            query.exec(tr("insert into bookTable values(%1, '%2', '%3', %4, %5, %6, 0, '%7', '%8', '%9')").arg(id).arg(order.at(1)).arg(order.at(2)).arg(typeId).arg(order.at(5)).arg(order.at(6)).arg(order.at(7)).arg(order.at(3)).arg(order.at(4)));
            msg = "addBookReceive;accept";

            emit updateServer_msg(tr("addBook[insert];%1;%2").arg(order.at(1)).arg(order.at(5)));
        }
        else
        {
            msg = "addBookReceive;reject;NotFoundType";

            emit updateServer_msg(tr("addBook;reject"));
        }
    }

    updateClients(msg, descriptor);
}

void Server::updateAddType(int descriptor)
{
    QSqlQuery query(db);
    query.exec(tr("select * from bookTypeTable where typename = '%1'").arg(order.at(1)));

    QString msg;
    if (query.next())
    {
        msg = "addTypeReceive;reject";

        emit updateServer_msg(tr("addType;reject"));
    }
    else
    {
        msg = "addTypeReceive;accept";
        int id = generateTypeId();
        query.exec(tr("insert into bookTypeTable values(%1, '%2', '%3')").arg(id).arg(order.at(1)).arg(order.at(2)));

        emit updateServer_msg(tr("addType;%1;%2").arg(order.at(1)).arg(order.at(2)));
    }

    updateClients(msg, descriptor);
}

void Server::updateDelBook(int descriptor)
{
    QSqlQuery query(db);
    query.exec(tr("select * from bookTable where bookid = %1").arg(order.at(1)));

    QString msg;
    if (query.next())
    {
        msg = "delBookReceive;accept";
        query.exec(tr("delete from bookTable where bookid = %1").arg(order.at(1)));

        emit updateServer_msg(tr("deleteBook;%1").arg(order.at(1)));
    }
    else
    {
        msg = "delBookReceive;reject";

        emit updateServer_msg(tr("deleteBook;reject"));
    }

    updateClients(msg, descriptor);
}

void Server::updateDelType(int descriptor)
{
    QSqlQuery query(db);
    query.exec(tr("select * from bookTypeTable where typename = '%1'").arg(order.at(1)));

    QString msg;
    if (query.next())
    {
        msg = "delTypeReceive;accept";
        int typeId = query.value(0).toInt();
        query.exec(tr("delete from bookTable where typeid = %1").arg(typeId));
        query.exec(tr("delete from bookTypeTable where typename = '%1'").arg(order.at(1)));

        emit updateServer_msg(tr("delType;%1").arg(order.at(1)));
    }
    else
    {
        msg = "delTypeReceive;reject";

        emit updateServer_msg(tr("delType;rejct"));
    }

    updateClients(msg, descriptor);
}

void Server::userInformation(QString userName, int descriptor)
{
    QSqlQuery query(db);

    query.exec(tr("select * from userTable where username = '%1'").arg(userName));

    if (query.next())
    {
        QString permission = query.value(3).toString();
        QString borrowNum = query.value(4).toString();
        QString totBorrowBook = query.value(5).toString();
        QString totBorrowDate = query.value(6).toString();

        QString msg = tr("informationReceive;%1;%2;%3;%4").arg(permission).arg(borrowNum).arg(totBorrowBook).arg(totBorrowDate);
        updateClients(msg, descriptor);
    }
}

void Server::clientQuery(int descriptor)
{
    QSqlQuery query(db);

    QString bookid;
    QString bookname;
    QString bookpress;
    QString booktype;
    QString bookwriter;
    QString bookyear;
    QString booknum;
    QString bookdetails;

    bool key = false;

    if (order.at(1) == "bookname")
    {
        query.exec(tr("select * from bookTable where bookname like '\%%1\%'").arg(order.at(2)));
    }
    else if (order.at(1) == "type")
    {
        query.exec(tr("select * from bookTable where type like '\%%1\%'").arg(order.at(2)));

    }
    else if (order.at(1) == "press")
    {
        query.exec(tr("select * from bookTable where press like '\%%1\%'").arg(order.at(2)));
    }
    else if (order.at(1) == "writer")
    {
        query.exec(tr("select * from bookTable where writer like '\%%1\%'").arg(order.at(2)));
    }

    if (query.next())
    {
        key = true;
        bookid = query.value(0).toString();
        bookname = query.value(1).toString();
        bookpress = query.value(2).toString();
        booktype = query.value(8).toString();
        bookwriter = query.value(9).toString();
        bookyear = query.value(4).toString();
        booknum = query.value(5).toString();
        bookdetails = query.value(7).toString();
    }
    while (query.next())
    {
        bookid = bookid + "|" + query.value(0).toString();
        bookname = bookname + "|" + query.value(1).toString();
        bookpress = bookpress + "|" + query.value(2).toString();
        booktype = booktype + "|" + query.value(8).toString();
        bookwriter = bookwriter + "|" + query.value(9).toString();
        bookyear = bookyear + "|" + query.value(4).toString();
        booknum = booknum + "|" + query.value(5).toString();
        bookdetails = bookdetails + "|" + query.value(7).toString();
    }

    QString msg;
    if (key)
    {
        msg = "queryReceive;accept;" + bookid + ";" + bookname + ";" + bookpress + ";" + booktype + ";" + bookwriter + ";" + bookyear + ";" + booknum + ";" + bookdetails;
    }
    else
    {
        msg = "queryReceive;reject";
    }

    updateClients(msg, descriptor);
}

void Server::clientBorrow(int descriptor)
{
    QSqlQuery query(db);
    query.exec(tr("select * from bookTable where bookid = %1").arg(order.at(2)));

    QString msg;
    if(query.next())
    {
        int num = query.value(5).toInt();
        int borrow = query.value(6).toInt();
        QString bookname = query.value(1).toString();
        QDateTime time = QDateTime::currentDateTime();
        QString bookdate = time.toString("yyyy-MM-dd");

        query.exec(tr("select * from userTable where username = '%1'").arg(order.at(1)));
        query.next();
        int borrownum = query.value(4).toInt();
        QString prebookname = query.value(5).toString();
        QString prebookdate = query.value(6).toString();

        num = num - 1;
        borrow = borrow + 1;
        borrownum = borrownum + 1;
        bookname = bookname + "|" + prebookname;
        bookdate = bookdate + "|" + prebookdate;

        if (num >= 0)
        {
            msg = "borrowReceive;accept";
            query.exec(tr("update bookTable set restnum = %1 where bookid = %2").arg(num).arg(order.at(2)));
            query.exec(tr("update bookTable set borrow = %1 where bookid = %2").arg(borrow).arg(order.at(2)));

            query.exec(tr("update userTable set borrownum = '%1' where username = '%2'").arg(borrownum).arg(order.at(1)));
            query.exec(tr("update userTable set borrowbook = '%1' where username = '%2'").arg(bookname).arg(order.at(1)));
            query.exec(tr("update userTable set borrowdate = '%1' where username = '%2'").arg(bookdate).arg(order.at(1)));

            emit updateServer_msg(tr("borrowBook;%1;%2").arg(order.at(1)).arg(order.at(2)));
        }
        else
        {
            msg = "borrowReceive;reject";

            emit updateServer_msg(tr("borrowBook;reject"));
        }
    }
    else
    {
        msg = "borrowReceive;reject";

        emit updateServer_msg(tr("borrowBook;reject"));
    }

    updateClients(msg, descriptor);
}

void Server::clientReturn(int descriptor)
{
    QSqlQuery query(db);
    query.exec(tr("select * from bookTable where bookid = %1").arg(order.at(2)));
    QString msg;

    if (query.next())
    {
        QString bookname = query.value(1).toString();
        int restnum = query.value(5).toInt();
        int borrow = query.value(6).toInt();

        query.exec(tr("select * from userTable where username = '%1'").arg(order.at(1)));
        if (query.next())
        {
            int index = -1;
            int borrownum = query.value(4).toInt();
            QString prebookname = query.value(5).toString();
            QString prebookdate = query.value(6).toString();
            QStringList totbook = prebookname.split("|");
            QStringList totdate = prebookdate.split("|");
            for (int i = 0; i < totbook.count(); i++)
            {
                if (totbook.at(i) == bookname)
                {
                    index = i;
                    break;
                }
            }
            if (index == -1)
            {
                msg = "returnReceive;reject";

                emit updateServer_msg(tr("returnBook;reject"));
            }
            else
            {
                msg = "returnReceive;accept";

                restnum = restnum + 1;
                borrow = borrow - 1;
                query.exec(tr("update bookTable set restnum = %1 where bookid = %2").arg(restnum).arg(order.at(2)));
                query.exec(tr("update bookTable set borrow = %1 where bookid = %2").arg(borrow).arg(order.at(2)));

                totbook.removeAt(index);
                totdate.removeAt(index);
                prebookname = totbook.join("|");
                prebookdate = totdate.join("|");
                borrownum = borrownum - 1;
                query.exec(tr("update userTable set borrownum = '%1' where username = '%2'").arg(borrownum).arg(order.at(1)));
                query.exec(tr("update userTable set borrowbook = '%1' where username = '%2'").arg(prebookname).arg(order.at(1)));
                query.exec(tr("update userTable set borrowdate = '%1' where username = '%2'").arg(prebookdate).arg(order.at(1)));

                emit updateServer_msg(tr("returnBook;%1;%2").arg(order.at(1)).arg(order.at(2)));
            }
        }
        else
        {
            msg = "returnReceive;reject";

            emit updateServer_msg(tr("returnBook;reject"));
        }
    }
    else
    {
        msg = "returnReceive;reject";

        emit updateServer_msg(tr("returnBook;reject"));
    }

    updateClients(msg, descriptor);
}

int Server::generateBookId()
{
    uniqueBookId += 1;
    return uniqueBookId;
}

int Server::generateTypeId()
{
    uniqueTypeId += 1;
    return uniqueTypeId;
}
