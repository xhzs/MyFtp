#include "client.h"
using namespace std;

Client::Client() {
    infoThread = new InfoThread;
}

Client::~Client() {
    close(dataSocket);
    close(controlSocket);
    delete infoThread;
    delete buf;
    delete databuf;
}

int Client::login(QString ip_addr, int port, QString username, QString password) {
    this->ip_addr = ip_addr.toStdString();
    this->username = username.toStdString();
    this->password = password.toStdString();
    this->port = port;
    return 0;
}

int Client::connectServer() {
    //create socket
    controlSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(controlSocket == -1){
        perror("socket");
        infoThread->sendInfo("Creating Control Socket Failed.\n");
        return -1;
    }
    //connect server addr_struct
    serverAddr.sin_family = AF_INET;  //IPv4
    serverAddr.sin_addr.s_addr = inet_addr(ip_addr.c_str()); //ip
    serverAddr.sin_port = htons(port);            //port
    memset(serverAddr.sin_zero, 0, sizeof(serverAddr.sin_zero));

    //connect
    if(-1 == connect(controlSocket,(struct sockaddr*)&serverAddr,sizeof(serverAddr))){
        perror("connect");
        infoThread->sendInfo("Control Socket Connecting Failed\n");
        return -1;
    }
    cout<<"Control Socket connecting is success."<<endl;
    recvControl(220);

    //用户名
    executeCmd("USER " + username);
    recvControl(331);

    //密码
    executeCmd("PASS " + password);
    recvControl(230);

    listPwd();
    return 0;
}

int Client::disconnect() {
    executeCmd("QUIT");
    recvControl(221);
    ip_addr = username = password = INFO = "";
    filelist.clear();
    memset(buf, 0, BUFLEN);
    memset(databuf, 0, DATABUFLEN);
    close(dataSocket);
    close(controlSocket);
    return 0;
}

int Client::changeDir(string tardir) {
    memset(buf, 0, BUFLEN);
    executeCmd("CWD "+tardir);
    recvControl(250);
    intoPasv();
    listPwd();
    return 0;
}

int Client::uplevelDir() {
    memset(buf, 0, BUFLEN);
    executeCmd("CDUP");
    recvControl(250);
    intoPasv();
    listPwd();
    return 0;
}

int Client::downFile(string remoteName, string localDir){
    string localFile = localDir + "/" + remoteName;
    ofstream ofile;
    ofile.open(localFile, ios_base::binary);
    intoPasv();
    getFileSize(remoteName);
    executeCmd("RETR "+remoteName);
    recvControl(150);
    memset(databuf, 0, DATABUFLEN);
    int ret = recv(dataSocket, databuf, DATABUFLEN, 0);
    while(ret>0)
    {
        ofile.write(databuf, ret);
        ret = recv(dataSocket, databuf, DATABUFLEN, 0);
    }
    ofile.close();
    close(dataSocket);
    recvControl(226);
    return 0;
}



int Client::upFile(string localName) {
    FILE* ifile = fopen(localName.c_str(), "rb");
    if(!ifile) {
        cout << "fail to open the file!\n" << endl;
        infoThread->sendInfo("fail to open the file!");
        return -1;
    }

    //get file name
    string localFileName;
    int p = localName.find_last_of("/");
    localFileName = localName.substr(p+1);

    intoPasv();
    executeCmd("STOR "+localFileName);
    recvControl(150, "Permission denied.");
    int count;
    while(!feof(ifile)){
        count = fread(databuf, 1, DATABUFLEN, ifile);
        send(dataSocket, databuf, count, 0);
    }
    memset(databuf, 0, DATABUFLEN);
    send(dataSocket, databuf, 1, 0);
    fclose(ifile);
    close(dataSocket);
    recvControl(226);
    listPwd();
    return 0;
}

int Client::deleteFile(string fname) {
    executeCmd("DELE "+fname);
    recvControl(250);
    listPwd();
    return 0;
}

int Client::deleteDir(string dname) {
    executeCmd("RMD "+dname);
    recvControl(250);
    listPwd();
    return 0;
}

int Client::rename(string src, string dst) {
    executeCmd("RNFR "+src);
    recvControl(350);
    executeCmd("RNTO "+dst);
    recvControl(250);
    listPwd();
    return 0;
}

int Client::mkDir(string name) {
    executeCmd("MKD "+name);
    recvControl(250);
    listPwd();
    return 0;
}

//private function---------------------------------------------------------
int Client::executeCmd(string cmd) {
    cmd += "\n";
    int cmdlen = cmd.size();
    infoThread->sendInfo(cmd);
    cout << "F(exe)" << cmd << endl;
    send(controlSocket, cmd.c_str(), cmdlen, 0);
    return 0;
}

