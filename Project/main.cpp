#include <iostream>
#include <boost/program_options.hpp>

namespace opt = boost::program_options;

int main(int ac, char **av)
{
    opt::options_description desc("Available options");

    desc.add_options()
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
    return 0;
}