//
// Created by doom on 11/01/18.
//

#ifndef RTYPE_SERVER_TCPPACKETREADER_HPP
#define RTYPE_SERVER_TCPPACKETREADER_HPP

#include <queue>
#include <functional>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/bind.hpp>
#include <protocol/Protocol.hpp>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

class TCPPacketReader
{
    static constexpr const size_t chunkSize = 2;

public:
    using Packet = proto::Packet;

    explicit TCPPacketReader(tcp::socket &&sock) noexcept : _sock(std::move(sock))
    {
    }

private:
    void _handleRead(size_t readBytes, const boost::system::error_code &ec)
    {
        if (ec) {
            _cb(ec);
        } else {
            _used += readBytes;
            if (_used > sizeof(size_t)) {
                size_t packetSize = _uf.unserializeSize(_buff);
                if (_buff.size() - sizeof(size_t) >= packetSize) {
                        auto start = _buff.begin() + sizeof(size_t);
                        _read.push(_uf.unserialize(proto::BufferSpan(start, start + packetSize)));
                        size_t consumedSize = sizeof(size_t) + packetSize;
                        _used -= consumedSize;
                        _buff.erase(_buff.begin(), _buff.begin() + consumedSize);
                        _cb({});
                        return;
                }
            }
            _doRead();
        }
    }

    void _doRead() noexcept
    {
        _buff.resize(_used + chunkSize);
        _sock.async_receive(asio::buffer(_buff.data() + _used, chunkSize),
                            boost::bind(&TCPPacketReader::_handleRead, this,
                                        asio::placeholders::bytes_transferred,
                                        asio::placeholders::error));
    }

public:
    template <typename Functor>
    void asyncRead(Functor &&f) noexcept
    {
        _cb = std::forward<Functor>(f);
        _doRead();
    }

    size_t available() const noexcept
    {
        return _read.size();
    }

    Packet pop() noexcept
    {
        Packet ret = _read.front();
        _read.pop();
        return ret;
    }

private:
    proto::Unformatter _uf;
    std::function<void(const boost::system::error_code &ec)> _cb;
    size_t _used{0};
    proto::Buffer _buff;
    std::queue<Packet> _read;
    tcp::socket _sock;
};

#endif //RTYPE_SERVER_TCPPACKETREADER_HPP
