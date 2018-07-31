#include "listenthread.h"

using namespace std;

//listen port
const int PORT = 2345;
const char *IP = "192.168.199.123";

ListenThread::ListenThread() {
    cur_client = 0;
}

ListenThread::~ListenThread() {
    close(clientSocket);
    close(listenSocket);
}

int ListenThread::setup() {
    //create socket
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(listenSocket == -1){
        perror("socket");
        return -1;
    }
    //set socket option
    int reuse = 0;
    if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0){
        perror("setsockopet");
        return -1;
    }
    //bind socket
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &listenAddr.sin_addr);
    if(bind(listenSocket, (struct sockaddr *)&listenAddr, sizeof(listenAddr)) == -1) {
        perror("bind");
        return -1;
    }
    //listenning
    if(listen(listenSocket, max_client) == -1) {
        perror("listen");
        return -1;
    }
    return 0;
}

void ListenThread::run() {
    QString ip;
    setup();
    socklen_t remoteAddrLen = sizeof(remoteAddr);
    while(true) {
        if(cur_client<max_client) {
            //accept the client from the listenSocket and get the remoteAddr
            clientSocket = accept(listenSocket, (struct sockaddr *)&remoteAddr, &remoteAddrLen);
            if(clientSocket == -1) {
                perror("accept");
                continue;
            }
            ++cur_client;
            ip = QString(QLatin1String(inet_ntoa(remoteAddr.sin_addr)));
            cout << "No." << cur_client << " client socket received!" << endl;
            emit emitSocket(clientSocket, ip); //recvSocket
            if(max_client == cur_client)
                cout << "Max client!" << endl;
        }
        msleep(100);
    }
}

void ListenThread::stop() {
    terminate();
    int a = this->isRunning();
    int b = this->isFinished();
    cout << "ListenThread isFinished " << b << endl;
    close(clientSocket);
    close(listenSocket);
}
