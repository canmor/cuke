#ifndef CUKE_SERVER_H_
#define CUKE_SERVER_H_

#include "session.h"
#include "step_hub.h"
#include <asio.hpp>
#include <string_view>

namespace cuke {
    class server {
    public:
        server(std::string_view address, unsigned short port, step_engine &hub = step_hub::engine()) : hub(hub) {
            run(address, port);
        }

        ~server() {
            ioc.stop();
        }

    private:
        void run(std::string_view address, unsigned short port) {
            using tcp = asio::ip::tcp;
            auto endpoint = tcp::endpoint{asio::ip::make_address(address), port};
            tcp::acceptor a(ioc, endpoint);
            tcp::iostream stream;
            a.accept(*stream.rdbuf());
            session s(stream, stream, hub);
            s.run();
        }

    private:
        asio::io_context ioc;
        step_engine &hub;
    };
}

#endif //CUKE_SERVER_H_
