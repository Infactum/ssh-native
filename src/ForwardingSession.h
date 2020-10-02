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

#ifndef FORWARDINGSESSION_H
#define FORWARDINGSESSION_H

#include <memory>
#include <libssh/libssh.h>
#include <boost/asio.hpp>
#include "SshChannel.h"

namespace asio = boost::asio;
using asio::ip::tcp;

class ForwardingSession : public std::enable_shared_from_this<ForwardingSession> {
public:
    ForwardingSession(tcp::socket socket_, std::shared_ptr<SshChannel> channel_) :
            socket(std::move(socket_)),
            channel(std::move(channel_)) {};

    void start();

    void stop();

    // Read from SSH channel and pass to socket
    void doRead();

    void handleRead();

    // Read from socket and forward to SSH channel
    void doWrite();

    virtual ~ForwardingSession();

private:
    std::shared_ptr<SshChannel> channel;
    tcp::socket socket;
    static constexpr int BUFFER_SIZE = 1024;
    char read_buffer[BUFFER_SIZE] = {0};
    char write_buffer[BUFFER_SIZE] = {0};
    asio::mutable_buffer write_mb = {write_buffer, BUFFER_SIZE};
};


#endif //FORWARDINGSESSION_H
