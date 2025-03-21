#include <iostream>
#include "../include/socket.hpp"

using namespace std;

int main() {
	os_sock::Socket client(AF_INET, SOCK_STREAM);

	client.connect("127.0.0.1", 8080);

	cout << client.send("Hello Server!") << endl;

	return 0;
}
