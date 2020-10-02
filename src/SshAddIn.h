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

#ifndef SSHADDIN_H
#define SSHADDIN_H

#include <libssh/libssh.h>
#include <libssh/callbacks.h>
#include <boost/asio.hpp>
#include "Component.h"
#include "SshSession.h"
#include "SshChannel.h"
#include "ForwardingServer.h"

namespace asio = boost::asio;

class SshAddIn final : public Component {
public:
    SshAddIn();

    ~SshAddIn() override;

private:
    /* Exposed methods */
    variant_t Connect();

    variant_t Exec(const variant_t &command, variant_t &res_stdout, variant_t &res_stderr);

    variant_t Read(const variant_t &path, variant_t &dst);

    variant_t Write(const variant_t &path, const variant_t &src, const variant_t &mode);

    variant_t ErrorDesc();

    variant_t EnablePortForwarding(const variant_t &dest_, const variant_t &dest_port_, variant_t &port_);

    void DisablePortForwarding();

    void EnableLog(const variant_t &level);

    variant_t GetLog();

    void ClearLog();

    /* Exposed properties */
    std::string password;

    /* Service API */
    std::string extensionName() override;

    void addMethods();

    void addProperties();

    variant_t authenticate();

    void setOption(ssh_options_e type, const variant_t &value);

    std::shared_ptr<variant_t> getOption(ssh_options_e type);

    std::shared_ptr<variant_t> getPort();

    static bool readChannel(std::vector<std::string> &chunks, ssh_channel channel, bool is_stderr);

    bool receive(ssh_scp scp, variant_t &dst);

    bool send(ssh_scp scp, const std::string &filename, const std::vector<char> &buffer, const variant_t &mode);

    static constexpr char EXT_NAME[] = u8"SSH";
    static constexpr char NOT_CONNECTED[] = u8"not connected";
    static constexpr char ALREADY_CONNECTED[] = u8"already connected";
    static constexpr char TYPE_ERR[] = u8"Неверный тип значения входного параметра";

    std::shared_ptr<SshSession> session;
    std::vector<std::string> log;
    std::string error_desc;

    std::thread port_forwarding_thread;
    asio::io_context io_context;
    std::vector<std::shared_ptr<ForwardingServer>> forwarding_servers;
};

#endif //SSHADDIN_H
