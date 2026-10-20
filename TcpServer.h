#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

class TcpServer{
    
private:
    string ip_address;
    int port;
    int server_fd; //server_file descriptor ,  is a number (posix socket)
    struct sockaddr_in server_addr;

public:
    //Constructor to setup ip and port
    TcpServer(string ip_address , int port);

    //Destructor to ensure sockets are closed when server stops 
    ~TcpServer();

    //starts the actual engine
    void start();
};


#endif