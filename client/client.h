#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <fstream>
#include <QString>
#include "../common/common.h"
#include <algorithm>
#include "infothread.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <QInputDialog>

const int BUFLEN = 1000;
const int DATABUFLEN = 1000;

class Client
{
private:
    int getStateCode();
    int getPortNum();
    int getFileSize(std::string fname);
    int listPwd();
    int intoPasv();
    int recvControl(int stateCode, std::string errorInfo="0");
    int executeCmd(std::string cmd);
    void removeSpace(std::string&);

    struct sockaddr_in serverAddr;
    std::string ip_addr, username, password, INFO;
    char* buf = new char[BUFLEN];
    char* databuf = new char[DATABUFLEN];
    int port;
    int controlSocket;
    int dataSocket;
    std::string recvInfo;
    std::string nextInfo; //JUNK

public:
    Client();
    ~Client();
    int connectServer();
    int disconnect();
    int changeDir(std::string tardir);
    int uplevelDir();
    int login(QString ip_addr, int port, QString username, QString password);
    int downFile(std::string remoteName, std::string localDir);
    int upFile(std::string localName);
    int deleteFile(std::string fname);
    int deleteDir(std::string dname);
    int rename(std::string src, std::string dst);
    int mkDir(std::string name);

    InfoThread* infoThread;
    std::string pwd;
    std::vector<std::vector<std::string>> filelist;
};

#endif // CLIENT_H