int Client::recvControl(int stateCode, string errorInfo) {
    if(errorInfo.size()==1)
        errorInfo = "state code error!";
    if(nextInfo.size()==0) {
        int t;
        memset(buf, 0, BUFLEN);
        recvInfo.clear();
        int infolen = recv(controlSocket, buf, BUFLEN, 0);
        if(infolen==BUFLEN) {
            cout << "ERROR! Too long information too receive!" << endl;
            infoThread->sendInfo("ERROR! Too long information too receive!\n");
            return -1;
        }
        buf[infolen] = '\0';
        t = getStateCode();
        recvInfo = buf;
        cout << recvInfo << endl;

        // JUNK
        int temp = recvInfo.find("\n226");
        if(temp>=0) {
            nextInfo = recvInfo.substr(temp+2);
        }
        // JUNK
        infoThread->sendInfo(recvInfo);
        if(t == stateCode)
            return 0;
        else {
            cout << errorInfo << endl;
            infoThread->sendInfo(errorInfo);
            return -1;
        }
    }
    else {
        recvInfo = nextInfo;
        nextInfo.clear();
        return 0;
    }
}

//从返回信息中获取状态码
int Client::getStateCode(){
    int num=0;
    char* p = buf;
    while(p != nullptr){
        num=10*num+(*p)-'0';
        p++;
        if(*p==' ' || *p=='-'){
            break;
        }
    }
    return num;
}

//Get Data Port from “227 Entering Passive Mode (182,18,8,37,10,25).”
int Client::getPortNum(){
    int num1 = 0, num2 = 0;

    char* p=buf;
    int cnt=0;
    while( 1 ){
        if(cnt == 4 && (*p) != ','){
            if(*p<='9' && *p>='0')
                num1 = 10*num1+(*p)-'0';
        }
        if(cnt == 5){
            if(*p<='9' && *p>='0')
                num2 = 10*num2+(*p)-'0';
        }
        if((*p) == ','){
            cnt++;
        }
        ++p;
        if((*p) == ')'){
            break;
        }
    }
//    std::cout << "The data port number is " << num1*256 + num2 << std::endl;
    return num1*256+num2;
}

int Client::listPwd() {
    intoPasv();
    executeCmd("LIST -al");
    recvControl(150);
    memset(databuf, 0, DATABUFLEN);
    string fulllist;

    int ret = recv(dataSocket, databuf, DATABUFLEN-1, 0);
    while(ret>0) {
        databuf[ret] = '\0';
        fulllist += databuf;
        ret = recv(dataSocket, databuf, DATABUFLEN-1, 0);
    }
    removeSpace(fulllist);

    int lastp, lastq, p, q;
    vector<string> eachrow;
    string rawrow;
    string item;
    filelist.clear();
    p = fulllist.find("\n");
    lastp = 0;

    while(p>=0) {
        eachrow.clear();
        rawrow = fulllist.substr(lastp, p-lastp);

        q = rawrow.find(' ');
        lastq = 0;
        for(int i=0; i<8; i++) {
            item = rawrow.substr(lastq, q-lastq);
            eachrow.push_back(item);
            lastq = q + 1;
            q = rawrow.find(' ', lastq);
        }
        item = rawrow.substr(lastq);
        eachrow.push_back(item);
        filelist.push_back(eachrow);

        lastp = p + 1;
        p = fulllist.find("\n", lastp);
    }
    close(dataSocket);
    recvControl(226);
    return 0;
}

int Client::intoPasv() {
    int dataPort;
    //into PASV
    executeCmd("PASV");
    recvControl(227);
    //executeFTPCmd(227, "PASV");                //227

    //Get:(h1,h2,h3,h4,p1,p2), IP="h1,h2,h3,h4"，Port=p1*256+p2
    dataPort = getPortNum();
    //client data socket
    dataSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    serverAddr.sin_port=htons(dataPort);    //set port
    if(-1 == connect(dataSocket,(struct sockaddr*)&serverAddr,sizeof(serverAddr))){
        perror("connect");
        cout << "Data Socket connecting Failed: " << endl;
        return -1;
    }
    cout << "Data Socket connecting is success." << endl;
    return 0;
}

int Client::getFileSize(string fname) {
    executeCmd("SIZE " + fname);
    recvControl(213);

    char* p = buf;
    while(p != nullptr && *p != ' ') {
        p++;
    }
    p++;
    int num = 0;
    while(p != nullptr && *p != '\n') {
        num *= 10;
        num += (*p - '0');
        p++;
    }
    memset(buf, 0, BUFLEN);
    return num;
}

void Client::removeSpace(string & src) {
    int p, q;
    p = src.find(' ');
    while(p>=0) {
        for(q=p+1; src[q]==' '; q++);
        src.erase(p+1, q-p-1);
        p = src.find(' ', p+1);
    }
}
