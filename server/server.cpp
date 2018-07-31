#include "server.h"

using namespace std;

Server::Server()
{
}

Server::~Server()
{
    close(clientSocket);
    close(dataSocket);
}

//configure initiate
int Server::setup() {
    if(!config->configed) {
        cout << "Not configed" << endl;
        return -1;
    }
    rootdir = config->wd;
    pwd = rootdir;
    dataListenAddr.sin_family = AF_INET;
    dataListenAddr.sin_addr.s_addr = INADDR_ANY;
    getLocalIp();

    return 0;
}

//listen to client and reactor
int Server::listenClient() {
    int pasvPort, pasvArg1, pasvArg2;
    string pasvIp;
    sendMessage("220 Welcome to FTP server. Author: hzs");

    while(1){
        if(!waitMessage("USER"))
            return -1;

        string username, password;
        username = arg;
        //anonymous login
        if(config->allowAnony && username == "anonymous") {
            sendMessage("331 Please specify the password.");
            waitMessage("PASS");
            sendMessage("230 Login successful.");
            config->username = "anonymous";
            break;  //login successfully exit
        }
        else {
            sendMessage("331 Please specify the password.");
            waitMessage("PASS");
            password = arg;
            string sqlPassword, sqlSalt;
            bool userExit;
            if(true == (userExit = queryUser(username, sqlPassword, sqlSalt)) && 0 == strcmp(sqlPassword.c_str(), crypt(password.c_str(), sqlSalt.c_str()))){
                sendMessage("230 Login successful.");
                config->username = QString::fromStdString(username);
                break;  //login successfully exit
            }
            else {
                cout << "Get here![pass wrong]" << endl;
                if(true == userExit)
                    sendMessage("530 Username incorrect.");
                else
                    sendMessage("530 Password incorrect.");
            }
        }
    }

    while(true) {
        //recv command from client and copy to buf, then cut buf to cmd and arg
        if(recvStr() < 0)
            continue;

        if(cmd == "SYST") {
            sendMessage("215 Ubuntu 16.04");
            continue;
        }

        if(cmd == "FEAT") {
            sendMessage("211-Features:\nPASV");
            sendMessage("211 End");
            continue;
        }

        if(cmd == "PWD") {
            sendMessage("257 \"" + pwd + "\" is the current directory");
            continue;
        }

        if(cmd == "TYPE") {
            if (arg == "A")
                sendMessage("200 Switching to ASCII mode.");
            else
                sendMessage("200 Switching to xxx mode.");
            continue;
        }

        if(cmd == "PASV") {
            pasvPort = setPasv();
            pasvArg2 = pasvPort % 256;
            pasvArg1 = pasvPort / 256;
            pasvIp = localIp;
            int loc = pasvIp.find('.');
            while(loc>=0) {
                pasvIp=pasvIp.replace(loc, 1, ",");
                loc = pasvIp.find('.');
            }

            socklen_t dataAddrLen = sizeof(dataAddr);
            sendMessage("227 Entering Passive Mode (" + pasvIp +
                        ", " + to_string(pasvArg1) + "," + to_string(pasvArg2) + ").");
            dataSocket = accept(dataListenSocket, (struct sockaddr *)&dataAddr, &dataAddrLen);
            close(dataListenSocket);
            continue;
        }

        if(cmd == "LIST") {
            sendMessage("150 Here comes the directory listing.");
            if (arg == "-al") {
                string allInfo;
                string curFile;
                vector<string> sizeAndType;
                vector<string> allFiles = getPwdInfo();
                for(size_t i=0; i<allFiles.size(); i++) {
                    curFile = allFiles[i];
                    sizeAndType = getFileSize(curFile);
                    if(sizeAndType.size()==0) continue;
                    //TD...
                    allInfo += sizeAndType[1];
                    allInfo += "--------- 1 user group ";
                    allInfo += sizeAndType[0];
                    //                    allInfo += " Dec 10 14:50 ";
                    allInfo += getTime();
                    allInfo += curFile;
                    allInfo += "\n";
                }
                send(dataSocket, allInfo.c_str(), allInfo.size(), 0);
                // Must close the data socket!!!
                // the one that SENDS through data socket must close the data socket.
                close(dataSocket);
                sendMessage("226 Directory send OK.");
                continue;
            }
            else {
                cout << cmd << " " << arg << endl;
                return -1;
            }
        }

        if(cmd == "MKD") {
            if(arg.size() == 0) continue;
            if(-1 == mkdir((pwd + '/' + arg).c_str(), 0777)){
                perror("mkdir");
                sendMessage("150 MKD fail.");
            }
            else
                sendMessage("250 MKD successfully.");
            continue;
        }

        if(cmd == "CWD") {
            if(arg.size() == 0) continue;
            string toDir = arg;
            if(toDir[0]=='/')
                pwd = toDir;
            else {
                if(pwd[pwd.size()-1] != '/')
                    pwd += "/";
                pwd += toDir;
            }
            cout << "cmd(Cwd):" << pwd << endl;
            sendMessage("250 CWD successfully.");
            continue;
        }

        if(cmd == "CDUP") {
            int p = pwd.find_last_of("/");
            if(p==pwd.size()+1) {
                pwd = pwd.substr(0, p);
                p = pwd.find_last_of("/");
            }
            pwd = pwd.substr(0, p);
            cout << "cmd(cdup):" << pwd << endl;
            sendMessage("250 CDUP successfully.");
            continue;
        }

        if(cmd == "RNFR") {
            sendMessage("350 RNFR successfully.");
            string src = pathConcat(pwd, arg);
            cout << "F(RNFR) src: " << src << endl;
            waitMessage("RNTO");
            string dst = pathConcat(pwd, arg);
            if(-1 == rename(src.c_str(), dst.c_str())){
                perror("remove");
                sendMessage("150 RNTO fail.");
            }
            else {
                sendMessage("250 RNTO successfully.");
            }
            continue;
        }

        if(cmd == "DELE"){
            if(arg.size()==0)
                continue;
            cout << "cmd(DELE) " << pwd << endl;
            if(-1 == remove(pathConcat(pwd, arg).c_str())){
                perror("remove");
                sendMessage("150 DELE fail.");
            }
            else
                sendMessage("250 DELE successfully.");
            continue;
        }

        if(cmd == "RMD"){
            if(arg.size() == 0)
                continue;
            cout << "cmd(RMD) " << pwd << endl;
            if(-1 == removerDir(pwd + '/' + arg)){
                perror("remove");
                sendMessage("150 RMD fail.");
            }
            else
                sendMessage("250 RMD successfully.");
            continue;
        }

        if(cmd == "SIZE") {
            vector<string> sizeAndType = getFileSize(arg);
            if(sizeAndType.size() != 2)
                sendMessage("550 could not get file size.");
            else if(sizeAndType[1]=="d")
                sendMessage("550 Failed.");
            else
                sendMessage("213 "+sizeAndType[0]);
            continue;
        }

        if(cmd == "RETR") {
            string fullname = pathConcat(pwd, arg);
            cout << "Cmd(RETR): " << fullname << endl;
            FILE* ifile = fopen(fullname.c_str(), "rb");
            if(!ifile) {
                cout << "fail to open the file!\n";
                sendMessage("550 Failed!");
                continue;
            }
            sendMessage("150 ready to transfer.");
            char databuf[DATABUFLEN];
            int count;
            while(!feof(ifile)){
                count = fread(databuf, 1, DATABUFLEN, ifile);
                send(dataSocket, databuf, count, 0);
            }
            memset(databuf, 0, DATABUFLEN);
            fclose(ifile);
            close(dataSocket);
            sendMessage("226 transfer complete.");
            continue;
        }

        if(cmd == "STOR") {
            int ret;
            char tempbuf[DATABUFLEN+1];
            sendMessage("150 OK to send data.");
            string dstFilename = pathConcat(pwd, arg);
            cout << "Cmd(STOR): " << dstFilename << endl;
            ofstream ofile;
            ofile.open(dstFilename, ios_base::binary);
            ret = recv(dataSocket, tempbuf, DATABUFLEN, 0);
            while(ret > 0) {
                ofile.write(tempbuf, ret);
                ret = recv(dataSocket, tempbuf, DATABUFLEN, 0);
            }
            ofile.close();
            close(dataSocket);
            sendMessage("226 transfer complete.");
            continue;
        }

        //        if(cmd == "SITE") {
        //            sendMessage("550 ???");
        //            continue;
        //        }

        if(cmd == "QUIT") {
            sendMessage("221 BYE!");
            break;
        }

    }

    cout << "client quit" << endl;
    return 0;
}

