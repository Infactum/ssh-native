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

#ifndef SSHSESSION_H
#define SSHSESSION_H

#include <memory>
#include <libssh/libssh.h>

// Not a full-featured libssh wrapper, but a Boost.Asio helper
class SshSession {
public:
    static std::shared_ptr<SshSession> Create() {
        struct enable_shared_ptr : public SshSession {
        };
        return std::make_shared<enable_shared_ptr>();
    }

    ssh_session c_session() {
        return session;
    }

    virtual ~SshSession() {
        // Disconnect will invalidate all ssh channels
        // Use with care and don't bind to 1C API
        if (ssh_is_connected(session)) {
            ssh_disconnect(session);
        }
        ssh_free(session);
    }

private:
    // Prey that we don't get memory error here
    SshSession() : session(ssh_new()) {};

    ssh_session session;
};

#endif //SSHSESSION_H
