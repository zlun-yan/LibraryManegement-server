#include "mainwindow.h"

#include <QGridLayout>
#include <QAbstractItemView>
#include <QHeaderView>
#include <QAction>
#include <QMenu>
#include <QMenuBar>

#include <QMessageBox>
#include <QSqlRecord>

#include <QDebug>
#include <QDateTime>

#include "adddialog.h"
#include "addtypedialog.h"
#include "adduserdialog.h"
extern int uniqueUserId;
extern int uniqueBookId;
extern int uniqueTypeId;

MainWindow::MainWindow(const QSqlDatabase &database, const QString &userTable, const QString &bookTable, const QString &bookTypeTable, QWidget *parent)
    : QMainWindow(parent)
{
    server = new Server(database, this, 8209);
    connect(server, SIGNAL(updateServer_msg(QString)), this, SLOT(updateServer(QString)));

    db = database;

    bookModel = new QSqlRelationalTableModel(this);
    bookModel->setTable(bookTable);
    bookModel->setRelation(3, QSqlRelation(bookTypeTable, "id", "typename"));
    bookModel->select();

    bookTypeModel = new QSqlTableModel(this);
    bookTypeModel->setTable(bookTypeTable);
    bookTypeModel->select();

    userModel = new QSqlTableModel(this);
    userModel->setTable(userTable);
    userModel->select();

    book = createBookGroupBox();
    bookType = createBookTypeGroupBox();
    user = createUserGroupBox();
    details = createDetailsGroupBox();
    query = createQueryGroupBox();
    oper = createOperatorGroupBox();
    oper->setFixedWidth(280);

    uniqueBookId = bookModel->rowCount();
    uniqueTypeId = bookTypeModel->rowCount();
    uniqueUserId = userModel->rowCount();

    QHBoxLayout *boxLayout = new QHBoxLayout;
    boxLayout->addWidget(bookType);
    boxLayout->addWidget(book);
    boxLayout->addWidget(user);
    user->hide();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(query);
    mainLayout->addLayout(boxLayout);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(oper);
    layout->addWidget(details);
    layout->addLayout(mainLayout);

    QWidget *widget = new QWidget;
    setCentralWidget(widget);
    widget->setLayout(layout);
    createMenuBox();

    setWindowTitle(tr("图书管理系统服务端"));
    setWindowState(Qt::WindowMaximized);
    QDateTime time = QDateTime::currentDateTime();
    QString msg = time.toString("yyyy-MM-dd hh:mm:ss dddd");
    operEdit->append(msg);
    operEdit->append(tr("服务端启动"));
}

MainWindow::~MainWindow()
{

}


void MainWindow::updateServer(QString msg)
{
    QDateTime time = QDateTime::currentDateTime();
    QString msg_date = time.toString("yyyy-MM-dd hh:mm:ss dddd");
    operEdit->append(msg_date);
    operEdit->append(msg);
}

void MainWindow::addUser()
{
    AddUserDialog *dialog = new AddUserDialog(db, userModel, this);

    if (dialog->exec() == QDialog::Accepted)
    {
        userModel->select();
        int lastRow = userModel->rowCount() - 1;
        userView->selectRow(lastRow);

        userView->scrollToBottom();
        showUserDetails(userModel->index(lastRow, 0));

        //需要显示在操作信息中的操作
        QDateTime time = QDateTime::currentDateTime();
        QString msg = time.toString("yyyy-MM-dd hh:mm:ss dddd");
        operEdit->append(msg);
        operEdit->append(tr("添加用户：%1(%2)").arg(dialog->getUSer()).arg(dialog->getPermission()));
    }

    delete dialog;
}

