#include "conndlg.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

ConnDlg::ConnDlg(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("数据库连接"));

    //布局的初始化
    driverLabel = new QLabel;
    driverLabel->setText(tr("驱动："));
    dbLabel = new QLabel;
    dbLabel->setText(tr("数据库名："));
    nameLabel = new QLabel;
    nameLabel->setText(tr("用户名："));
    psdLabel = new QLabel;
    psdLabel->setText(tr("密码："));
    hostLabel = new QLabel;
    hostLabel->setText(tr("主机名："));
    portLabel = new QLabel;
    portLabel->setText(tr("端口："));
    statusLabel = new QLabel;
    statusLabel->setText(tr("状态："));

    comboDriver = new QComboBox;
    dbEdit = new QLineEdit;
    nameEdit = new QLineEdit;
    psdEdit = new QLineEdit;
    hostEdit = new QLineEdit;
    portEdit = new QLineEdit;

    QStringList drivers  = QSqlDatabase::drivers();
    comboDriver->addItems(drivers);

    comboDriver->setView(new QListView());

    dbEdit->setEnabled(false);
    nameEdit->setEnabled(false);
    psdEdit->setEnabled(false);
    hostEdit->setEnabled(false);
    portEdit->setEnabled(false);

    connect(comboDriver, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(driverChanged(const QString &)));
    statusLabel->setText(tr("准备连接数据库..."));

    button_conn = new QPushButton;
    button_conn->setText(tr("连接"));
    button_cancel = new QPushButton;
    button_cancel->setText(tr("取消"));

    button_cancel->setStyleSheet("background-color: rgb(231, 76, 60);color: rgb(255, 251, 240)");
    button_conn->setStyleSheet("background-color: rgb(26, 188, 156);color: rgb(255, 251, 240)");

    connect(button_conn, &QPushButton::clicked, this, &ConnDlg::ConnectButtonClicked);
    connect(button_cancel, &QPushButton::clicked, this, &ConnDlg::CancelButtonClicked);

    QVBoxLayout *labelLayout = new QVBoxLayout;
    labelLayout->addWidget(driverLabel);
    labelLayout->addWidget(dbLabel);
    labelLayout->addWidget(nameLabel);
    labelLayout->addWidget(psdLabel);
    labelLayout->addWidget(hostLabel);
    labelLayout->addWidget(portLabel);

    QVBoxLayout *inputLayout = new QVBoxLayout;
    inputLayout->addWidget(comboDriver);
    inputLayout->addWidget(dbEdit);
    inputLayout->addWidget(nameEdit);
    inputLayout->addWidget(psdEdit);
    inputLayout->addWidget(hostEdit);
    inputLayout->addWidget(portEdit);

    QHBoxLayout *compLayout = new QHBoxLayout;
    compLayout->addLayout(labelLayout);
    compLayout->addLayout(inputLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(button_conn);
    buttonLayout->addWidget(button_cancel);
    buttonLayout->setAlignment(Qt::AlignRight);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(compLayout);
    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
    //布局的初始化
}

ConnDlg::~ConnDlg()
{

}

void ConnDlg::driverChanged(const QString &text)
{
    if(text == "QSQLITE")
    {
        dbEdit->setEnabled(false);
        nameEdit->setEnabled(false);
        psdEdit->setEnabled(false);
        hostEdit->setEnabled(false);
        portEdit->setEnabled(false);
    }
    else
    {
        dbEdit->setEnabled(true);
        nameEdit->setEnabled(true);
        psdEdit->setEnabled(true);
        hostEdit->setEnabled(true);
        portEdit->setEnabled(true);
    }
}

QString ConnDlg::driverName() const
{
    return comboDriver->currentText();
}

QString ConnDlg::dbName() const
{
    return dbEdit->text();
}

QString ConnDlg::userName() const
{
    return nameEdit->text();
}

QString ConnDlg::psd() const
{
    return psdEdit->text();
}

QString ConnDlg::hostName() const
{
    return hostEdit->text();
}

int ConnDlg::port() const
{
    return portEdit->text().toInt();
}

void ConnDlg::ConnectButtonClicked()
{
    if(comboDriver->currentText().isEmpty())  //检测用户是否选择了一个数据库驱动
    {
        statusLabel->setText(tr("请选择一个数据库驱动！"));
        comboDriver->setFocus();
    }
    else if(comboDriver->currentText() == "QSQLITE")
    {
        addSqliteConnection();  //如果是QSQLITE 则用addSqliteConnection创建一个内存数据库
        accept();
    }
    else  //对驱动类型进行处理  这里是如果选择除QSQLITE外的情形
    {
        QSqlError error = addConnection(driverName(), dbName(), hostName(), userName(), psd(), port());
        //如果是其他类型的，则用addConnection建立一个所选的其他类型的数据库

        if(error.type() != QSqlError::NoError)  statusLabel->setText(error.text());  //如果建立过程出现错误，则显示错误信息
        else  statusLabel->setText(tr("数据库连接成功！"));

        accept();
    }
}

void ConnDlg::CancelButtonClicked()
{
    reject();
}

