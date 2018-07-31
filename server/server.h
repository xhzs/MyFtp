#ifndef SERVER_H
#define SERVER_H

#include "common.h"
#include "serverconfig.h"
#include <random>
#include <fstream>
#include <time.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <qsqldatabase.h>
#include <QSqlQuery>
#include <string>
#include <QVariant>

const int BUFLEN = 255;
const int DATABUFLEN = 1024;

class Server
{
public:
    Server();
    ~Server();
    int setup();
    int listenClient();
    void bindClientSocket(int c);
    ServerConfig* config;

private:
    int clientSocket;
    int dataListenSocket;
    int dataSocket;
    struct sockaddr_in dataListenAddr;
    struct sockaddr_in dataAddr;
    std::string buf;
    std::string cmd;
    std::string arg;
    std::string pwd;
    std::string rootdir;
    std::string localIp;

    int getPortNum();
    int sendMessage(std::string s);
    int waitMessage(std::string s);
    int recvStr();
    int setPasv();
    bool getLocalIp();
    int removerDir(std::string dir_full_path);
    std::vector<std::string> getPwdInfo();
    std::vector<std::string> getFileSize(std::string fname);
    std::string pathConcat(std::string, std::string);
    std::string getTime();
    bool queryUser(std::string username, std::string& sqlPassword, std::string& sqlSalt);
};

#endif // SERVER_H
