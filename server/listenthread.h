#ifndef LISTENTHREAD_H
#define LISTENTHREAD_H

#include <QThread>
#include <iostream>
#include <QString>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>

class ListenThread : public QThread
{
    Q_OBJECT
public:
    ListenThread();
    ~ListenThread();

    void listening();
    int setup();
    void stop();

    int max_client;
    int cur_client;

protected:
    void run();

private:
    bool stopped;
    int listenSocket;
    int clientSocket;
    struct sockaddr_in listenAddr;
    struct sockaddr_in remoteAddr;

private slots:

signals:
    void emitSocket(int, QString);
};

#endif // LISTENTHREAD_H
