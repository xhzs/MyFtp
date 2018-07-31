#include "usermana.h"
#include "ui_usermana.h"

usermana::usermana(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::usermana)
{
    connected = false;
    ui->setupUi(this);
    connect(ui->addButton, SIGNAL(pressed()), this, SLOT(on_addButton_clicked()));
    connect(ui->delButton, SIGNAL(pressed()), this, SLOT(on_delButton_clicked()));
    connect(ui->exitButton, SIGNAL(pressed()), this, SLOT(on_exitButton_clicked()));
}

usermana::~usermana()
{
    delete ui;
}

void usermana::on_connButton_clicked(){
    if(!connected) {
        sqlUser = ui->sqlUser->text();
        sqlPassword = ui->sqlPassword->text();
        sqlDatabase = ui->sqlDatabase->text();

        db = QSqlDatabase::addDatabase("QMYSQL");
        db.setHostName(server);
        db.setUserName(sqlUser);
        db.setPassword(sqlPassword);
        db.setDatabaseName(sqlDatabase);
        if(0 == db.open()){
            info("open database wrong!");
            db.close();
        }
        info("open database success!");
        connected = true;
        ui->connButton->setText("discon");
        showUser();
    }
    else {
        db.close();
        connected = false;
        info("database has disconnect!");
        ui->connButton->setText("connect");
    }
}

void usermana::on_addButton_clicked(){
    if(false == connected){
        QMessageBox::warning(this, tr("Warning"), tr("Please connect to sql!"));
        return ;
    }

    bool ok;
    QSqlQuery query;
    std::string addName = QInputDialog::getText(this,tr("Add user"),tr("Add username"),QLineEdit::Normal,tr("enter add username.."),&ok).toStdString();
    if(ok && !addName.empty()){
        std::string passwd = QInputDialog::getText(this,tr("Add user"),tr("Password"),QLineEdit::Password,tr("..."),&ok).toStdString();;
        std::string repasswd = QInputDialog::getText(this,tr("Add user"),tr("Check password"),QLineEdit::Password,tr("..."),&ok).toStdString();;
        if(ok && (passwd == repasswd)){
            query.prepare("insert into user values(?,?,?)");
            const char* salt = passwd.substr(0,2).c_str();
            char *password = crypt(passwd.c_str(), salt);
            query.bindValue(0, addName.c_str());
            query.bindValue(1, password);
            query.bindValue(2, salt);
            if(false == query.exec()){
                info("Add user wrong[" + QString::fromStdString(addName) + "]");
                return;
            }
            info("Add user success[" + QString::fromStdString(addName) + "]");
            showUser();
        }
        else{
            info("Password wrong[" + QString::fromStdString(repasswd) + "]");
        }
    }
    else
        info("Name wrong[" + QString::fromStdString(addName) + "]");
}

void usermana::on_delButton_clicked(){
    if(false == connected){
        QMessageBox::warning(this, tr("Warning"), tr("Please connect to sql!"));
        return ;
    }

    bool ok;
    QSqlQuery query;
    std::string delName = QInputDialog::getText(this,tr("Inputing..."),tr("Delete username"),QLineEdit::Normal,tr("Enter del username"),&ok).toStdString();
    if(ok && !delName.empty()){
        std::string passwd = QInputDialog::getText(this,tr("Del user"),tr("Password"),QLineEdit::Password,tr("..."),&ok).toStdString();
        std::string repasswd = QInputDialog::getText(this,tr("Del user"),tr("Check password"),QLineEdit::Password,tr("..."),&ok).toStdString();
        if(ok && (passwd == repasswd)){
            query.prepare("delete from user where user_name=?");
            query.bindValue(0, delName.c_str());
            if(false == query.exec()){
                info("Delete user wrong[" + QString::fromStdString(delName) + "]");
                return;
            }
            info("Delete user success[" + QString::fromStdString(delName) + "]");
            showUser();
        }
        else{
            info("Password wrong[" + QString::fromStdString(repasswd) + "]");
        }
    }
    else
        info("Name wrong[" + QString::fromStdString(delName) + "]");
}

void usermana::on_exitButton_clicked(){
    db.close();
    ui->infoEdit->clear();
    this->accept();
}

void usermana::info(QString information){
    QTextCursor cursor = ui->infoEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->infoEdit->setTextCursor(cursor);
    ui->infoEdit->setText(information);
}

void usermana::showUser(){
    QSqlQuery query;

    query.prepare("select user_name from user");
    if(false == query.exec()){
        info("Sql wrong!");
        return;
    }
    QString userList;
    while (query.next()) {
        userList += query.value(0).toString() + "\n";
    }
    info(userList);
}
