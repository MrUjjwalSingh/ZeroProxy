#include "TcpServer.h"

int main(){
    //Creating a server object and binding it localhost port 8080
    TcpServer proxy("127.0.0.1" , 8080);

    //starting the server object to kepp listening to new request
    proxy.start();

    return 0;
}