#include "adddialog.h"

#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "addtypedialog.h"

int uniqueBookId;

AddDialog::AddDialog(const QSqlDatabase &database, QSqlRelationalTableModel *book, QSqlTableModel *type, QWidget *parent)
    : QDialog(parent)
{
    db = database;

    bookModel = book;
    bookTypeModel = type;

    QDialogButtonBox *buttonBox = createButtons();

    QLabel *bookLabel = new QLabel(tr("书名:"));
    QLabel *typeLabel = new QLabel(tr("分区:"));
    QLabel *pressLabel = new QLabel(tr("出版社:"));
    QLabel *writerLabel = new QLabel(tr("作者:"));
    QLabel *yearLabel = new QLabel(tr("出版时间:"));
    QLabel *numLabel = new QLabel(tr("添加数量:"));

    bookEditor = new QLineEdit;
    typeEditor = new QLineEdit;
    pressEditor = new QLineEdit;
    writerEditor = new QLineEdit;

    yearEditor = new QSpinBox;
    yearEditor->setMinimum(1900);
    yearEditor->setMaximum(QDate::currentDate().year());
    yearEditor->setValue(yearEditor->maximum());
    yearEditor->setReadOnly(false);

    restNumEditor = new QSpinBox;
    restNumEditor->setMinimum(1);
    restNumEditor->setMaximum(1000);
    restNumEditor->setValue(1);
    restNumEditor->setReadOnly(false);

    detailsEditor = new QTextEdit;
    detailsEditor->setPlaceholderText(tr("简介"));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(bookLabel, 0, 0);
    mainLayout->addWidget(bookEditor, 0, 1);
    mainLayout->addWidget(typeLabel, 1, 0);
    mainLayout->addWidget(typeEditor, 1, 1);
    mainLayout->addWidget(pressLabel, 2, 0);
    mainLayout->addWidget(pressEditor, 2, 1);
    mainLayout->addWidget(writerLabel, 3, 0);
    mainLayout->addWidget(writerEditor, 3, 1);
    mainLayout->addWidget(yearLabel, 4, 0);
    mainLayout->addWidget(yearEditor, 4, 1);
    mainLayout->addWidget(numLabel, 5, 0);
    mainLayout->addWidget(restNumEditor, 5, 1);

    mainLayout->addWidget(detailsEditor, 0, 2, 6, 1);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addLayout(mainLayout);
    layout->addWidget(buttonBox);

    setLayout(layout);
    setWindowTitle(tr("添加书籍"));
}

void AddDialog::submit()
{
    QString book = bookEditor->text();
    QString type = typeEditor->text();
    QString press = pressEditor->text();
    QString writer = writerEditor->text();

    if(book.isEmpty() || type.isEmpty() || press.isEmpty() || writer.isEmpty())
    {
        QMessageBox::information(this, tr("添加书籍"), tr("请输入书名、分区、出版社和作者！"));
    }
    else
    {
        int typeId = findTypeId(type);
        if (typeId == -1)
        {
            //询问是否需要新增一个分区
            int choice = QMessageBox::question(this, tr("添加书籍"), tr("您选择的分区'%1'不存在\n是否需要添加一个分区？").arg(type), QMessageBox::Yes | QMessageBox::No);

            if(choice == QMessageBox::Yes)
            {
                AddTypeDialog *dialog = new AddTypeDialog(db, bookModel, bookTypeModel, this);
                dialog->exec();
            }
            else return;
        }

        int result = addNewBook(book, press, typeId, type, writer);
        if (result == 1)
        {
           accept();
        }
    }
}

int AddDialog::addNewBook(const QString &name, const QString &press, int typeId, const QString &type, const QString &writer)
{
    QSqlQuery query(db);
    query.exec(tr("select * from bookTable where bookname = '%1' and press = '%2' and typeid = %3 and year = %4").arg(name).arg(press).arg(typeId).arg(yearEditor->value()));
    if (query.next())
    {
        QMessageBox::information(this, tr("添加书籍"), tr("'%1(%2)'已存在").arg(name).arg(press));
        return 0;
    }

    int id = generateBookId();
    query.exec(tr("insert into bookTable values(%1, '%2', '%3', %4, %5, %6, 0, '%7', '%8', '%9')").arg(id).arg(name).arg(press).arg(typeId).arg(yearEditor->value()).arg(restNumEditor->value()).arg(detailsEditor->toPlainText()).arg(type).arg(writer));
    return 1;
}

int AddDialog::generateBookId()
{
    uniqueBookId += 1;
    return uniqueBookId;
}

int AddDialog::findTypeId(const QString &type)
{
    int row = 0;
    while (row < bookTypeModel->rowCount())
    {
        QSqlRecord record = bookTypeModel->record(row);
        if (record.value("typename") == type)  return record.value("id").toInt();
        row++;
    }

    return -1;
}

void AddDialog::revert()
{
    bookEditor->clear();
    typeEditor->clear();
    pressEditor->clear();
    writerEditor->clear();
    yearEditor->setValue(QDate::currentDate().year());
    restNumEditor->setValue(QDate::currentDate().year());
    detailsEditor->clear();
}

QDialogButtonBox *AddDialog::createButtons()
{
    QPushButton *closeButton = new QPushButton(tr("关闭"));
    QPushButton *revertButton = new QPushButton(tr("撤销"));
    QPushButton *submitButton = new QPushButton(tr("提交"));

    closeButton->setDefault(true);

    closeButton->setStyleSheet("background-color: rgb(231, 76, 60);color: rgb(255, 251, 240)");
    revertButton->setStyleSheet("background-color: rgb(52, 152, 219);color: rgb(255, 251, 240)");
    submitButton->setStyleSheet("background-color: rgb(26, 188, 156);color: rgb(255, 251, 240)");

    connect(closeButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(revertButton, SIGNAL(clicked()), this, SLOT(revert()));
    connect(submitButton, SIGNAL(clicked()), this, SLOT(submit()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    buttonBox->addButton(submitButton, QDialogButtonBox::ResetRole);
    buttonBox->addButton(revertButton, QDialogButtonBox::ResetRole);
    buttonBox->addButton(closeButton, QDialogButtonBox::RejectRole);

    return buttonBox;
}

QString AddDialog::getBookname()
{
    return bookEditor->text();
}

QString AddDialog::getType()
{
    return typeEditor->text();
}

QString AddDialog::getPress()
{
    return pressEditor->text();
}

QString AddDialog::getWriter()
{
    return writerEditor->text();
}

int AddDialog::getYear()
{
    return yearEditor->value();
}

int AddDialog::getRestnum()
{
    return restNumEditor->value();
}
