#ifndef CONNDLG_H
#define CONNDLG_H

#include <QDialog>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QtSql>

class QSqlError;

class ConnDlg : public QDialog
{
    Q_OBJECT

public:
    ConnDlg(QWidget *parent = 0);
    ~ConnDlg();

    QString driverName() const;
    QString dbName() const;
    QString userName() const;
    QString psd() const;
    QString hostName() const;

    int port() const;

    QSqlError addConnection(const QString &driver, const QString &dbName,
                            const QString &host, const QString &user, const QString &psd, int port = -1);

    void addSqliteConnection();

    void createUserDB();
    void createDB();
    void createDB2();

    QSqlDatabase getDatabase();

private slots:
    void ConnectButtonClicked();
    void CancelButtonClicked();
    void driverChanged(const QString &);

private:
    QLabel *driverLabel;
    QLabel *dbLabel;
    QLabel *nameLabel;
    QLabel *psdLabel;
    QLabel *hostLabel;
    QLabel *portLabel;
    QLabel *statusLabel;

    QComboBox *comboDriver;
    QLineEdit *dbEdit;
    QLineEdit *nameEdit;
    QLineEdit *psdEdit;
    QLineEdit *hostEdit;
    QLineEdit *portEdit;

    QPushButton *button_conn;
    QPushButton *button_cancel;

    QSqlDatabase db;
};

#endif // CONNDLG_H
