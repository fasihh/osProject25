#include "../include/socket.hpp"
#include <iostream>

using namespace std;

int main() {
    sockets::Socket server(AF_INET, SOCK_STREAM);

    server.bind("127.0.0.1", 8080);
    server.listen(5);

    while (true) {
        auto [client_socket, client_address] = server.accept();
        string response = client_socket.recv();
        cout << client_address.first << ":" << client_address.second << " = " << response << endl;
    }

    return 0;
}