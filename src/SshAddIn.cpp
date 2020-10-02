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

#include <cstdlib>
#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem.hpp>
#include "SshAddIn.h"

std::string SshAddIn::extensionName() {
    return EXT_NAME;
}

SshAddIn::SshAddIn() {
    addMethods();
    addProperties();

    ssh_set_log_level(SSH_LOG_NOLOG);
    session = SshSession::Create();
}

SshAddIn::~SshAddIn() {
    DisablePortForwarding();

    ssh_set_log_callback([](int, const char *, const char *, void *) {});
    ssh_set_log_userdata(nullptr);
}

variant_t SshAddIn::Connect() {
    error_desc.clear();

    auto c_session = session->c_session();

    if (ssh_is_connected(c_session)) {
        error_desc = ALREADY_CONNECTED;
        return false;
    }

    if (ssh_connect(c_session) != SSH_OK) {
        error_desc = ssh_get_error(c_session);
        return false;
    }

    return authenticate();
}

void SshAddIn::addMethods() {
    AddMethod(L"Connect", L"Подключиться", this, &SshAddIn::Connect);
    AddMethod(L"Exec", L"Выполнить", this, &SshAddIn::Exec, {{1, ""},
                                                             {2, ""}});
    AddMethod(L"Read", L"Прочитать", this, &SshAddIn::Read);
    AddMethod(L"Write", L"Записать", this, &SshAddIn::Write, {{2, "0644"}});

    AddMethod(L"EnableLog", L"ВключитьЖурнал", this, &SshAddIn::EnableLog, {{0, SSH_LOG_DEBUG}});
    AddMethod(L"GetLog", L"ПолучитьЖурнал", this, &SshAddIn::GetLog);
    AddMethod(L"ClearLog", L"ОчиститьЖурнал", this, &SshAddIn::ClearLog);

    AddMethod(L"EnablePortForwarding", L"ВключитьПеренаправлениеПорта", this, &SshAddIn::EnablePortForwarding);
    AddMethod(L"DisablePortForwarding", L"ОтключитьПеренаправлениеПортов", this, &SshAddIn::DisablePortForwarding);

    AddMethod(L"EnableLog", L"ВключитьЖурнал", this, &SshAddIn::EnableLog, {{0, SSH_LOG_DEBUG}});
    AddMethod(L"GetLog", L"ПолучитьЖурнал", this, &SshAddIn::GetLog);
    AddMethod(L"ClearLog", L"ОчиститьЖурнал", this, &SshAddIn::ClearLog);

    AddMethod(L"ErrorDesc", L"ОписаниеОшибки", this, &SshAddIn::ErrorDesc);
}

void SshAddIn::addProperties() {
    using namespace std::placeholders;
    AddProperty(L"Host", L"АдресСервера",
                std::bind(&SshAddIn::getOption, this, SSH_OPTIONS_HOST),
                std::bind(&SshAddIn::setOption, this, SSH_OPTIONS_HOST, _1));
    AddProperty(L"User", L"Пользователь",
                std::bind(&SshAddIn::getOption, this, SSH_OPTIONS_USER),
                std::bind(&SshAddIn::setOption, this, SSH_OPTIONS_USER, _1));
    AddProperty(L"Port", L"Порт",
                std::bind(&SshAddIn::getPort, this),
                std::bind(&SshAddIn::setOption, this, SSH_OPTIONS_PORT, _1));

    AddProperty(L"Password", L"Пароль", nullptr, [&](variant_t value) {
        std::visit(overloaded{
                [&](const std::string &v) { password = v; },
                [&](const int32_t &v) { password = std::to_string(v); },
                [&](const auto &) {
                    error_desc = TYPE_ERR;
                    throw std::runtime_error(TYPE_ERR);
                }
        }, value);
    });
}

void SshAddIn::setOption(ssh_options_e type, const variant_t &value) {
    error_desc.clear();

    int result{SSH_ERROR};
    std::visit(overloaded{
            [&](const std::string &v) { result = ssh_options_set(session->c_session(), type, v.c_str()); },
            [&](const int32_t &v) { result = ssh_options_set(session->c_session(), type, &v); },
            [&](const auto &) {}
    }, value);

    if (result != SSH_OK) {
        error_desc = ssh_get_error(session->c_session());
        AddError(ADDIN_E_FAIL, EXT_NAME, error_desc, true);
    }
}

std::shared_ptr<variant_t> SshAddIn::getOption(ssh_options_e type) {
    error_desc.clear();
    char *pValue;

    if (ssh_options_get(session->c_session(), type, &pValue) == SSH_OK) {
        std::string value{pValue};
        ssh_string_free_char(pValue);
        return std::make_shared<variant_t>(value);
    } else {
        error_desc = ssh_get_error(session->c_session());
        AddError(ADDIN_E_FAIL, EXT_NAME, error_desc, true);
        ssh_string_free_char(pValue);
        return std::make_shared<variant_t>(UNDEFINED);
    }
}

