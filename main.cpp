#include "tenuki.h"
#include <boost/asio.hpp>

using namespace tenuki;
using std::string;
using std::vector;
using std::map;
using boost::asio::ip::address;
using boost::asio::ip::tcp;


namespace {

  void write_line(tcp::socket& socket, const std::string& s) {
    std::cout << "> " << s << "\n";
    boost::asio::write(socket, boost::asio::buffer(s + "\n"));
  }

  boost::asio::streambuf sb;
  const std::string read_line(tcp::socket& socket) {
    boost::asio::read_until(socket, sb, "\n");
    std::istream is(&sb);
    std::string s;
    std::getline(is, s);
    s.erase(s.find_last_not_of("\n\r") + 1);
    std::cout << "< " << s << "\n";
    return s;
  }

  std::smatch wait_line(tcp::socket& socket, std::regex re) {
    std::smatch m;
    while (!std::regex_search(read_line(socket), m, re));
    return m;
  }

}


boost::asio::io_service io_service;


int main(int argc, char* argv[]) {

  if (argc < 5) {
    std::cerr << "Usage: tenuki host port username password\n";
    return 1;
  }

  const string HOST = argv[1];
  const int PORT = atoi(argv[2]);
  const string USERNAME = argv[3];
  const string PASSWORD = argv[4];

  std::cout << "Connecting to " << HOST << " port " << PORT << ".\n";
  tcp::socket socket(io_service);
  socket.connect(tcp::endpoint(address::from_string(HOST), PORT));

  write_line(socket, "LOGIN " + USERNAME + " " + PASSWORD);
  const side MYSIDE = wait_line(socket, std::regex("Your_Turn:(\\+|-)"))[1].str() == "+" ? side::BLACK : side::WHITE;
  wait_line(socket, std::regex("END Game_Summary"));

  write_line(socket, "AGREE");
  wait_line(socket, std::regex("START"));

  position p = parse_position("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1");
  std::cerr << to_string(p) << "\n";

  for (;;) {

    if (p.side_to_move == MYSIDE) {
      write_line(socket, to_string(ponder(p)));
    }
    move m = parse_move(read_line(socket));
    std::cerr << to_string(m) << "\n";
    p = do_move(p, m);
    std::cerr << to_string(p) << "\n";

  }

  return 0;
}
