#include "serverconfig.h"

using namespace std;

ServerConfig::ServerConfig()
{
    configed = false;
}

string ServerConfig::setup(string ip, string port,
                           string pasvDown, string pasvUp,
                           int maxClient, bool allowAnony, string wd) {
    if(ip.size() == 0)
        return "Please input ip";
    if(port.size() == 0)
        return "Please input port";
    if(wd.size() == 0)
        return "Please select a working directory";
    int pasvDownInt, pasvUpInt;
    pasvDownInt = std::stoi(pasvDown);
    pasvUpInt = std::stoi(pasvUp);

    this->pasvDown = pasvDownInt;
    this->pasvUp = pasvUpInt;
    this->maxClient = maxClient;
    this->allowAnony = allowAnony;
    this->wd = wd;
    configed = true;
    cout << "Configed success! Max client: " << maxClient << endl;
    cout << "Working directory: " << this->wd << endl;
    return "success";
}
