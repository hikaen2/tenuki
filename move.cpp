#include "tenuki.h"

using std::map;
using std::string;
using std::vector;

namespace tenuki {

  /**
   * to_string(move)
   */
  const std::string to_string(move_t m, const position& p) {

    static const map<type_t, string> to_csa {
      {type::PAWN,            "FU"},
      {type::LANCE,           "KY"},
      {type::KNIGHT,          "KE"},
      {type::SILVER,          "GI"},
      {type::GOLD,            "KI"},
      {type::BISHOP,          "KA"},
      {type::ROOK,            "HI"},
      {type::KING,            "OU"},
      {type::PROMOTED_PAWN,   "TO"},
      {type::PROMOTED_LANCE,  "NY"},
      {type::PROMOTED_KNIGHT, "NK"},
      {type::PROMOTED_SILVER, "NG"},
      {type::PROMOTED_BISHOP, "UM"},
      {type::PROMOTED_ROOK,   "RY"},
    };
    assert(to_csa.count(m.t) > 0);
    int from = move::is_drop(m) ? 0 : move::from(m);
    int to = move::to(m);
    type_t t = move::is_drop(m) ? move::from(m) : move::is_promote(m) ? square::type_of(square::promote(p.squares[move::from(m)])) : square::type_of(p.squares[move::from(m)]);
    return (boost::format("%s%02d%02d%s") % (p.side_to_move == side::BLACK ? "+" : "-") % from % to % to_csa.at(t)).str();
  }


  /**
   * parse_move
   */
  move_t parse_move(const std::string& str, const position& p) {

    static const map<string, type_t> dic {
      {"FU", type::PAWN},
      {"KY", type::LANCE},
      {"KE", type::KNIGHT},
      {"GI", type::SILVER},
      {"KI", type::GOLD},
      {"KA", type::BISHOP},
      {"HI", type::ROOK},
      {"OU", type::KING},
      {"TO", type::PROMOTED_PAWN},
      {"NY", type::PROMOTED_LANCE},
      {"NK", type::PROMOTED_KNIGHT},
      {"NG", type::PROMOTED_SILVER},
      {"UM", type::PROMOTED_BISHOP},
      {"RY", type::PROMOTED_ROOK},
    };

    static std::regex re("(-|\\+)(\\d{2})(\\d{2})(\\w{2})");
    std::smatch match;
    std::regex_search(str, match, re);
    int from = stoi(match[2]);
    int to = stoi(match[3]);
    type_t t = dic.at(match[4]);

    if (from == 0) {
      return move::drop(t, to); // fromが0なら駒打ち
    } else if (t != square::type_of(p.squares[from])) {
      return move::promote(from, to); // 成る
    } else {
      return move::create(from, to);
    }
  }


  /**
   * do_move
   */
  const position do_move(position p, move_t m) {

    static const std::unordered_map<square_t, uint16_t> score = {
      {square::B_PAWN,              87},
      {square::B_LANCE,            235},
      {square::B_KNIGHT,           254},
      {square::B_SILVER,           371},
      {square::B_GOLD,             447},
      {square::B_BISHOP,           571},
      {square::B_ROOK,             647},
      {square::B_KING,            9999},
      {square::B_PROMOTED_PAWN,    530},
      {square::B_PROMOTED_LANCE,   482},
      {square::B_PROMOTED_KNIGHT,  500},
      {square::B_PROMOTED_SILVER,  489},
      {square::B_PROMOTED_BISHOP,  832},
      {square::B_PROMOTED_ROOK,    955},
      {square::W_PAWN,             -87},
      {square::W_LANCE,           -235},
      {square::W_KNIGHT,          -254},
      {square::W_SILVER,          -371},
      {square::W_GOLD,            -447},
      {square::W_BISHOP,          -571},
      {square::W_ROOK,            -647},
      {square::W_KING,           -9999},
      {square::W_PROMOTED_PAWN,   -530},
      {square::W_PROMOTED_LANCE,  -482},
      {square::W_PROMOTED_KNIGHT, -500},
      {square::W_PROMOTED_SILVER, -489},
      {square::W_PROMOTED_BISHOP, -832},
      {square::W_PROMOTED_ROOK,   -955},
    };

    assert(0 <= move::from(m) &&  move::from(m) <= 99);
    assert(11 <= move::to(m) && move::to(m) <= 99);

    square_t& sq_from = p.squares[move::from(m)];
    square_t& sq_to = p.squares[move::to(m)];

    // pick
    if (sq_to != square::EMPTY) {
      p.pieces_in_hand[p.side_to_move][square::type_of(square::unpromote(sq_to))]++;
      p.static_value += -score.at(sq_to) + -score.at(square::unpromote(sq_to));
    }

    if (move::is_drop(m)) {
      assert(p.pieces_in_hand[p.side_to_move][move::from(m)] > 0);
      sq_to = ((p.side_to_move == side::BLACK ? square::B : square::W) | move::from(m));
      p.pieces_in_hand[p.side_to_move][move::from(m)]--;
    } else {
      sq_to = move::is_promote(m) ? square::promote(sq_from) : sq_from;
      p.static_value += -score.at(sq_from) + score.at(sq_to);
      sq_from = square::EMPTY;
    }
    p.side_to_move = (p.side_to_move == side::BLACK) ? side::WHITE : side::BLACK;
    return p;
  }