void MainWindow::confirmButtonClicked()
{
    if (queryTarget == "BOOK")
    {
        bookModel->select();
//        bookModel->setFilter(tr("bookname = '%1'").arg(queryEdit->text()));
        bookModel->setFilter(tr("bookname like '\%%1\%'").arg(queryEdit->text()));

        QDateTime time = QDateTime::currentDateTime();
        QString msg = time.toString("yyyy-MM-dd hh:mm:ss dddd");
        operEdit->append(msg);
        operEdit->append(tr("查询书籍：%1").arg(queryEdit->text()));
    }
    else if (queryTarget == "USER")
    {
        userModel->select();
//        userModel->setFilter(tr("username = '%1'").arg(queryEdit->text()));
        userModel->setFilter(tr("username like '\%%1\%'").arg(queryEdit->text()));

        QDateTime time = QDateTime::currentDateTime();
        QString msg = time.toString("yyyy-MM-dd hh:mm:ss dddd");
        operEdit->append(msg);
        operEdit->append(tr("查询用户：%1").arg(queryEdit->text()));
    }
    else
    {
        QMessageBox::information(this, tr("图书管理系统服务端"), tr("请选择一种关键字进行查询"));
    }

}

void MainWindow::addBook()
{
    AddDialog *dialog = new AddDialog(db, bookModel, bookTypeModel, this);

    if (dialog->exec() == QDialog::Accepted)
    {
        bookModel->select();
        int lastRow = bookModel->rowCount() - 1;
        bookView->selectRow(lastRow);

        bookView->scrollToBottom();
        showBookDetails(bookModel->index(lastRow, 0));

        //需要显示在操作信息中的操作
        QDateTime time = QDateTime::currentDateTime();
        QString msg = time.toString("yyyy-MM-dd hh:mm:ss dddd");
        operEdit->append(msg);
        operEdit->append(tr("添加书籍：%1(分区：%2 | 出版社：%3 | 出版年份：%4 | 添加数量：%5)").arg(dialog->getBookname()).arg(dialog->getType()).arg(dialog->getPress()).arg(dialog->getYear()).arg(dialog->getRestnum()));
    }
}

int MainWindow::generateBookId()
{
    uniqueBookId += 1;
    return uniqueBookId;
}

int MainWindow::generateTypeId()
{
    uniqueTypeId += 1;
    return uniqueTypeId;
}

void MainWindow::addType()
{
    AddTypeDialog *dialog = new AddTypeDialog(db, bookModel, bookTypeModel, this);

    if (dialog->exec() == QDialog::Accepted)
    {
        bookTypeModel->select();
        int lastRow = bookTypeModel->rowCount() - 1;
        typeView->selectRow(lastRow);

        typeView->scrollToBottom();
        showTypeProfile(bookTypeModel->index(lastRow, 0));

        //需要显示在操作信息中的操作
        QDateTime time = QDateTime::currentDateTime();
        QString msg = time.toString("yyyy-MM-dd hh:mm:ss dddd");
        operEdit->append(msg);
        operEdit->append(tr("添加分区：%1(位置：%2)").arg(dialog->getType()).arg(dialog->getAddress()));
    }
}

//以下为删除操作

void MainWindow::delType()
{
    if (delBookKey == false)
    {
        QMessageBox::information(this, tr("删除分区"), tr("请选择分区。"));
        return;
    }
    QModelIndexList selection  = typeView->selectionModel()->selectedRows(0);
    if(!selection.empty())
    {
        QModelIndex idIndex = selection.at(0);
        int row = idIndex.row();
        QString type = idIndex.sibling(row, 1).data().toString();

        QMessageBox::StandardButton button;
        button = QMessageBox::question(this, tr("删除分区"), QString(tr("确认删除'%1'吗?").arg(type)), QMessageBox::Yes | QMessageBox::No);

        if(button == QMessageBox::Yes)
        {
            qDebug()<<"yes";
            bookModel->select();
            int count = bookModel->rowCount();

            if(count != 0)
            {
                QMessageBox::StandardButton subbutton;
                subbutton = QMessageBox::question(this, tr("删除分区"), QString(tr("分区'%1'中已仍有书籍\n是否删除?").arg(type)), QMessageBox::Yes | QMessageBox::No);

                if(subbutton == QMessageBox::Yes)
                {
                    for (int i = 0; i < count; i++)
                    {
                        bookModel->removeRow(i);
                    }
                    bookModel->select();
                }
                else if (subbutton == QMessageBox::No)  return;
            }
        }
        else if (button == QMessageBox::No)  return;
        bookTypeModel->removeRow(row);
        bookTypeModel->select();

        QDateTime time = QDateTime::currentDateTime();
        QString msg = time.toString("yyyy-MM-dd hh:mm:ss dddd");
        operEdit->append(msg);
        operEdit->append(tr("删除分区：%1").arg(type));
    }
    else  //如果没有选择，则提示用户进行选择
    {
        QMessageBox::information(this, tr("删除分区"), tr("请选择分区。"));
    }
}

