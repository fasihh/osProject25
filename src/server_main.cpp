#include <iostream>
#include "../include/socket.hpp"

using namespace std;

int main() {
	sockets::Socket server(AF_INET, SOCK_STREAM);

	server.bind("127.0.0.1", 8080);
	server.listen(5);

	server.accept();
	string response = server.recv();
	cout << response << endl;
	cout << server.send("Hello Client!") << endl;

	return 0;
}
