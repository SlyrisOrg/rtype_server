//
// Created by doom on 11/01/18.
//

#include <gtest/gtest.h>
#include <netproto/TCPProtocolConnection.hpp>

struct Lala
{
    int lol;
    float tralala;

    static constexpr auto serializableFields() noexcept
    {
        return meta::makeMap(reflect_member(&Lala::lol),
                             reflect_member(&Lala::tralala));
    }
};

using Packets = meta::TypeList<Lala>;
using TCPConn = TCPProtocolConnection<Packets>;
using Packet = typename TCPConn::Packet;

TEST(TCPPacketReader, Basic)
{
    constexpr unsigned short port = 31344;
    asio::io_service io;
    tcp::acceptor acc(io, tcp::v4());
    bool worked = false;
    boost::shared_ptr<TCPConn> reader;
    boost::shared_ptr<TCPConn> writer;

    acc.set_option(tcp::acceptor::reuse_address(true));
    ASSERT_NO_THROW(acc.bind(tcp::endpoint(tcp::v4(), port)));
    ASSERT_NO_THROW(acc.listen(1));

    tcp::socket servSock(io);
    acc.async_accept(servSock, [&reader, &worked, &servSock](const boost::system::error_code &ec) {
        if (ec)
            return;
        reader = TCPConn::makeShared(std::move(servSock));
        reader->asyncRead([&worked, &reader](const boost::system::error_code &ec) {
            if (ec)
                return;
            while (reader->available() > 0) {
                Packet p = reader->pop();
                if (std::holds_alternative<Lala>(p)) {
                    Lala l = std::get<Lala>(p);
                    if (l.lol == 2 && l.tralala == 3.5f)
                        worked = true;
                }
            }
        });
    });

    tcp::socket client(io);

    tcp::endpoint endpoint(asio::ip::address::from_string("127.0.0.1"), port);
    client.async_connect(endpoint, [&writer, &client](const boost::system::error_code &ec) {
        if (ec)
            return;
        writer = TCPConn::makeShared(std::move(client));
        Lala l;
        l.lol = 2;
        l.tralala = 3.5f;
        writer->asyncWrite(l, [](const boost::system::error_code &, size_t) {});
    });

    io.run();
    ASSERT_TRUE(worked);
}