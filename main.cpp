#include "tenuki.h"


using namespace tenuki;
using std::string;
using std::vector;
using std::map;

const map<square, uint16_t> score = {
  {square::B_PAWN,              100},
  {square::B_LANCE,             600},
  {square::B_KNIGHT,            700},
  {square::B_SILVER,           1000},
  {square::B_GOLD,             1200},
  {square::B_BISHOP,           1800},
  {square::B_ROOK,             2000},
  {square::B_KING,            99999},
  {square::B_PROMOTED_PAWN,    1200},
  {square::B_PROMOTED_LANCE,   1200},
  {square::B_PROMOTED_KNIGHT,  1200},
  {square::B_PROMOTED_SILVER,  1200},
  {square::B_PROMOTED_BISHOP,  2200},
  {square::B_PROMOTED_ROOK,    2400},
  {square::W_PAWN,             -100},
  {square::W_LANCE,            -600},
  {square::W_KNIGHT,           -700},
  {square::W_SILVER,          -1000},
  {square::W_GOLD,            -1200},
  {square::W_BISHOP,          -1800},
  {square::W_ROOK,            -2000},
  {square::W_KING,           -99999},
  {square::W_PROMOTED_PAWN,   -1200},
  {square::W_PROMOTED_LANCE,  -1200},
  {square::W_PROMOTED_KNIGHT, -1200},
  {square::W_PROMOTED_SILVER, -1200},
  {square::W_PROMOTED_BISHOP, -2200},
  {square::W_PROMOTED_ROOK,   -2400},
};



int main() {

  position p = parse_position("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1");
  //position p = parse_sfen("lnsgkg1n1/1r5sl/1ppppp1p1/p5p1p/8P/2P6/PP1PPPPP1/L6R1/1NSGKGSNL b bB 1");

  //vector<move> ms = legal_moves(p);
  //for (move m: ms) {
  //  std::cerr << to_string(m) << " ";
  //}
  //std::cerr << to_string(p) << "\n";
  //exit(1);


  std::cerr << to_string(p) << "\n";
  
  
  for (;;) {
    std::cerr << "> ";
    string line;
    std::getline(std::cin, line);

    try {
      move m = parse_move(line);
      std::cerr << to_string(m) << "\n";
      p = do_move(p, m);
      std::cerr << to_string(p) << "\n";

      p = do_move(p, ponder(p));
      std::cerr << to_string(p) << "\n";

    } catch(std::exception e) {
      std::cerr << e.what() << "\n";
    }


  }
  
  
  //  position p = parse_sfen("l6nl/5+P1gk/2np1S3/p1p4Pp/3P2Sp1/1PPb2P1P/P5GS1/R8/LN4bKL w GR5pnsg 1");
  //  vector<move> moves;
  //  for (int i = 0 ; i <= 5000000; i++) {
    //moves.clear();
    //legal_moves(p, moves);
    //  }
  //  for (move m : moves) {
//    std::cout << to_string(m) << ", ";
//  }
//  std::cout << "\n";
//  std::cout << to_ki2(p) << "\n";
//  std::cout << to_sfen(p) << "\n";
  return 0;
}