std::shared_ptr<variant_t> SshAddIn::getPort() {
    error_desc.clear();

    unsigned int port;
    if (ssh_options_get_port(session->c_session(), &port) == SSH_OK) {
        return std::make_shared<variant_t>(static_cast<int32_t>(port));
    } else {
        error_desc = ssh_get_error(session->c_session());
        AddError(ADDIN_E_FAIL, EXT_NAME, error_desc, true);
        return std::make_shared<variant_t>(UNDEFINED);
    }
}

variant_t SshAddIn::authenticate() {
    error_desc.clear();

    auto c_session = session->c_session();
    if (!ssh_is_connected(c_session)) {
        error_desc = NOT_CONNECTED;
        return false;
    }

    int rc;

    if (password.empty()) {
        rc = ssh_userauth_publickey_auto(c_session, nullptr, nullptr);
    } else {
        rc = ssh_userauth_password(c_session, nullptr, password.c_str());
    }

    if (rc != SSH_AUTH_SUCCESS) {
        error_desc = ssh_get_error(c_session);
        return false;
    }

    return true;
}

variant_t SshAddIn::Exec(const variant_t &command, variant_t &res_stdout, variant_t &res_stderr) {

    error_desc.clear();
    if (!(std::holds_alternative<std::string>(command))) {
        error_desc = TYPE_ERR;
        throw std::runtime_error(TYPE_ERR);
    }

    std::string cmd = std::get<std::string>(command);
    auto c_session = session->c_session();

    if (!ssh_is_connected(c_session)) {
        error_desc = NOT_CONNECTED;
        return false;
    }

    std::shared_ptr<SshChannel> channel;
    try {
        channel = std::make_shared<SshChannel>(session);
    } catch (const std::runtime_error &error) {
        error_desc = error.what();
        return false;
    }
    ssh_channel c_channel = channel->c_channel();

    if (ssh_channel_open_session(c_channel) != SSH_OK) {
        error_desc = ssh_get_error(c_session);
        return false;
    }

    if (ssh_channel_request_exec(c_channel, cmd.c_str()) != SSH_OK) {
        error_desc = ssh_get_error(c_session);
        return false;
    }

    std::vector<std::string> stdout_chunks;
    std::vector<std::string> stderr_chunks;

    while (!ssh_channel_is_eof(c_channel)) {
        bool success = readChannel(stdout_chunks, c_channel, false) &&
                       readChannel(stderr_chunks, c_channel, true);

        if (!success) {
            error_desc = ssh_get_error(c_session);
            return false;
        }
    }

    res_stdout = boost::algorithm::join(stdout_chunks, "");
    res_stderr = boost::algorithm::join(stderr_chunks, "");

    ssh_channel_send_eof(c_channel);

    return true;
}

bool SshAddIn::readChannel(std::vector<std::string> &chunks, ssh_channel channel, bool is_stderr) {

    char buffer[256];

    for (;;) {
        int num_bytes = ssh_channel_read(channel, buffer, sizeof(buffer), is_stderr);
        if (num_bytes > 0) {
            chunks.emplace_back(buffer, num_bytes);
        } else if (num_bytes == 0) {
            break;
        } else {
            return false;
        }
    }

    return true;
}

void SshAddIn::EnableLog(const variant_t &level) {
    error_desc.clear();

    if (!(std::holds_alternative<int32_t>(level))) {
        error_desc = TYPE_ERR;
        throw std::runtime_error(TYPE_ERR);
    }

    ssh_set_log_level(std::get<int32_t>(level));
    ssh_set_log_userdata(this);
    ssh_set_log_callback([](int priority, const char *function, const char *buffer, void *userdata) {
        reinterpret_cast<SshAddIn *>(userdata)->log.emplace_back(std::string(function) + ": " + std::string(buffer));
    });
}

variant_t SshAddIn::GetLog() {
    error_desc.clear();
    std::string result = boost::algorithm::join(log, "\n");
    return result;
}

void SshAddIn::ClearLog() {
    error_desc.clear();
    log = std::vector<std::string>();
}

variant_t SshAddIn::Read(const variant_t &path, variant_t &dst) {

    error_desc.clear();
    if (!(std::holds_alternative<std::string>(path))) {
        error_desc = TYPE_ERR;
        throw std::runtime_error(TYPE_ERR);
    }

    boost::filesystem::path p{std::get<std::string>(path)};
    ssh_scp scp;
    auto c_session = session->c_session();

    if (!ssh_is_connected(c_session)) {
        error_desc = NOT_CONNECTED;
        return false;
    }

    scp = ssh_scp_new(c_session, SSH_SCP_READ, p.generic_string().c_str());
    if (scp == nullptr) {
        error_desc = ssh_get_error(c_session);
        return false;
    }

    if (ssh_scp_init(scp) != SSH_OK) {
        error_desc = ssh_get_error(c_session);
        ssh_scp_free(scp);
        return false;
    }

    auto result = receive(scp, dst);

    ssh_scp_close(scp);
    ssh_scp_free(scp);
    return result;
}

