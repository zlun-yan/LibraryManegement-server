#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGroupBox>
#include <QTableView>
#include <QListWidget>
#include <QLabel>

#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>

#include <QSqlRelationalTableModel>
#include <QSqlTableModel>

#include <QSqlDatabase>
#include "server.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const QSqlDatabase &database, const QString &userTable, const QString &bookTable, const QString &bookTypeTable, QWidget *parent = 0);
    ~MainWindow();

private:
    QGroupBox *createQueryGroupBox();
    QGroupBox *createBookGroupBox();
    QGroupBox *createBookTypeGroupBox();
    QGroupBox *createUserGroupBox();
    QGroupBox *createDetailsGroupBox();
    QGroupBox *createOperatorGroupBox();
    void createMenuBox();
    QModelIndex indexOfType(const QString &type);

    void removeBookFromDatabase(QModelIndex index, QString name);
    void removeUserFromDatabase(QModelIndex index, QString name);
    void decreaseBookCount(QModelIndex index, QString type);

    int generateBookId();
    int generateTypeId();

    QGroupBox *book;
    QGroupBox *bookType;
    QGroupBox *user;

    QGroupBox *details;
    QGroupBox *query;
    QGroupBox *oper;

    QString queryTarget = "NOT";

    QLineEdit *queryEdit;
    QComboBox *queryCombo;
    QPushButton *confirmButton;
    QComboBox *typeCombo;

    QTableView *bookView;
    QTableView *typeView;
    QTableView *userView;

    QTextEdit *profileEdit;
    QTextEdit *operEdit;
    QLabel *profileLabel;

    QSqlTableModel *bookTypeModel;
    QSqlRelationalTableModel *bookModel;
    QSqlTableModel *userModel;

    QSqlDatabase db;

    Server *server;

    bool delKey = true;

private slots:
    void updateServer(QString);

    void confirmButtonClicked();
    void showBookDetails(QModelIndex index);
    void showUserDetails(QModelIndex index);
    void queryChanged(const QString &text);

    void changeType(QModelIndex index);
    void showTypeProfile(QModelIndex index);

    void addBook();
    void addType();
    void delBook();
    void delType();
    void addUser();
    void delUser();

signals:
    void response_mainpage(QString);
};

#endif // MAINWINDOW_H
