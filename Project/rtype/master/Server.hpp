//
// Created by doom on 04/01/18.
//

#ifndef RTYPE_MASTER_SERVER_HPP
#define RTYPE_MASTER_SERVER_HPP

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/filesystem.hpp>
#include <boost/bind.hpp>
#include <log/Logger.hpp>
#include <protocol/Protocol.hpp>
#include <master/PlayerPool.hpp>

namespace asio = boost::asio;
namespace fs = boost::filesystem;
namespace sys = boost::system;
namespace lg = logging;
using tcp = asio::ip::tcp;

namespace rtype::master
{
    class Server
    {
    public:
        Server(unsigned short port, fs::path resDir) noexcept : _port(port), _resourcesDir(std::move(resDir))
        {
        }

    private:
        bool _setupAcceptor() noexcept
        {
            boost::system::error_code ec;

            _acc.open(tcp::v4(), ec);
            if (ec)
                return false;
            _acc.set_option(tcp::acceptor::reuse_address(true));
            _acc.bind(tcp::endpoint(tcp::v4(), _port), ec);
            if (ec)
                return false;
            _acc.listen(tcp::socket::max_connections, ec);
            if (ec)
                return false;
            _startAccepting();
            return true;
        }

        void _startAccepting() noexcept
        {
            _nextClient.reset(new tcp::socket(_io));
            _acc.async_accept(*_nextClient, boost::bind(&Server::_addToMatchMaker, this, asio::placeholders::error));
        }

        void _addToMatchMaker(const sys::error_code &ec)
        {
            if (!ec) {
                _log(lg::Debug) << "Accepted a connection from "
                                << _nextClient->remote_endpoint().address().to_string() << std::endl;
                _playerPool.addPlayer(std::move(*_nextClient));
                _nextClient.reset();
            } else {
                _log(lg::Warning) << "Unable to accept socket: " << ec.message() << std::endl;
            }
            _nextClient.reset();
            _startAccepting();
        }

        void _setupSignals() noexcept
        {
            _sigSet.async_wait([this]([[maybe_unused]] const sys::error_code &error, int sig) {
                this->_log(lg::Debug) << "Stopping: caught signal " << sig << std::endl;
                this->_io.stop();
            });
        }

    public:
        bool run() noexcept
        {
            _setupSignals();
            _log(lg::Info) << "Successfully signal handlers" << std::endl;
            if (!_setupAcceptor())
                return false;
            _log(lg::Info) << "Successfully setup acceptor on port " << _port << std::endl;

            boost::system::error_code ec;
            _io.run(ec);
            return !ec;
        }

    private:
        PlayerPool _playerPool;
        std::unique_ptr<tcp::socket> _nextClient{nullptr};
        asio::io_service _io;
        asio::signal_set _sigSet{_io, SIGINT, SIGTERM};
        tcp::acceptor _acc{_io};
        unsigned short _port;
        fs::path _resourcesDir;
        lg::Logger _log{"master", lg::Debug};
    };
}

#endif //RTYPE_MASTER_SERVER_HPP
