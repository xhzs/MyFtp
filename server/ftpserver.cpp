#include "ftpserver.h"
#include "ui_ftpserver.h"

using namespace std;

ftpServer::ftpServer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ftpServer){
    listenThread = new ListenThread();
    serverConfig = new ServerConfig();
    ui->setupUi(this);
    connected = false;
    ui->infoTree->setColumnWidth(0, 50);
    ui->infoTree->setColumnWidth(1, 200);
    ui->infoTree->setColumnWidth(2, 200);
    connect(listenThread, SIGNAL(emitSocket(int, QString)), this, SLOT(recvSocket(int, QString)), Qt::DirectConnection);
    connect(ui->userButton, SIGNAL(pressed()), this, SLOT(on_userButton_clicked()));
}

ftpServer::~ftpServer(){
    stopAll();
    listenThread->stop();
    delete ui;
    delete serverConfig;
    delete listenThread;
    delete userDialog;
}

void ftpServer::on_userButton_clicked(){
    userDialog = new usermana(this);
    userDialog->show();
}

void ftpServer::on_startButton_clicked(){
    string info;
    if(!connected) {
        //configure setup
        info = serverConfig->setup(ui->ipEdit->text().toStdString(),
                                   ui->portEdit->text().toStdString(), ui->pasvDown->text().toStdString(),
                                   ui->pasvUp->text().toStdString(), maxClientSlideNum,
                                   ui->allowAnony->isChecked(), wd);
        if(info!="success") {
            QMessageBox::warning(this, "Warning!", QString::fromStdString(info));
            return;
        }
        listenThread->max_client = maxClientSlideNum;
        listenThread->start();
        connected = true;
        ui->startButton->setText("End");
    }
    else {
        stopAll();
        listenThread->stop();
        connected = false;
        ui->infoTree->clear();
        ui->startButton->setText("Start");
    }
}


void ftpServer::on_dirButton_clicked(){
    QString workDir;
    workDir = QFileDialog::getExistingDirectory(this, "Please select a working direcotry.");
    wd = workDir.toStdString();
}

void ftpServer::on_maxClientSlide_sliderMoved(int position){
    maxClientSlideNum = position;
    ui->maxNum->setText(QString::fromStdString(std::to_string(maxClientSlideNum)));
}

void ftpServer::recvSocket(int socket, QString ip) {
    if(!connected) return;

    ServerThread* serverThread = new ServerThread;
    //get date and time
    QDateTime dt;
    QTime time;
    QDate date;
    dt.setTime(time.currentTime());
    dt.setDate(date.currentDate());
    serverThread->time = dt.toString("yyyy.MM.dd hh:mm:ss");
    //set current server ip and its information
    serverThread->ip = ip;
    serverThread->curServer->bindClientSocket(socket);
    serverThread->curServer->config = serverConfig;
    //add to subThread list
    subThread.push_back(serverThread);
    serverThread->num = subThread.size() - 1;
    //connect(serverThread, SIGNAL(finished()), serverThread, SLOT(stop()));
    connect(serverThread, SIGNAL(emitSubThreadStop(int)), this, SLOT(recvSubThreadStop(int)));

    //serverThread begin to work
    serverThread->start();
    flushList();
}

//flush the QTreeWidgetItem(client list)
void ftpServer::flushList() {
    ui->infoTree->clear();
    QTreeWidgetItem* item;;
    for(size_t i=0; i<subThread.size(); i++) {
        subThread[i]->num = i;
        item = new QTreeWidgetItem(ui->infoTree);
        item->setText(0, QString::fromStdString(to_string(i+1)));
        item->setText(1, subThread[i]->ip);
        item->setText(2, subThread[i]->time);
        ui->infoTree->addTopLevelItem(item);
    }
}

//delete and erase from subThread
void ftpServer::recvSubThreadStop(int num) {
    delete subThread[num];
    subThread.erase(subThread.begin()+num);
    flushList();
    listenThread->cur_client--;
}

//delete and erase all the subThread
void ftpServer::stopAll() {
    listenThread->cur_client = 0;
    for(size_t i=0; i<subThread.size(); i++) {
        subThread[i]->forceStop();
        delete subThread[i];
    }
    subThread.clear();
}
