#include "tenuki.h"
#include <boost/asio.hpp>

using namespace tenuki;
using std::string;
using std::vector;
using std::map;
using boost::asio::ip::tcp;

namespace {
    boost::asio::io_service io_service;
    std::ofstream logfile;

    void write_line(tcp::socket& socket, const std::string& s) {
        std::cout << ">" << s << "\n";
        boost::asio::write(socket, boost::asio::buffer(s + "\n"));
    }

    const std::string read_line(tcp::socket& socket) {
        static boost::asio::streambuf b;
        boost::asio::read_until(socket, b, "\n");
        std::istream is(&b);
        std::string line;
        std::getline(is, line);
        std::cerr << line << "\n";
        logfile << line << std::endl;
        return line;
    }

    std::smatch read_line_until(tcp::socket& socket, std::regex re) {
        std::smatch m;
        for (std::string s = read_line(socket); !std::regex_search(s, m, re); s = read_line(socket));
        return m;
    }
}

int main(int argc, char* argv[]) {

    if (argc < 5) {
        std::cerr << "Usage: tenuki host port username password\n";
        return 1;
    }

    const string HOST = argv[1];
    const string PORT = argv[2];
    const string USERNAME = argv[3];
    const string PASSWORD = argv[4];

    logfile.open("tenuki.log");

    std::cout << "Connecting to " << HOST << " port " << PORT << ".\n";
    tcp::resolver resolver(io_service);
    tcp::socket socket(io_service);
    boost::asio::connect(socket, resolver.resolve({HOST, PORT}));

    write_line(socket, "LOGIN " + USERNAME + " " + PASSWORD);
    const side_t MYSIDE = read_line_until(socket, std::regex("Your_Turn:(\\+|-)"))[1].str() == "+" ? side::BLACK : side::WHITE;
    read_line_until(socket, std::regex("END Game_Summary"));

    write_line(socket, "AGREE");
    read_line_until(socket, std::regex("START"));

    position p = parse_position("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1");
    std::cerr << to_string(p) << "\n";

    for (;;) {

        if (p.side_to_move == MYSIDE) {
            write_line(socket, to_string(ponder(p), p));
        }

        move_t m;
        for (bool retry = true; retry; ) {
            try {
                string line = read_line(socket);
                if (line == "#LOSE" || line == "#WIN" || line == "#DRAW" || line == "#CENSORED") {
                    return 0;
                }
                m = parse_move(line, p);
                retry = false;
            } catch (...) {
                retry = true;
            }
        }
        std::cerr << to_string(m, p) << "\n";
        p = do_move(p, m);
        std::cerr << to_string(p) << "\n";

    }

    return 0;
}
