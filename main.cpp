#include "mainwindow.h"
#include <QApplication>

#include "conndlg.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile file(":/qss/flatwhite.css");

    if(file.open(QFile::ReadOnly))
    {
        qDebug("open success");
        QString style = QLatin1String(file.readAll());
        a.setStyleSheet(style);
        file.close();
    }
    else
    {
        qDebug("Open failed");
    }

    ConnDlg dialog;
    dialog.setStyleSheet("QWidget {font-family:'Microsoft YaHei';font-size: 15px;}");
    if(dialog.exec() != QDialog::Accepted)  return -1;

    MainWindow window(dialog.getDatabase(), "userTable", "bookTable", "bookTypeTable");
    window.setStyleSheet("QWidget {font-family:'Microsoft YaHei';font-size: 15px;}");
    window.show();

    return a.exec();
}
