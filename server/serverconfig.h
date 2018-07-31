#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include "common.h"

class ServerConfig
{
public:
    ServerConfig();
    std::string setup(std::string ip, std::string port,
                      std::string pasvDown, std::string pasvUp,
                      int maxClient, bool allowAnony, std::string wd);

    QString username;
    int pasvDown;
    int pasvUp;
    int maxClient;
    bool allowAnony;
    std::string wd;
    bool configed;

    int port = 2345;
    std::string ip = "192.168.199.123";

    QString sqlUser = "root";
    QString sqlPassword = "iamboss";
    QString server = "localhost";
    QString sqlDatabase = "ftp";
};

#endif // SERVERCONFIG_H
