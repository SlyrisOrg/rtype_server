//
// Created by doom on 04/01/18.
//

#include <iostream>
#include <boost/program_options.hpp>
#include <master/Server.hpp>

namespace opt = boost::program_options;

static int launchMaster(const opt::variables_map &vm)
{
    rtype::master::Server serv(vm["port"].as<uint16_t>(), vm["resources"].as<std::string>());

    return !serv.run();
}

int main(int ac, char **av)
{
    opt::options_description desc("Available options");

    desc.add_options()
        ("port", opt::value<uint16_t>()->default_value(31337), "The port on which to listen")
        ("resources", opt::value<std::string>()->default_value("resources"), "The resources directory")
        ("help", "display this help message");

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

    return launchMaster(vm);
}
