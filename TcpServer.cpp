#include "TcpServer.h"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std ;

//Constructor 
TcpServer::TcpServer(string ip_address , int port){
    this->ip_address = ip_address;
    this->port = port;

    //Creating a socket
    server_fd = socket(AF_INET , SOCK_STREAM , 0);

    if(server_fd<0){
        cerr << "Failed to generate a file descriptor(socket)" << endl;
        exit(EXIT_FAILURE);
    }

    cout << "Socket created successfully" << server_fd << endl;

    //Configuring the socket so that it can be used again immediately after killing the application avoiding address already in use for 60 seconds while restarting
    int opt = 1;
    if(setsockopt(server_fd , SOL_SOCKET,SO_REUSEADDR,&opt ,sizeof(opt))<0){
        cerr << "failed to set socket options" <<endl;
        exit(EXIT_FAILURE);
    }

    //Define the server address structure
    server_addr.sin_family = AF_INET;  //IPv4
    server_addr.sin_port = htons(this->port); //reversing the byte order so that network devices can understand

    //Converting the ip string eg:127.0.0.1 into binary and reversing it , (the os jsut copy pastes it to responses)
    if(inet_pton(AF_INET, this->ip_address.c_str(), &server_addr.sin_addr)<=0){
        cerr << "Invalid IP address provided" <<endl;
        exit(EXIT_FAILURE);
    }

    //bind the socket integer to the actual ip and port and checks that no other process has taken the port or ip

    if(bind(server_fd , (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        cerr << "Bind failed , port might be in use by another program" << endl;
        exit(EXIT_FAILURE);
    }

    //now that server is made , make it listen to req(listening mode)
    if(listen(server_fd,10)<0){
        cerr << "Listen failed" <<endl;
        exit(EXIT_FAILURE);
    }

    cout << "Server successfully listening on " << this->ip_address << ":" << this->port << endl;



}

//Destructor
TcpServer::~TcpServer(){
    close(server_fd);
    cout << "server socket closed" << endl;
}

void TcpServer::start(){
    cout << "Waiting for a connection ..." << endl ;

    while(true){
        //storing the client's info when they connect 
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr); // this is the size of the container , not the value

        //accept() blocks the program here until someone connects
        int client_fd = accept(server_fd , (struct sockaddr*)&client_addr , &client_len);

        if(client_fd<0){
            cerr << "Failed to connnect client connection" <<endl;
            return;
        }

        cout << "New client connected successfully" << endl; 

        //Creating a temporary buffer container to store the incoming packets
        char buffer[4096] = {0};

        ssize_t bytes_read = recv(client_fd , buffer , sizeof(buffer)-1 , 0);

        if(bytes_read>0){
            cout << "Client sent" << bytes_read << "bytes:\n" << endl;
            cout << buffer << endl;
        }
    

        //Hang up the phone so we can loop back for accepting next client
        close(client_fd);

    }
    
    
}
