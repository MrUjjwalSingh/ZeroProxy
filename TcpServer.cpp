#include "TcpServer.h"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

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
    make_socket_non_blocking(server_fd);
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


//Applying non blocking flag to socket
void TcpServer::make_socket_non_blocking(int fd){
    //Get the current flag settings of the socket
    int flags = fcntl(fd , F_GETFL ,0);
    if(flags == -1){
        cerr << "Failed to get the socket flags" << endl;
        return;
    }
    //turn the O_NONBLOCK to 1 in the flags
    flags = flags | O_NONBLOCK;

    //set the new flags to the socket
    if(fcntl(fd , F_SETFL , flags)==-1){
        cerr << "Failed to change the flags settings to nonBlocking" << endl;
    }

}



//new start() for non blocking version
void TcpServer::start(){
    //1. Create the kqueue (our receptionist)
    int kq = kqueue();
    if(kq==-1){
        cerr << "Failed to create kqueu " << endl;
        return;
    }

    //2.Tell the receptionist to look out for
    struct kevent change_event;

    //EV_SET is a macro that configures our rule
    //Watch 'server_fd' for read events (incoming connections)
    //EV_ADD means add this rule and EV_ENABLE means turn it on
    EV_SET(&change_event , server_fd , EVFILT_READ , EV_ADD | EV_ENABLE , 0 ,0 ,NULL);

    if(kevent(kq,&change_event,1 , NULL , 0 , NULL)==-1){
        cerr << "Failed to register server socket to kqueue" << endl;
        return;
    }

    cout << "Server booted up . kqueue is waiting for events ... " <<endl;

    //We need an array to handle events and this kqueue can hand us upto 32 events at the exact milisecond
    struct kevent event_list[32];

    //3.New asynchronous event loop which is non blocking and does not freeze if the user requests gets stopped due to network or other errors
    while(true){
        int num_events = kevent(kq , NULL , 0 , event_list , 32 , NULL);
        if(num_events == -1){
            cerr << "Kqueue Error " << endl;
            continue;
        }

        //Loop through all the events that woke us up on that specific millisecond
        //not using event_list.size() ot loop because it is just a struct and very low level so .size() is not present for usage here
        for(int i=0;i<num_events;i++){   
            int current_fd = event_list[i].ident;  //ident short for identifier is simply the fd of the socket that triggered the action

            //if the event has occured on our server_socket , that means new client is trying to connect(Master Server has a new connection)
            if(current_fd == server_fd){
                struct sockaddr_in client_addr ;
                socklen_t client_len = sizeof(client_addr);

                //Answer the phone
                int client_fd = accept(server_fd , (struct sockaddr*)&client_addr , &client_len);
                if(client_fd<0){
                    continue; 
                }
                cout << "New client connected via kqueue" << endl;

                //1.Make the client socket non blocking
                make_socket_non_blocking(client_fd);

                //2.Adding a new rule to kqueue : watch this client for incoming data 
                struct kevent client_event;
                EV_SET(&client_event , client_fd , EVFILT_READ , EV_ADD | EV_ENABLE , 0,0,NULL);

                if(kevent(kq , &client_event,1,NULL , 0 ,NULL)==-1){
                    cerr << "Failed to add client to the queue" << endl;
                    close(client_fd);
                }

            }

            //Scenario 2 : An existing client send us data
            else if(event_list[i].filter == EVFILT_READ){

                //kqueue guarantees there is water in the bucket
                char buffer[4096] = {0};
                ssize_t bytes_read = recv(current_fd , buffer , sizeof(buffer) - 1 ,0);

                if(bytes_read <= 0){
                    //if recv returns 0 , the client hung up
                    cout<<"Client Disconnected" << endl;
                    close(current_fd);
                }
                else{
                    cout << "Read" << bytes_read << "here are the bytes from the client :\n " << endl;
                    cout << buffer << endl;

                    string fake_response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello Client!";
                    send(current_fd, fake_response.c_str(), fake_response.size(), 0);
                    close(current_fd);
                }
            }
            
        }
    }

}