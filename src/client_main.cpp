#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <socket.hpp>

std::atomic<bool> error_status(false);

void receive_messages(os_sock::Socket client_socket) {
    try {
        while (true) {
            std::string response = client_socket.recv();
            if (response.empty())
                continue;
            std::cout << ">> " << response << "\n";
        }
    } catch (std::exception& err) {
        std::cout << "receive_messages | " << err.what() << std::endl;
        error_status = true;
    }
}

int main() {
	os_sock::Socket client(AF_INET, SOCK_STREAM);
    client.connect("127.0.0.1", 8080);

    // std::thread read_thread(std::bind(receive_messages, client));
    // read_thread.detach();

    std::string buffer;
    try {
        while (!error_status) {
            std::cout << "<< ";
            std::getline(std::cin, buffer);
            if (buffer.size() > BUFSIZ)
                buffer = buffer.substr(BUFSIZ);
    
            if (buffer == ":quit")
                break;

            std::string response = client.recv();
            if (response.empty())
                continue;
            std::cout << ">> " << response << "\n";
            
            client.send(buffer);
        }
    } catch (std::exception& err) {
        std::cout << "main | " << err.what() << std::endl;
        client.close();
        exit(EXIT_FAILURE);
    } catch (...) {
        std::cout << "main | unexpected error" << std::endl;
        client.close();
        exit(EXIT_FAILURE);
    }

	return 0;
}
