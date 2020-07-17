#ifndef ADDTYPEDIALOG_H
#define ADDTYPEDIALOG_H

#include <QDialog>
#include <QtGui>
#include <QtSql>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QtWidgets/QDialogButtonBox>

#include <QSqlDatabase>

class AddTypeDialog : public QDialog
{
    Q_OBJECT

public:
    AddTypeDialog(const QSqlDatabase &database, QSqlRelationalTableModel *book, QSqlTableModel *type, QWidget *parent = 0);

    QString getAddress();
    QString getType();

private:
    void addNewType(const QString &type, const QString &address);

    QDialogButtonBox *createButtons();

    int generateTypeId();

    QSqlRelationalTableModel *bookModel;
    QSqlTableModel *bookTypeModel;

    QLineEdit *addressEditor;
    QLineEdit *typeEditor;

    QSqlDatabase db;

private slots:
    void revert();
    void submit();
};

#endif // ADDTYPEDIALOG_H