void MainWindow::delBook()
{
    if (delBookKey == false)
    {
        QMessageBox::information(this, tr("删除书籍"), tr("请选择书籍。"));
        return;
    }
    QModelIndexList selection  = bookView->selectionModel()->selectedRows(0);
    if(!selection.empty())
    {
        QModelIndex idIndex = selection.at(0);
        QString name = idIndex.sibling(idIndex.row(), 1).data().toString();
        QString type = idIndex.sibling(idIndex.row(), 3).data().toString();

        QMessageBox::StandardButton button;
        button = QMessageBox::question(this, tr("删除书籍"), QString(tr("确认删除'%1'吗?").arg(name)), QMessageBox::Yes | QMessageBox::No);

        if(button == QMessageBox::Yes)
        {
            removeBookFromDatabase(idIndex, name);
            decreaseBookCount(indexOfType(type), type);
        }
    }
    else  //如果没有选择，则提示用户进行选择
    {
        QMessageBox::information(this, tr("删除书籍"), tr("请选择书籍。"));
    }
}

void MainWindow::removeBookFromDatabase(QModelIndex index, QString name)
{
    bookModel->removeRow(index.row());

    QDateTime time = QDateTime::currentDateTime();
    QString msg = time.toString("yyyy-MM-dd hh:mm:ss dddd");
    operEdit->append(msg);
    operEdit->append(tr("删除书籍：%1").arg(name));
}

void MainWindow::decreaseBookCount(QModelIndex index, QString type)
{
    int row = index.row();
    bookModel->select();
    int count = bookModel->rowCount();

    if(count == 0)
    {
        QMessageBox::StandardButton button;
        button = QMessageBox::question(this, tr("删除分区"), QString(tr("分区'%1'中已没有书籍\n是否删除?").arg(type)), QMessageBox::Yes | QMessageBox::No);

        if(button == QMessageBox::Yes)
        {
            bookTypeModel->removeRow(row);
            bookTypeModel->select();

            QDateTime time = QDateTime::currentDateTime();
            QString msg = time.toString("yyyy-MM-dd hh:mm:ss dddd");
            operEdit->append(msg);
            operEdit->append(tr("删除分区：%1").arg(type));
        }
    }
}

//以上为删除操作

void MainWindow::queryChanged(const QString &text)
{
    if(text == "书籍")
    {
        book->show();
        bookType->show();
        user->hide();
        profileLabel->show();
        profileEdit->clear();
        queryTarget = "BOOK";

        delBookKey = true;
    }
    else if (text == "用户")
    {
        book->hide();
        bookType->hide();
        user->show();
        profileLabel->hide();
        profileEdit->clear();
        queryTarget = "USER";

        delBookKey = false;
    }
    else
    {
        queryTarget = "NOT";
    }
}

void MainWindow::showUserDetails(QModelIndex index)
{
    QSqlRecord record = userModel->record(index.row());

    QString username = record.value("username").toString();
    QString permission = record.value("permission").toString();
    QString borrownum = record.value("borrownum").toString();
    QString borrowbook = record.value("borrowbook").toString();
    QString borrowdate = record.value("borrowdate").toString();

    profileEdit->clear();
    profileEdit->append(tr("用户名：") + username);
    profileEdit->append(tr("权限：") + permission);
    profileEdit->append(tr("借书数量：") + borrownum);
    profileEdit->append(tr("所借书籍：") + borrowbook);
    profileEdit->append(tr("借出日期：") + borrowdate);
}

