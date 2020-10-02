/*
 *  SSH Native
 *  Copyright (C) 2018  Infactum
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef FORWARDINGSERVER_H
#define FORWARDINGSERVER_H

#include <memory>
#include <libssh/libssh.h>
#include <boost/asio.hpp>
#include "ForwardingSession.h"

namespace asio = boost::asio;
using asio::ip::tcp;

class ForwardingServer {
public:
    ForwardingServer(asio::io_context &io_context,
                     std::shared_ptr<SshSession> session_,
                     std::string dest_, unsigned short dest_port_,
                     unsigned short port);

    unsigned short GetPort();

private:
    void doAccept();

    std::shared_ptr<ForwardingSession> createSession(tcp::socket socket);

    std::shared_ptr<SshSession> session;
    tcp::acceptor acceptor;
    const std::string dest;
    const unsigned short dest_port;
};


#endif //FORWARDINGSERVER_H
