#include <iostream>
#include "include/socket.hpp"

using namespace std;

int main() {
	sockets::Socket ss(AF_INET, SOCK_STREAM);

	ss.bind("localhost", 8080);


	return 0;
}
