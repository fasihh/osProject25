#include <iostream>
#include <socket-address.hpp>

namespace os_sock {
    std::ostream& operator<<(std::ostream& out, const SocketAddress& obj) {
        out << obj.host << ":" << obj.port;
        return out;
    }

    bool SocketAddress::operator==(const SocketAddress& obj) {
        return obj.host == this->host && obj.port == this->port;
    }
}