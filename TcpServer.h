#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/event.h> //for kqueue and kevent
#include <fcntl.h> //for making sockets non blocking


using namespace std;

class TcpServer{
    
private:
    string ip_address;
    int port;
    int server_fd; //server_file descriptor ,  is a number (posix socket)
    struct sockaddr_in server_addr;
    void make_socket_non_blocking(int fd);

public:
    //Constructor to setup ip and port
    TcpServer(string ip_address , int port);

    //Destructor to ensure sockets are closed when server stops 
    ~TcpServer();

    //starts the actual engine
    void start();
};


#endif