QSqlError ConnDlg::addConnection(const QString &driver, const QString &dbName, const QString &host, const QString &user, const QString &psd, int port)
{
    QSqlError error;
    db = QSqlDatabase::addDatabase(driver);
    db.setDatabaseName(dbName);
    db.setHostName(host);
    db.setPort(port);

    if(!db.open(user, psd))
    {
        error = db.lastError();
    }

    return error;
}

void ConnDlg::addSqliteConnection()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("databasefile.db");

    if(!db.open())
    {
        statusLabel->setText(db.lastError().text());
        return;
    }

    QSqlQuery query(db);
    query.exec("select * from sqlite_master where type = 'table' and name='userTable'");
    if (!query.next())
    {
        bool success = query.exec("create table userTable(id int primary key, username varchar, password varchar, permission varchar, borrownum int, borrowbook varchar, borrowdate varchar)");
        createUserDB();

        if(success)  qDebug()<<QObject::tr("用户表创建成功");
        else  qDebug()<<QObject::tr("用户表创建失败");
    }

    query.exec("select * from sqlite_master where type = 'table' and name='bookTable'");
    if (!query.next())
    {
        bool success = query.exec("create table bookTable(bookid int primary key, bookname varchar, press varchar, typeid int, year int, restnum int, borrow int, details varchar, type varchar, writer varchar, foreign key(typeid) references bookTypeTable)");
        createDB();

        if(success)  qDebug()<<QObject::tr("书籍表创建成功");
        else  qDebug()<<QObject::tr("书籍表创建失败");
    }

    query.exec("select * from sqlite_master where type = 'table' and name='bookTypeTable'");
    if (!query.next())
    {
        bool success = query.exec("create table bookTypeTable(id int primary key, typename varchar, address varachar)");

        createDB2();

        if(success)  qDebug()<<QObject::tr("书籍表创建成功");
        else  qDebug()<<QObject::tr("书籍表创建失败");
    }

    statusLabel->setText(tr("创建SQLite数据库成功！"));
}

void ConnDlg::createUserDB()
{
    QSqlQuery query(db);

    //id int primary key, username varchar, password varchar, permission varchar, borrownum int, borrowbook varchar, borrowdate varchar
    query.exec("insert into userTable values(0, 'admin', 'admin', 'manager', 0, null, null)");
    query.exec("insert into userTable values(1, '8209', '8209', 'student', 0, null, null)");

    qDebug()<<QObject::tr("用户表内容构建成功");
}

void ConnDlg::createDB()
{
    QSqlQuery query(db);

    //bookid int, bookname varchar, press varchar, typeid int, year int, restnum int, borrow int, details varchar, type varchar, writer varchar
    query.exec(QObject::tr("insert into bookTable values(1, '书1', '出版社1', 1, 2001, 1, 1, '太好看了吧1', '类型1', '作者1')"));
    query.exec(QObject::tr("insert into bookTable values(2, '书2', '出版社2', 2, 2002, 2, 2, '太好看了吧2', '类型2', '作者2')"));
    query.exec(QObject::tr("insert into bookTable values(3, '书3', '出版社3', 3, 2003, 3, 3, '太好看了吧3', '类型3', '作者3')"));
    query.exec(QObject::tr("insert into bookTable values(4, '书4', '出版社4', 4, 2004, 4, 4, '太好看了吧4', '类型4', '作者4')"));
    query.exec(QObject::tr("insert into bookTable values(5, '书5', '出版社5', 5, 2005, 5, 5, '太好看了吧5', '类型5', '作者5')"));
    query.exec(QObject::tr("insert into bookTable values(6, '书6', '出版社6', 6, 2006, 6, 6, '太好看了吧6', '类型6', '作者6')"));
    query.exec(QObject::tr("insert into bookTable values(7, '书7', '出版社7', 7, 2007, 7, 7, '太好看了吧7', '类型7', '作者7')"));
    query.exec(QObject::tr("insert into bookTable values(8, '书8', '出版社8', 8, 2008, 8, 8, '太好看了吧8', '类型8', '作者8')"));

    qDebug()<<QObject::tr("书籍表内容构建成功");
}

void ConnDlg::createDB2()
{
    QSqlQuery query(db);

    //id int, typename varchar, address varachar
    query.exec(QObject::tr("insert into bookTypeTable values(1, '类型1', '一楼')"));
    query.exec(QObject::tr("insert into bookTypeTable values(2, '类型2', '二楼')"));
    query.exec(QObject::tr("insert into bookTypeTable values(3, '类型3', '三楼')"));
    query.exec(QObject::tr("insert into bookTypeTable values(4, '类型4', '四楼')"));
    query.exec(QObject::tr("insert into bookTypeTable values(5, '类型5', '五楼')"));
    query.exec(QObject::tr("insert into bookTypeTable values(6, '类型6', '六楼')"));
    query.exec(QObject::tr("insert into bookTypeTable values(7, '类型7', '七楼')"));
    query.exec(QObject::tr("insert into bookTypeTable values(8, '类型8', '八楼')"));

    qDebug()<<QObject::tr("书籍类型表内容构建成功");
}

QSqlDatabase ConnDlg::getDatabase()
{
    return db;
}
