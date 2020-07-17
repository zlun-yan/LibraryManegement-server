#ifndef ADDUSERDIALOG_H
#define ADDUSERDIALOG_H

#include <QDialog>
#include <QtGui>
#include <QtSql>
#include <QLineEdit>
#include <QComboBox>
#include <QtWidgets/QDialogButtonBox>

#include <QSqlDatabase>

class AddUserDialog : public QDialog
{
    Q_OBJECT

public:
    AddUserDialog(const QSqlDatabase &database, QSqlTableModel *user, QWidget *parent = 0);

    QString getUSer();
    QString getPermission();

private:
    void addNewUser(const QString &user);

    QDialogButtonBox *createButtons();

    int generateUserId();

    QSqlTableModel *userModel;

    QLineEdit *userEditor;
    QComboBox *permissionEditor;

    QString permission = "student";

    QSqlDatabase db;

private slots:
    void submit();
    void permissionChanged(const QString &per);
};

#endif // ADDUSERDIALOG_H
