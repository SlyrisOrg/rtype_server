//
// Created by doom on 11/01/18.
//

#include <gtest/gtest.h>
#include <netproto/TCPPacketReader.hpp>

TEST(TCPPacketReader, Basic)
{
    constexpr unsigned short port = 31344;
    asio::io_service io;
    tcp::acceptor acc(io, tcp::v4());
    bool worked = false;

    ASSERT_NO_THROW(acc.bind(tcp::endpoint(tcp::v4(), port)));
    ASSERT_NO_THROW(acc.listen(1));

    tcp::socket servSock(io);
    acc.async_accept(servSock, [&worked, &servSock](const boost::system::error_code &ec) {
        if (ec)
            return;
        std::shared_ptr<TCPPacketReader> _reader = std::make_shared<TCPPacketReader>(std::move(servSock));
        _reader->asyncRead([&worked, _reader](const boost::system::error_code &ec) {
            if (ec)
                return;
            while (_reader->available() > 0) {
                proto::Packet p = _reader->pop();
                if (std::holds_alternative<proto::Lala>(p)) {
                    proto::Lala l = std::get<proto::Lala>(p);
                    if (l.lol == 2 && l.tralala == 3.5f)
                        worked = true;
                }
            }
        });
    });

    tcp::socket client(io);

    tcp::endpoint endpoint(asio::ip::address::from_string("127.0.0.1"), port);
    client.async_connect(endpoint, [&client](const boost::system::error_code &ec) {
        if (ec)
            return;
        proto::Formatter f;
        proto::Lala l;
        l.lol = 2;
        l.tralala = 3.5f;
        f.serialize(l);
        f.prefixSize();
        auto buff = f.extract();
        size_t halfSize = buff.size() / 2;
        client.write_some(asio::buffer(buff.data(), halfSize));
        client.write_some(asio::buffer(buff.data() + halfSize, buff.size() - halfSize));
        client.close();
    });

    io.run();
    ASSERT_TRUE(worked);
}