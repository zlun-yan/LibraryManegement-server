#ifndef ADDDIALOG_H
#define ADDDIALOG_H

#include <QDialog>
#include <QtGui>
#include <QtSql>
#include <QGroupBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QtWidgets/QDialogButtonBox>

#include <QSqlDatabase>

class AddDialog : public QDialog
{
    Q_OBJECT

public:
    AddDialog(const QSqlDatabase &database, QSqlRelationalTableModel *book, QSqlTableModel *type, QWidget *parent = 0);
    QString getBookname();
    QString getType();
    QString getPress();
    QString getWriter();
    int getYear();
    int getRestnum();

private:
    int addNewBook(const QString &name, const QString &press, int typeId, const QString &type, const QString &writer);

    QDialogButtonBox *createButtons();

    int findTypeId(const QString &type);
    int generateBookId();

    QSqlRelationalTableModel *bookModel;
    QSqlTableModel *bookTypeModel;

    QLineEdit *bookEditor;
    QLineEdit *typeEditor;
    QLineEdit *pressEditor;
    QLineEdit *writerEditor;
    QSpinBox *yearEditor;
    QSpinBox *restNumEditor;
    QTextEdit *detailsEditor;

    QSqlDatabase db;

private slots:
    void revert();
    void submit();
};

#endif // ADDDIALOG_H
