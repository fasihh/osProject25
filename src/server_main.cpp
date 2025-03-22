#include <iostream>
#include <utility>
#include <vector>
#include <algorithm>
#include <functional>
#include <thread>
#include <mutex>
#include <socket.hpp>

std::vector<std::pair<os_sock::Socket, os_sock::SocketAddress>> clients;
std::mutex client_mutex;

void remove_client(os_sock::Socket& client_socket) {
    std::lock_guard<std::mutex> lock(client_mutex);
    std::remove_if(
        clients.begin(),
        clients.end(),
        [&](const std::pair<os_sock::Socket, os_sock::SocketAddress>& client) {
            return client.first == client_socket;
        }
    );
    client_socket.close();
}

void broadcast(const std::string& message, os_sock::Socket& sender) {
    for (auto& [client, address] : clients) {
        if (client == sender)
            continue;
        client.send(message);
    }
}

void handle_client(os_sock::Socket client_socket, os_sock::SocketAddress client_address) {
    try {
        client_socket.send("Welcome!");
        while (true) {
            std::string response = client_socket.recv();
            if (response.empty())
                continue;
            std::cout << client_address << " = " << response << "\n";
            broadcast(response, client_socket);
        }
    } catch (std::exception& err) {
        std::cout << "handle_client | " << err.what() << std::endl;
    }

    remove_client(client_socket);
    std::cout << client_address << " disconnected" << std::endl;
}

int main() {
    os_sock::Socket server(AF_INET, SOCK_STREAM);

    server.bind("127.0.0.1", 8080);
    server.listen(5);

    try {
        while (true) {
            auto [client_socket, client_address] = server.accept();
    
            std::cout << client_address << " connected\n";
    
            std::lock_guard<std::mutex> lock(client_mutex);
            clients.push_back({client_socket, client_address});
    
            std::thread client_thread(std::bind(handle_client, client_socket, client_address));
            client_thread.detach();
        }
    } catch (std::exception& err) {
        std::cout << "main | " << err.what() << std::endl;
        server.close();
        exit(EXIT_FAILURE);
    } catch (...) {
        std::cout << "main | unexpected error" << std::endl;
        server.close();
        exit(EXIT_FAILURE);
    }

    return 0;
}