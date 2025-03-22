#include <string>
#include <netinet/in.h>

namespace os_sock {
    struct SocketAddress {
        std::string host;
        in_port_t port;

        friend std::ostream& operator<<(std::ostream& out, const SocketAddress& obj);
        bool operator==(const SocketAddress& obj);
    };
}