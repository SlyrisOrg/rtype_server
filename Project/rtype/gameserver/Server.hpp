//
// Created by doom on 23/01/18.
//

#ifndef RTYPE_SERVER_SERVER_HPP
#define RTYPE_SERVER_SERVER_HPP

#include <vector>
#include <thread>
#include <SFML/Graphics.hpp>
#include <utils/Enums.hpp>
#include <log/Logger.hpp>
#include <protocol/Game.hpp>
#include <gameserver/IOThread.hpp>
#include <variant>

namespace rtype
{
    ENUM(Mode,
         Solo,
         Duo,
         Trio,
         Quatuor);

    ENUM(PeerID,
         All,
         One,
         Two,
         Three,
         Four);

    using GamePacket = meta::list::Convert<meta::list::PushFront<game::Packets, std::monostate>, std::variant>;
    using PeerAndPacket = std::pair<PeerID, GamePacket>;

    class ServerIOThread : public IOThread<GamePacket>
    {
    private:
        using TCPProtoConn = TCPProtocolConnection<game::Packets>;

        void _handlePacket(const boost::system::error_code &ec, boost::weak_ptr<TCPProtoConn> conn)
        {
            if (auto shared = conn.lock()) {
                if (ec) {
                    auto it = std::find(_clients.begin(), _clients.end(), shared);
                    _clients.erase(it);
                    return;
                }
                while (shared->available() > 0) {
                    auto packet = shared->pop();
                    _queue.push(std::move(packet));
                }
                _readFromClient(shared);
            } else {
                auto it = std::find(_clients.begin(), _clients.end(), shared);
                _clients.erase(it);
            }
        }

        void _readFromClient(boost::shared_ptr<TCPProtoConn> clientConn)
        {
            boost::weak_ptr<TCPProtoConn> weakPtr(clientConn);
            clientConn->asyncRead(boost::bind(&ServerIOThread::_handlePacket, this,
                                              asio::placeholders::error, weakPtr));
        }

        void _startAcceptor() noexcept
        {
            _acc.async_accept(_sock, boost::bind(&ServerIOThread::_onAccept, this, asio::placeholders::error));
        }

        void _onAccept(const boost::system::error_code &ec)
        {
            if (ec) {
                return;
            }
            _clients.push_back(TCPProtoConn::makeShared(std::move(_sock)));
            _readFromClient(_clients.back());
            _startAcceptor();
        }

    public:
        void run(unsigned short port) noexcept
        {
            IOThread::run([this, port]() {
                boost::system::error_code ec;

                _acc.open(tcp::v4(), ec);
                if (ec)
                    return;
                _acc.set_option(tcp::acceptor::reuse_address(true));
                _acc.bind(tcp::endpoint{tcp::v4(), port}, ec);
                if (ec)
                    return;
                _acc.listen(4, ec);
                if (ec)
                    return;
                _startAcceptor();
            });
        }

        template <typename Packet>
        void broadcastPacket(const Packet &packet)
        {
            for (auto &curClient : _clients) {
                boost::system::error_code ec;
                curClient->write(packet, ec);
                if (ec) {
                    //?
                }
            }
        }

    private:
        std::vector<boost::shared_ptr<TCPProtoConn>> _clients;
        tcp::acceptor _acc{_io};
        tcp::socket _sock{_io};
    };

    class GameServer
    {
    public:
        GameServer(unsigned short port, Mode mode, std::vector<std::string> authToks) noexcept :
            _port(port), _mode(mode), _authToks(std::move(authToks))
        {
        }

        void _update(const sf::Time &elapsed) noexcept
        {
            GamePacket packet;

            while (_ioThread.queue().pop(packet)) {
                auto visitor = meta::makeVisitor([this](auto &&v) {
                    _log(logging::Debug) << "Got a packet of type " << typeid(v).name() << std::endl;
                    game::MatchStarted ms;
                    _ioThread.broadcastPacket(ms);
                });

                std::visit(visitor, packet);
            }
        }

        void start() noexcept
        {
            _log(logging::Info) << "Starting game with mode " << _mode.toString() << std::endl;
            _log(logging::Info) << "Using port " << _port << std::endl;
            _ioThread.run(_port);

            sf::Clock clock;
            sf::Time timeSinceLastUpdate;
            const sf::Time TimePerFrame = sf::seconds(1.f / 60.f);
            while (!_ioThread.stopped()) {
                _update(timeSinceLastUpdate);
                timeSinceLastUpdate = clock.restart();

                if (auto rest = TimePerFrame.asMilliseconds() - clock.getElapsedTime().asMilliseconds(); rest > 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(rest));
                }
            }
        }

    private:
        //TODO: link packet and the person to which we wanna send it
        ServerIOThread _ioThread;

        unsigned short _port;
        Mode _mode;
        std::vector<std::string> _authToks;
        logging::Logger _log{"game", logging::Debug};
    };
}

#endif //RTYPE_SERVER_SERVER_HPP
