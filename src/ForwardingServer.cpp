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

#include <utility>
#include "ForwardingServer.h"

ForwardingServer::ForwardingServer(asio::io_context &io_context,
                                   std::shared_ptr<SshSession> session_,
                                   std::string dest_, unsigned short dest_port_,
                                   unsigned short port) :
        acceptor(io_context, tcp::endpoint(asio::ip::address::from_string("0.0.0.0"), port)),
        session(std::move(session_)),
        dest(std::move(dest_)),
        dest_port(dest_port_) {
    doAccept();
}

void ForwardingServer::doAccept() {
    acceptor.async_accept([this](boost::system::error_code error, tcp::socket socket) {
        if (!error) {
            std::shared_ptr<ForwardingSession> s;
            try {
                s = createSession(std::move(socket));
            } catch (const std::runtime_error &) {
                // Failed to create channel or start forwarding
                // Just ignore since I don't know where to send this error.
            }
            s->start();
        }
        doAccept();
    });
}

unsigned short ForwardingServer::GetPort() {
    return acceptor.local_endpoint().port();
}

std::shared_ptr<ForwardingSession> ForwardingServer::createSession(tcp::socket socket) {

    auto forwarding_channel = std::make_shared<SshChannel>(session);
    auto rc = ssh_channel_open_forward(forwarding_channel->c_channel(),
                                       dest.data(), dest_port,
                                       "localhost", GetPort());
    if (rc != SSH_OK) {
        throw std::runtime_error(ssh_get_error(session->c_session()));
    }

    return std::make_shared<ForwardingSession>(std::move(socket), std::move(forwarding_channel));
}