QGroupBox* MainWindow::createOperatorGroupBox()
{
    QGroupBox *box = new QGroupBox(tr("操作信息"));

    operEdit = new QTextEdit;
    operEdit->setReadOnly(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(operEdit, 0, 0);

    box->setLayout(layout);
    return box;
}

QGroupBox* MainWindow::createQueryGroupBox()
{
    QGroupBox *box = new QGroupBox(tr("查询"));

    queryEdit = new QLineEdit;
    confirmButton = new QPushButton;
    confirmButton->setText(tr("查询"));

    confirmButton->setStyleSheet("background-color: rgb(248, 147, 29);color: rgb(255, 251, 240)");

    connect(confirmButton, SIGNAL(clicked(bool)), this, SLOT(confirmButtonClicked()));
    queryCombo = new QComboBox;
    queryCombo->addItem(tr("<关键词>"));
    queryCombo->addItem(tr("书籍"));
    queryCombo->addItem(tr("用户"));
    queryCombo->setView(new QListView());
    connect(queryCombo, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(queryChanged(const QString &)));
    //当用户选中“书籍”的时候，就显示下面选择书籍类型的复选框

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(queryEdit);
    layout->addWidget(queryCombo);
    layout->addWidget(confirmButton);

    box->setLayout(layout);
    return box;
}

QGroupBox* MainWindow::createBookGroupBox()
{
    QGroupBox *box = new QGroupBox(tr("书籍"));

    bookView = new QTableView;
    bookView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bookView->setSortingEnabled(true);
    bookView->setSelectionBehavior(QAbstractItemView::SelectRows);
    bookView->setSelectionMode(QAbstractItemView::SingleSelection);
    bookView->setShowGrid(false);
//    bookView->verticalHeader()->hide();
    bookView->setAlternatingRowColors(true);

    bookView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    bookView->setModel(bookModel);
    connect(bookView, SIGNAL(clicked(QModelIndex)), this, SLOT(showBookDetails(QModelIndex)));
    connect(bookView, SIGNAL(activated(QModelIndex)), this, SLOT(showBookDetails(QModelIndex)));

    bookView->hideColumn(0);
    for (int i = 2; i < bookModel->columnCount() - 1; i++)
    {
        bookView->hideColumn(i);
    }

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(bookView);

    box->setLayout(layout);
    return box;
}

QGroupBox* MainWindow::createUserGroupBox()
{
    QGroupBox *box = new QGroupBox(tr("用户"));

    userView = new QTableView;
    userView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    userView->setSortingEnabled(true);
    userView->setSelectionBehavior(QAbstractItemView::SelectRows);
    userView->setSelectionMode(QAbstractItemView::SingleSelection);
    userView->setShowGrid(false);
    userView->verticalHeader()->hide();
    userView->setAlternatingRowColors(true);
    //下面这一句是设置每一列的列宽是恰好包含其中的内容
    userView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    userView->setModel(userModel);
    connect(userView, SIGNAL(clicked(QModelIndex)), this, SLOT(showUserDetails(QModelIndex)));
    connect(userView, SIGNAL(activated(QModelIndex)), this, SLOT(showUserDetails(QModelIndex)));

    userView->hideColumn(0);
    userView->hideColumn(2);
    for (int i = 4; i < userModel->columnCount(); i++)
    {
        userView->hideColumn(i);
    }

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(userView);

    box->setLayout(layout);
    return box;
}

QGroupBox* MainWindow::createDetailsGroupBox()
{
    QGroupBox *box = new QGroupBox(tr("详细信息"));

    profileLabel = new QLabel;
    profileLabel->setWordWrap(true);
    profileLabel->setAlignment(Qt::AlignBottom);
    profileLabel->setText(tr("未选择"));
    profileLabel->hide();

    profileEdit = new QTextEdit;
    profileEdit->setReadOnly(true);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(profileLabel, 0, 0, 1, 2);
    layout->addWidget(profileEdit, 1, 0, 1, 2);
//    layout->setRowStretch(2, 1);

    box->setLayout(layout);
    return box;
}

QGroupBox* MainWindow::createBookTypeGroupBox()
{
    typeView = new QTableView;
    typeView->setEditTriggers(QAbstractItemView::NoEditTriggers);  //允许用户编辑其中的字段？
    typeView->setSortingEnabled(true);

    typeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    typeView->setSelectionMode(QAbstractItemView::SingleSelection);
    typeView->setShowGrid(false);
    typeView->setAlternatingRowColors(true);
    typeView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    typeView->setModel(bookTypeModel);
    connect(typeView, SIGNAL(clicked(QModelIndex)), this, SLOT(changeType(QModelIndex)));

    typeView->hideColumn(0);

    QGroupBox *box = new QGroupBox(tr("书籍类型"));
    QGridLayout *layout = new QGridLayout;

    layout->addWidget(typeView, 0, 0);
    box->setLayout(layout);

    return box;
}

void MainWindow::createMenuBox()
{
    QAction *addAction = new QAction(tr("添加书籍"), this);
    QAction *addTypeAction = new QAction(tr("添加分区"), this);
    QAction *deleteAction = new QAction(tr("删除书籍"), this);
    QAction *deleteTypeAction = new QAction(tr("删除分区"), this);
    QAction *addUserAction = new QAction(tr("添加用户"), this);
    QAction *quitAction = new QAction(tr("退出"), this);
    //借书、还书、添加用户、

    addAction->setShortcut(tr("Ctrl+A"));
    addTypeAction->setShortcut(tr("Ctrl+Shift+A"));
    deleteAction->setShortcut(tr("Ctrl+D"));
    deleteTypeAction->setShortcut(tr("Ctrl+Shift+D"));
    addUserAction->setShortcut(tr("Ctrl+U"));
    quitAction->setShortcut(tr("Ctrl+Q"));

    QMenu *fileMenu = menuBar()->addMenu(tr("操作菜单"));
    fileMenu->addAction(addAction);
    fileMenu->addAction(addTypeAction);
    fileMenu->addAction(deleteAction);
    fileMenu->addAction(deleteTypeAction);
    fileMenu->addSeparator();
    fileMenu->addAction(addUserAction);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAction);

    connect(addAction, SIGNAL(triggered(bool)), this, SLOT(addBook()));
    connect(addTypeAction, SIGNAL(triggered(bool)), this, SLOT(addType()));
    connect(deleteAction, SIGNAL(triggered(bool)), this, SLOT(delBook()));
    connect(deleteTypeAction, SIGNAL(triggered(bool)), this, SLOT(delType()));
    connect(addUserAction, SIGNAL(triggered(bool)), this, SLOT(addUser()));
    connect(quitAction, SIGNAL(triggered(bool)), this, SLOT(close()));
}