  namespace {
    inline bool can_promote(square_t sq, int rank_to, int rank_from) {
      if (square::type_of(sq) > type::ROOK) {
        return false;
      }
      if (square::is_black(sq)) {
        return rank_to <= 3 || rank_from <= 3;
      } else {
        return rank_to >= 7 || rank_from >= 7;
      }
    }
  }

  /**
   * legal_moves
   */
  const std::vector<move_t> legal_moves(const position& p) {

    const static vector<vector<dir_t>> DIRECTIONS {
      { dir::N },                                                                     //  0:PAWN
      { dir::FN },                                                                    //  1:LANCE
      { dir::NNE, dir::NNW },                                                         //  2:KNIGHT
      { dir::N,   dir::NE,  dir::NW,  dir::SE,  dir::SW },                            //  3:SILVER
      { dir::FNE, dir::FNW, dir::FSE, dir::FSW },                                     //  4:BISHOP
      { dir::FN,  dir::FE,  dir::FW,  dir::FS },                                      //  5:ROOK
      { dir::N,   dir::NE,  dir::NW,  dir::E,   dir::W,  dir::S },                    //  6:GOLD
      { dir::N,   dir::NE,  dir::NW,  dir::E,   dir::W,  dir::S,  dir::SE, dir::SW }, //  7:KING
      { dir::N,   dir::NE,  dir::NW,  dir::E,   dir::W,  dir::S },                    //  8:PROMOTED_PAWN
      { dir::N,   dir::NE,  dir::NW,  dir::E,   dir::W,  dir::S },                    //  9:PROMOTED_LANCE
      { dir::N,   dir::NE,  dir::NW,  dir::E,   dir::W,  dir::S },                    // 10:PROMOTED_KNIGHT
      { dir::N,   dir::NE,  dir::NW,  dir::E,   dir::W,  dir::S },                    // 11:PROMOTED_SILVER
      { dir::FNE, dir::FNW, dir::FSE, dir::FSW, dir::N,  dir::E,  dir::W,  dir::S },  // 12:PROMOTED_BISHOP
      { dir::FN,  dir::FE,  dir::FW,  dir::FS,  dir::NE, dir::NW, dir::SE, dir::SW }, // 13:PROMOTED_ROOK
    };

    const static int RANK_MIN[] {
      // ▲歩,香,桂,銀,角,飛,金,王,と,成香,成桂,成銀,馬,龍,-,-,△歩,香,桂,銀,角,飛,金,王,と,成香,成桂,成銀,馬,龍
      2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    };

    const static int RANK_MAX[] {
      // ▲歩,香,桂,銀,角,飛,金,王,と,成香,成桂,成銀,馬,龍,-,-,△歩,香,桂,銀,角,飛,金,王,と,成香,成桂,成銀,馬,龍
      9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 0, 0, 8, 8, 7, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    };

    vector<move_t> moves;
    moves.reserve(256);

    if (p.pieces_in_hand[side::BLACK][type::KING] > 0 || p.pieces_in_hand[side::WHITE][type::KING] > 0) {
      return moves;
    }

    bool fued[10] = {false, false, false, false, false, false, false, false, false, false}; // 0～9筋に味方の歩があるか

    // 盤上の駒を動かす
    for (int from = 11; from <= 99; from++) {
      const square_t& sq_from = p.squares[from];
      if (!square::is_friend(sq_from, p.side_to_move)) {
        continue; // fromが味方の駒でなければcontinue
      }

      fued[file_of(from)] |= (square::type_of(p.squares[from]) == type::PAWN);
      for (dir_t d : DIRECTIONS[square::type_of(sq_from)]) {
        int v = (p.side_to_move == side::BLACK ? dir::value(d) : -dir::value(d));
        for (int to = from + v; square::is_empty(p.squares[to]) || square::is_enemy(p.squares[to], p.side_to_move);  to += v) {
          if (can_promote(sq_from, rank_of(to), rank_of(from))) {
            moves.push_back(move::promote(from, to));
          }
          if (rank_of(to) < RANK_MIN[sq_from & 0x1f] || RANK_MAX[sq_from & 0x1f] < rank_of(to)) {
            break;
          }
          moves.push_back(move::create(from, to));
          if (!dir::is_fly(d) || square::is_enemy(p.squares[to], p.side_to_move)) {
            break; // 飛び駒でなければここでbreak
          }
        }

      }
    }

    // 持ち駒を打つ
    for (int to = 11; to <= 99; to++) {
      if (!square::is_empty(p.squares[to])) {
        continue;
      }

      for (type_t t = (fued[file_of(to)] ? type::LANCE : type::PAWN); t <= type::GOLD; t++) { // 歩,香,桂,銀,角,飛,金
        if (p.pieces_in_hand[p.side_to_move][t] > 0 && rank_of(to) >= RANK_MIN[p.side_to_move << 4 | t] && RANK_MAX[p.side_to_move << 4 | t] >= rank_of(to)) {
          moves.push_back(move::drop(t, to));
        }
      }
    }

    return moves;
  }

}
