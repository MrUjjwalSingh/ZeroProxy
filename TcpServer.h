#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/event.h> //for kqueue and kevent
#include <fcntl.h> //for making sockets non blocking
#include <vector>
#include <unordered_map>


using namespace std;

enum class RoutingStrategy{
        ROUND_ROBIN , 
        IP_HASH
};

class TcpServer{
    
private:
    string ip_address;
    int port;
    int server_fd; //server_file descriptor ,  is a number (posix socket)
    struct sockaddr_in server_addr;
    void make_socket_non_blocking(int fd);
    vector<int>backend_ports;
    size_t current_backend;
    unordered_map<int,int>backend_to_client;
    RoutingStrategy strategy;

public:
    //Constructor to setup ip and port
    TcpServer(string ip_address , int port ,RoutingStrategy strat);

    //Destructor to ensure sockets are closed when server stops 
    ~TcpServer();

    //starts the actual engine
    void start();
};


#endif