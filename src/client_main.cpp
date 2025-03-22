#include <iostream>
#include <string>
#include <thread>
#include <socket.hpp>
#include <mutex>
#include <atomic>

std::mutex console_mutex;
std::atomic<bool> is_running(true);

void receive_messages(os_sock::Socket& client_socket) {
    try {
        while (is_running) {
            std::string response = client_socket.recv();
            if (response.empty()) {
                std::lock_guard<std::mutex> lock(console_mutex);
                std::cerr << "Server disconnected." << std::endl;
                is_running = false;
                break;
            }
            std::lock_guard<std::mutex> lock(console_mutex);
            std::cout << "Server: " << response << std::endl;
        }
    } catch (const std::exception& err) {
        std::lock_guard<std::mutex> lock(console_mutex);
        std::cerr << "receive_messages | Exception: " << err.what() << std::endl;
    } catch (...) {
        std::lock_guard<std::mutex> lock(console_mutex);
        std::cerr << "receive_messages | Unexpected error occurred." << std::endl;
    }
}

int main() {
    try {
        os_sock::Socket client_socket(AF_INET, SOCK_STREAM);
        client_socket.connect("127.0.0.1", 8080);

        {
            std::lock_guard<std::mutex> lock(console_mutex);
            std::cout << "Connected to the server!" << std::endl;
        }

        std::thread listener_thread(receive_messages, std::ref(client_socket));

        std::string message;
        while (is_running) {
            std::getline(std::cin, message);

            if (message == "/exit") {
                is_running = false;
                break;
            }

            try {
                client_socket.send(message);
            } catch (const std::exception& err) {
                std::lock_guard<std::mutex> lock(console_mutex);
                std::cerr << "main | Send exception: " << err.what() << std::endl;
                break;
            } catch (...) {
                std::lock_guard<std::mutex> lock(console_mutex);
                std::cerr << "main | Unexpected error during send." << std::endl;
                break;
            }
        }

        is_running = false;
        client_socket.close();
        if (listener_thread.joinable()) {
            listener_thread.join();
        }

    } catch (const std::exception& err) {
        std::lock_guard<std::mutex> lock(console_mutex);
        std::cerr << "main | Exception: " << err.what() << std::endl;
    } catch (...) {
        std::lock_guard<std::mutex> lock(console_mutex);
        std::cerr << "main | Unexpected error occurred." << std::endl;
    }

    return 0;
}
