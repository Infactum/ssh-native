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

#include "ForwardingSession.h"

void ForwardingSession::start() {
    doRead();
    doWrite();
}

void ForwardingSession::stop() {
    if (socket.is_open()) {
        socket.close();
    }
}

void ForwardingSession::doRead() {
    // Keep this object alive
    auto self = shared_from_this();
    asio::post(socket.get_executor(), [this, self]() { handleRead(); });
}

void ForwardingSession::handleRead() {
    auto c_channel = channel->c_channel();

    if (ssh_channel_is_eof(c_channel)) {
        return;
    }

    if (!socket.is_open()) {
        return;
    }

    int len = ssh_channel_read_nonblocking(c_channel, read_buffer, BUFFER_SIZE, false);

    if (len == SSH_ERROR) {
        return;
    }

    auto self = shared_from_this();
    asio::async_write(socket, asio::buffer(read_buffer, len),
                      [this, self, len](const boost::system::error_code &error, std::size_t size) {
                          if (!error && size == len) {
                              doRead();
                          }
                      });
}

void ForwardingSession::doWrite() {
    auto self = shared_from_this();

    if (!socket.is_open()) {
        return;
    }

    socket.async_read_some(write_mb, [this, self](const boost::system::error_code &error, std::size_t size) {
        if (error) {
            return;
        }
        auto c_channel = channel->c_channel();

        if (!ssh_channel_is_open(c_channel)) {
            return;
        }

        auto rc = ssh_channel_write(c_channel, write_buffer, size);

        if (rc == SSH_ERROR || rc != size) {
            return;
        }

        doWrite();
    });
}

ForwardingSession::~ForwardingSession() {

}
