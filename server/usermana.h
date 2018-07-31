#ifndef USERMANA_H
#define USERMANA_H

#include <QDialog>
#include <iostream>
#include <QInputDialog>
#include <QBitArray>
#include <string>
#include <qsqldatabase.h>
#include <QMessageBox>
#include <QSqlQuery>
#include <unistd.h>

namespace Ui {
class usermana;
}

class usermana : public QDialog
{
    Q_OBJECT

public:
    explicit usermana(QWidget *parent = 0);
    ~usermana();

private slots:
    void on_addButton_clicked();
    void on_delButton_clicked();
    void on_exitButton_clicked();
    void on_connButton_clicked();

private:
    Ui::usermana *ui;

    QString sqlUser;
    QString sqlPassword;
    QString server = "localhost";
    QString sqlDatabase;

    QSqlDatabase db;
    bool connected;

    void info(QString information);
    void showUser();
};

#endif // USERMANA_H