//private functions-----------------------------------------------

//send message to client
int Server::sendMessage(string message) {
    message.push_back('\n');
    int ret = send(clientSocket,  message.data(), message.size(), 0);
    return ((ret >0 ) ? 0 : -1);
}

//wait and recieve message from clientSocket
int Server::waitMessage(string s) {
    int ret;
    while(true) {
        ret = recvStr();
        if(ret)
            continue;
        else
            break;
    }
    if(s == cmd)
        return -1;
    else
        return 0;
}

//recv command from client and copy to buf, then cut buf to cmd and arg
int Server::recvStr() {
    char tempbuf[BUFLEN];
    int ret = recv(clientSocket, tempbuf, BUFLEN, 0);
    if(ret<0)
        return ret;
    buf.clear();
    buf = tempbuf;
    int idx = buf.find(' ');
    int idxr = buf.find('\n');
    if(idx>0 && idx<idxr) {
        cmd = buf.substr(0, idx);
        idx++;
        arg = buf.substr(idx, idxr-idx);
    }
    else {
        cmd = buf.substr(0, idxr);
        arg = "";
    }
    //    cout << "F(recvStr) cmd : " << cmd << " arg : " << arg << endl;
    return 0;
}

//set PASV
int Server::setPasv() {
    //create socket
    dataListenSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(dataListenSocket == -1){
        perror("F(setPasv) socket");
        //        system("pause");
        return -1;
    }
    //set socket option
    int reuse = 0;
    if (setsockopt(dataListenSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0){
        perror("setsockopet");
        return -1;
    }
    //open a random port between <config->pasvDown, config->pasvUp>
    default_random_engine random(time(NULL));
    uniform_int_distribution<int> dis1(config->pasvDown, config->pasvUp);
    int pasvPort = dis1(random);
    //bind socket
    dataListenAddr.sin_family = AF_INET;
    dataListenAddr.sin_port = htons(pasvPort);
    while(bind(dataListenSocket, (struct sockaddr *)&dataListenAddr, sizeof(dataListenAddr)) == -1) {
        perror("F(setPasv) bind");
        pasvPort = dis1(random);
        dataListenAddr.sin_port = htons(pasvPort);
    }
    if(listen(dataListenSocket, 5) == -1) {
        perror("F(setPasv) listen");
        return -1;
    }
    //return the open port for data link
    return pasvPort;
}

//get local ip by hostname
bool Server::getLocalIp() {
    //    char hostname[256];
    //    char ip[256];

    //    if (gethostname(hostname, sizeof(hostname)) == -1)
    //    {
    //        perror("gethostname");
    //        return false;
    //    }
    //    struct hostent *host = gethostbyname(hostname);
    //    if (host == NULL)
    //    {
    //        perror("gethostbyname");
    //        return false;
    //    }
    //    strcpy(ip, inet_ntoa(*(in_addr*)*host->h_addr_list));
    //    localIp = ip;
    //    cout << "F(getLocalIp):localIp = " << localIp << endl;
    //    return true;

    localIp = "192.168.199.123";
    return true;
}

vector<string> Server::getPwdInfo() {
    DIR* dir;
    dirent* ptr;
    vector<string> allFiles;
    dir = opendir(pwd.c_str());
    while((ptr = readdir(dir)) != NULL)
        allFiles.push_back(ptr->d_name);
    return allFiles;
}

//get file size and type
vector<string> Server::getFileSize(string fname) {
    vector<string> sizeAndAttrib;
    const char* fullname;

    string strfullname = pathConcat(pwd, fname);
    fullname = strfullname.c_str();
    //    cout << "F(getFileSize) fullname: " << fullname << endl;
    struct stat stat_buf;
    memset(&stat_buf, 0, sizeof(stat_buf));
    if (-1 == stat(fullname, &stat_buf)){
        perror("F(getFileSize) stat");
        return sizeAndAttrib;
    }
    if (S_ISDIR(stat_buf.st_mode)) {
        sizeAndAttrib.push_back("0");
        sizeAndAttrib.push_back("d");
    }
    else {
        sizeAndAttrib.push_back(to_string(stat_buf.st_size));
        sizeAndAttrib.push_back("-");
    }
    //    cout << "F(getFileSize) " << sizeAndAttrib[0] << sizeAndAttrib[1] << endl;
    return sizeAndAttrib;
}

//concat p1, p2 to complete path
string Server::pathConcat(string p1, string p2) {
    if(p1[p1.size()-1]=='/')
        return p1+p2;
    else
        return p1+"/"+p2;
}

//bind c to the clientSocket
void Server::bindClientSocket(int c) {
    clientSocket = c;
}

string Server::getTime(){
    time_t timep;
    time (&timep);
    char tmp[64];
    strftime(tmp, sizeof(tmp), " %Y %m-%d %H:%M:%S ",localtime(&timep) );//" Dec 10 14:50 "
    return tmp;
}

int Server::removerDir(std::string dir_full_path){
    DIR* dirp = opendir(dir_full_path.c_str());
    if(!dirp){
        return -1;
    }
    struct dirent *dir;
    struct stat st;
    while((dir = readdir(dirp)) != NULL){
        if(strcmp(dir->d_name,".") == 0
                || strcmp(dir->d_name,"..") == 0){
            continue;
        }
        std::string sub_path = dir_full_path + '/' + dir->d_name;
        if(lstat(sub_path.c_str(),&st) == -1){
            cout << "rm_dir:lstat " << sub_path << " error";
            continue;
        }
        if(S_ISDIR(st.st_mode)){
            if(removerDir(sub_path) == -1) {// 如果是目录文件，递归删除
                closedir(dirp);
                return -1;
            }
            rmdir(sub_path.c_str());
        }
        else if(S_ISREG(st.st_mode)){
            unlink(sub_path.c_str());     // 如果是普通文件，则unlink
        }
        else{
            cout << "rm_dir:st_mode " << sub_path << " error";
            continue;
        }
    }
    if(rmdir(dir_full_path.c_str()) == -1) {//delete dir itself.
        closedir(dirp);
        return -1;
    }
    closedir(dirp);
    return 0;
}

bool Server::queryUser(string username, string& sqlPassword, string& sqlSalt){
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(config->server);
    db.setUserName(config->sqlUser);
    db.setPassword(config->sqlPassword);
    db.setDatabaseName(config->sqlDatabase);
    if(0 == db.open()){
        cout << "open database wrong!" << endl;
        db.close();
    }

    QSqlQuery query;
    QString sql = "select user_password,user_salt from user where user_name=\"" + QString::fromStdString(username) + "\"";
    //    cout << sql.toStdString() << endl;
    query.prepare(sql);
    if(false == query.exec()){
        db.close();
        cout << "SqlQuery wrong!" << endl;
        return false;
    }
    while(query.next()){
        sqlPassword = query.value(0).toString().toStdString();
        sqlSalt = query.value(1).toString().toStdString();
        //        cout << "sqlpassword:" << sqlPassword << "sqlsalt:" << sqlSalt << endl;
    }
    db.close();
    return true;
}