bool SshAddIn::receive(ssh_scp scp, variant_t &dst) {

    if (ssh_scp_pull_request(scp) != SSH_SCP_REQUEST_NEWFILE) {
        error_desc = ssh_get_error(session->c_session());
        return false;
    }

    int size = ssh_scp_request_get_size(scp);
    std::vector<char> buffer(size);

    ssh_scp_accept_request(scp);
    if (ssh_scp_read(scp, buffer.data(), size) == SSH_ERROR) {
        error_desc = ssh_get_error(session->c_session());
        return false;
    }

    if (ssh_scp_pull_request(scp) != SSH_SCP_REQUEST_EOF) {
        error_desc = ssh_get_error(session->c_session());
        return false;
    }

    dst = std::move(buffer);
    return true;
}

variant_t SshAddIn::Write(const variant_t &path, const variant_t &src, const variant_t &mode) {

    error_desc.clear();

    if (!(std::holds_alternative<std::string>(path))
        || !(std::holds_alternative<std::vector<char>>(src))) {
        error_desc = TYPE_ERR;
        throw std::runtime_error(TYPE_ERR);
    }

    ssh_scp scp;
    auto c_session = session->c_session();
    boost::filesystem::path p{std::get<std::string>(path)};
    std::vector<char> buffer = std::get<std::vector<char>>(src);

    if (!ssh_is_connected(c_session)) {
        error_desc = NOT_CONNECTED;
        return false;
    }

    if (!p.has_filename()) {
        error_desc = "path doesn't contain filename";
        return false;
    }

    scp = ssh_scp_new(c_session, SSH_SCP_WRITE, p.generic_string().c_str());
    if (scp == nullptr) {
        error_desc = ssh_get_error(c_session);
        return false;
    }

    if (ssh_scp_init(scp) != SSH_OK) {
        error_desc = ssh_get_error(c_session);
        ssh_scp_free(scp);
        return false;
    }

    auto result = send(scp, p.filename().generic_string(), buffer, mode);

    ssh_scp_close(scp);
    ssh_scp_free(scp);
    return true;
}

bool SshAddIn::send(ssh_scp scp, const std::string &filename, const std::vector<char> &buffer, const variant_t &mode) {

    int raw_mode;

    std::visit(overloaded{
            [&](const std::string &v) {
                // см. ssh_scp_integer_mode
                raw_mode = std::strtoul(v.c_str(), nullptr, 8) & 0xffff;
            },
            [&](const int32_t &v) { raw_mode = v; },
            [&](const auto &) {
                error_desc = TYPE_ERR;
                throw std::runtime_error(TYPE_ERR);
            }
    }, mode);

    if (ssh_scp_push_file(scp, filename.c_str(), buffer.size(), raw_mode) != SSH_OK) {
        error_desc = ssh_get_error(session->c_session());
        return false;
    }

    if (ssh_scp_write(scp, buffer.data(), buffer.size()) != SSH_OK) {
        error_desc = ssh_get_error(session->c_session());
        return false;
    }

    return true;
}

variant_t SshAddIn::ErrorDesc() {
    if (error_desc.empty()) {
        return UNDEFINED;
    }
    return error_desc;
}

variant_t SshAddIn::EnablePortForwarding(const variant_t &dest_, const variant_t &dest_port_, variant_t &port_) {
    error_desc.clear();

    if (!std::holds_alternative<std::string>(dest_)
        || !std::holds_alternative<int32_t>(dest_port_)) {
        error_desc = TYPE_ERR;
        throw std::runtime_error(TYPE_ERR);
    }

    std::string dest = std::get<std::string>(dest_);
    unsigned short dest_port = std::get<int32_t>(dest_port_);

    unsigned short port = 0;
    if (std::holds_alternative<int32_t>(port_)) {
        port = std::get<int32_t>(port_);
    }

    if (!ssh_is_connected(session->c_session())) {
        error_desc = NOT_CONNECTED;
        return false;
    }

    auto server = std::make_shared<ForwardingServer>(io_context, session, dest, dest_port, port);
    port_ = server->GetPort();
    forwarding_servers.emplace_back(std::move(server));

    if (forwarding_servers.size() == 1) {
        port_forwarding_thread = std::thread([&io = io_context]() { io.run(); });
    }

    return true;
}

void SshAddIn::DisablePortForwarding() {
    error_desc.clear();

    io_context.stop();
    if (port_forwarding_thread.joinable()) {
        port_forwarding_thread.join();
    }

    forwarding_servers.clear();
}