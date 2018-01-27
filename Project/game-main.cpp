//
// Created by doom on 22/01/18.
//

#include <iostream>
#include <boost/program_options.hpp>
#include <gameserver/Server.hpp>

namespace opt = boost::program_options;

int main(int ac, char **av)
{
    opt::options_description desc("");

    desc.add_options()
        ("player", opt::value<std::vector<std::string>>()->multitoken(), "The players' tokens")
        ("mode", opt::value<unsigned int>(), "The game mode (ranging from 1 to 4)")
        ("port", opt::value<unsigned short>(), "")
        ("help", "Display this help message");

    opt::variables_map vm;

    try {
        opt::store(opt::parse_command_line(ac, (const char *const *)av, desc), vm);
        opt::notify(vm);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    if (!vm["help"].empty()) {
        std::cout << desc << std::endl;
        return 0;
    }

    if (vm["port"].empty() || vm["player"].empty() || vm["mode"].empty() || vm["mode"].as<unsigned int>() >= 4) {
        std::cerr << desc << std::endl;
        return 1;
    }

    if (vm["mode"].as<unsigned int>() + 1 != vm["player"].as<std::vector<std::string>>().size()) {
        std::cerr << "Invalid number of players for the given mode" << std::endl;
        return 1;
    }

    rtype::GameServer gs(vm["port"].as<unsigned short>(),
                         static_cast<rtype::Mode::EnumType>(vm["mode"].as<unsigned int>()),
                         vm["player"].as<std::vector<std::string>>());

    gs.start();
    return 0;
}
