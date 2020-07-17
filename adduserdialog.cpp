#include "adduserdialog.h"

#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

int uniqueUserId;

AddUserDialog::AddUserDialog(const QSqlDatabase &database, QSqlTableModel *user, QWidget *parent)
    : QDialog(parent)
{
    db = database;

    userModel = user;

    userEditor = new QLineEdit;
    permissionEditor = new QComboBox;
    permissionEditor->addItem(tr("普通用户"));
    permissionEditor->addItem(tr("管理员"));
    permissionEditor->setView(new QListView());
    connect(permissionEditor, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(permissionChanged(const QString &)));

    QLabel *userLabel = new QLabel;
    userLabel->setText(tr("用户名:(密码默认为与用户名相同)"));
    QLabel *permissionLabel = new QLabel;
    permissionLabel->setText(tr("权限:"));

    QDialogButtonBox *buttonBox = createButtons();

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(userLabel);
    layout->addWidget(userEditor);
    layout->addWidget(permissionLabel);
    layout->addWidget(permissionEditor);
    layout->addWidget(buttonBox);

    setLayout(layout);
    setWindowTitle(tr("添加用户"));
}

void AddUserDialog::submit()
{
    QString user = userEditor->text();

    if(user.isEmpty())
    {
        QMessageBox::information(this, tr("添加用户"), tr("请输入用户名！"));
    }
    else
    {
        addNewUser(user);
        accept();
    }
}

void AddUserDialog::addNewUser(const QString &user)
{
    QSqlQuery query(db);
    query.exec(tr("select * from userTable where username = '%1'").arg(user));
    if (query.next())
    {
        QMessageBox::information(this, tr("添加用户"), tr("'%1'已存在").arg(user));
        return;
    }

    int id = generateUserId();
    query.exec(tr("insert into userTable values(%1, '%2', '%3', '%4', %5, null, null)").arg(id).arg(user).arg(user).arg(permission).arg(0));
}

int AddUserDialog::generateUserId()
{
    uniqueUserId += 1;
    return uniqueUserId;
}

QDialogButtonBox *AddUserDialog::createButtons()
{
    QPushButton *closeButton = new QPushButton(tr("关闭"));
    QPushButton *submitButton = new QPushButton(tr("提交"));

    closeButton->setDefault(true);

    closeButton->setStyleSheet("background-color: rgb(231, 76, 60);color: rgb(255, 251, 240)");
    submitButton->setStyleSheet("background-color: rgb(26, 188, 156);color: rgb(255, 251, 240)");

    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(submitButton, SIGNAL(clicked()), this, SLOT(submit()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    buttonBox->addButton(submitButton, QDialogButtonBox::ResetRole);
    buttonBox->addButton(closeButton, QDialogButtonBox::RejectRole);

    return buttonBox;
}

void AddUserDialog::permissionChanged(const QString &per)
{
    if(per == "管理员")
    {
        permission = "manager";
    }
    else if (per == "普通用户")
    {
        permission = "student";
    }
}

QString AddUserDialog::getUSer()
{
    return userEditor->text();
}

QString AddUserDialog::getPermission()
{
    return permission;
}
