//
// Created by doom on 27/01/18.
//

#ifndef RTYPE_SERVER_SERVERIOTHREAD_HPP
#define RTYPE_SERVER_SERVERIOTHREAD_HPP

#include <variant>
#include <log/Logger.hpp>
#include <netproto/TCPProtocolConnection.hpp>
#include <protocol/Game.hpp>
#include <gameserver/IOThread.hpp>

namespace rtype
{
    using GamePacket = meta::list::Convert<meta::list::PushFront<game::Packets, std::monostate>, std::variant>;

    struct PeerAndPacket
    {
        size_t id;
        GamePacket packet;
    };

    class ServerIOThread : public IOThread<PeerAndPacket>
    {
    private:
        using TCPProtoConn = TCPProtocolConnection<game::Packets>;

        void _handlePacket(const boost::system::error_code &ec, boost::weak_ptr<TCPProtoConn> conn)
        {
            if (auto shared = conn.lock()) {
                auto it = std::find(_clients.begin(), _clients.end(), shared);

                if (ec) {
                    _clients.erase(it);
                    return;
                }
                auto id = static_cast<size_t>(it - _clients.begin());

                while (shared->available() > 0) {
                    PeerAndPacket peerAndPacket;
                    peerAndPacket.id = id;
                    peerAndPacket.packet = shared->pop();
                    _queue.push(std::move(peerAndPacket));
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
                _log(logging::Debug) << ec.message() << std::endl;
                return;
            }
            auto shared = TCPProtoConn::makeShared(std::move(_sock));
            boost::system::error_code ec2;
            shared->write(game::Welcome(), ec2);
            if (!ec2) {
                _clients.push_back(std::move(shared));
                _readFromClient(_clients.back());
            }
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

        template <typename Packet>
        void sendPacket(size_t id, const Packet &packet)
        {
            boost::system::error_code ec;
            _clients[id]->write(packet, ec);
            if (ec) {
                //?
            }
        }

    private:
        logging::Logger _log{"IO", logging::Debug};
        //FIXME: Use a better container with associative IDs
        std::vector<boost::shared_ptr<TCPProtoConn>> _clients;
        tcp::acceptor _acc{_io};
        tcp::socket _sock{_io};
    };
}

#endif //RTYPE_SERVER_SERVERIOTHREAD_HPP
