#include <iostream>
#include <utility>
#include <vector>
#include <algorithm>
#include <functional>
#include <thread>
#include <mutex>
#include <socket.hpp>

std::vector<std::pair<os_sock::Socket, os_sock::SocketAddress>> clients;
std::vector<std::thread> threads;
std::mutex client_mutex;

void remove_client(os_sock::SocketAddress& client_addr) {
    std::lock_guard<std::mutex> lock(client_mutex);
    clients.erase(
        std::remove_if(
            clients.begin(),
            clients.end(),
            [&](const std::pair<os_sock::Socket, os_sock::SocketAddress>& client) {
                auto [sock, addr] = client;
                return addr == client_addr;
            }
        ),
        clients.end()
    );
}

void broadcast(const std::string& message, os_sock::SocketAddress& sender_addr) {
    std::lock_guard<std::mutex> lock(client_mutex);
    for (auto& [client, address] : clients) {
        if (address == sender_addr)
            continue;
        client.send(message);
    }
}

void handle_client(os_sock::Socket client_sock, os_sock::SocketAddress client_addr) {
    try {
        client_sock.send("Welcome!");
        while (true) {
            std::string response = client_sock.recv();
            if (response.empty())
                continue;
            std::cout << client_addr << " = " << response << "\n";
            broadcast(response, client_addr);
        }
    } catch (std::exception& err) {
        std::cout << "handle_client | " << err.what() << std::endl;
    }

    remove_client(client_addr);
    std::cout << client_addr << " disconnected" << std::endl;
}

int main() {
    os_sock::Socket server(AF_INET, SOCK_STREAM);

    server.bind("127.0.0.1", 8080);
    server.listen(5);

    try {
        while (true) {
            auto [client_socket, client_address] = server.accept();
    
            std::cout << client_address << " connected\n";

            {
                std::lock_guard<std::mutex> lock(client_mutex);
                clients.push_back({client_socket, client_address});
            }
    
            threads.emplace_back(
                std::thread(handle_client, client_socket, client_address)
            );
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