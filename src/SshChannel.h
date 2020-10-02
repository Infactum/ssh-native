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

#ifndef SSHCHANNEL_H
#define SSHCHANNEL_H

#include <memory>
#include <libssh/libssh.h>
#include "SshSession.h"

class SshChannel {
public:
    explicit SshChannel(std::shared_ptr<SshSession> session_) :
            session(std::move(session_)) {
        channel = ssh_channel_new(session->c_session());
        if (channel == nullptr) {
            throw std::runtime_error(ssh_get_error(session->c_session()));
        }
    };

    virtual ~SshChannel() {
        if (ssh_channel_is_open(channel)) {
            ssh_channel_close(channel);
        }
        ssh_channel_free(channel);
    }

    ssh_channel c_channel() {
        return channel;
    }

private:
    std::shared_ptr<SshSession> session;
    ssh_channel channel;
};

#endif //SSHCHANNEL_H