void MainWindow::changeType(QModelIndex index)
{
    QSqlRecord record = bookTypeModel->record(index.row());
    QString typeId = record.value("id").toString();
    bookModel->setFilter("id= '" + typeId + "'");

    profileEdit->clear();
    showTypeProfile(index);
}

void MainWindow::showTypeProfile(QModelIndex index)
{
    QSqlRecord record = bookTypeModel->record(index.row());
    QString name = record.value("typename").toString();

//    int count = bookModel->rowCount();
    QSqlQuery query(db);
    query.exec(tr("select * from bookTable where type = '%1'").arg(name));
    int count = 0;
    while (query.next())
    {
        count = count + query.value(5).toInt();
    }

    QString address = record.value("address").toString();

    profileLabel->setText(tr("分区类型：%1\n书籍数量：%2\n分区地址：%3").arg(name).arg(count).arg(address));
    profileLabel->show();
}

void MainWindow::showBookDetails(QModelIndex index)
{
    QSqlRecord record = bookModel->record(index.row());

    QString name = record.value("bookname").toString();
    QString press = record.value("press").toString();
    QString writer = record.value("writer").toString();
    QString year = record.value("year").toString();
    QString rest = record.value("restnum").toString();
    QString borrow = record.value("borrow").toString();
    QString details = record.value("details").toString();
    QString type = record.value("typename").toString();

    showTypeProfile(indexOfType(type));

    profileEdit->clear();
    profileEdit->append(tr("书名：") + name);
    profileEdit->append(tr("出版社：") + press);
    profileEdit->append(tr("作者：") + writer);
    profileEdit->append(tr("出版年份：") + year);
    profileEdit->append(tr("剩余：") + rest + tr(" 本"));
    profileEdit->append(tr("已借出：") + borrow + tr(" 本"));
    profileEdit->append(tr("详细信息：\n") + details);
}

QModelIndex MainWindow::indexOfType(const QString &type)
{
    for (int i = 0; i < bookTypeModel->rowCount(); i++)
    {
        QSqlRecord record = bookTypeModel->record(i);
        if (record.value("typename") == type)  return bookTypeModel->index(i, 1);
    }

    return QModelIndex();
}
