#include "tenuki.h"

using std::map;
using std::string;
using std::vector;

namespace tenuki {

  /**
   * to_string(move)
   */
  const std::string to_string(const move& m) {
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

    std::ostringstream oss;
    oss << (m.s == side::BLACK ? "+" : "-");
    oss << std::setfill('0') << std::right << std::setw(2) << (int)m.from;
    oss << std::setfill('0') << std::right << std::setw(2) << (int)m.to;
    oss << to_csa.at(m.t);
    return oss.str();
  }


  /**
   * parse_move
   */
  const move parse_move(const std::string& str) {

    static const map<string, type_t> DIC = {
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
    side_t si = (match[1] == "+") ? side::BLACK : side::WHITE;
    int from = stoi(match[2]);
    int to = stoi(match[3]);
    string koma = match[4];

    move move{si, (int8_t)from, (int8_t)to, DIC.at(koma)};
    return move;
  }


  /**
   * do_move
   */
  const position do_move(position p, const move& m) {

    static const map<square_t, uint16_t> score = {
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

    assert(0 <= m.from && m.from <= 99);
    assert(11 <= m.to && m.to <= 99);

    square_t& from = p.squares[m.from];
    square_t& to = p.squares[m.to];

    // pick
    if (to != square::EMPTY) {
      p.pieces_in_hand[p.side_to_move][square::type_of(square::unpromote(to))]++;
      p.static_value += -score.at(to) + -score.at(square::unpromote(to));
    }

    to = (square_t)((p.side_to_move == side::BLACK ? square::B : square::W) | m.t);
    
    if (m.from == 0) {
      assert(p.pieces_in_hand[p.side_to_move][m.t] > 0);
      p.pieces_in_hand[p.side_to_move][m.t]--;
    } else {
      p.static_value += -score.at(from) + score.at(to);
      from = square::EMPTY;
    }
    p.side_to_move = (p.side_to_move == side::BLACK) ? side::WHITE : side::BLACK;
    return p;
  }


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


  /**
   * legal_moves
   */
  const std::vector<move> legal_moves(const position& p) {

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

    vector<move> moves;

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

      fued[file_of(from)] |= (square::type_of(sq_from) == type::PAWN);
      const int RANK_MIN = (sq_from == square::B_PAWN || sq_from == square::B_LANCE) ? 2 : sq_from == square::B_KNIGHT ? 3 : 1;
      const int RANK_MAX = (sq_from == square::W_PAWN || sq_from == square::W_LANCE) ? 8 : sq_from == square::W_KNIGHT ? 7 : 9;

      for (dir_t d : DIRECTIONS[square::type_of(sq_from)]) {
        int v = (p.side_to_move == side::BLACK ? dir::value(d) : -dir::value(d));
        for (int to = from + v; square::is_empty(p.squares[to]) || square::is_enemy(p.squares[to], p.side_to_move);  to += v) {
          square_t sq_to = p.squares[to];
          if (can_promote(sq_from, rank_of(to), rank_of(from))) {
            moves.push_back(move{p.side_to_move, (int8_t)from, (int8_t)to, type::promote(square::type_of(sq_from))});
          }
          if (rank_of(to) < RANK_MIN || RANK_MAX < rank_of(to)) {
            break;
          }
          moves.push_back(move{p.side_to_move, (int8_t)from, (int8_t)to, square::type_of(sq_from)});
          if (!dir::is_fly(d) || square::is_enemy(sq_to, p.side_to_move)) {
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
        if ((t == type::PAWN || t == type::LANCE) && (p.side_to_move == side::BLACK ? rank_of(to) == 1 : rank_of(to) == 9)) {
          continue; // 歩,香は1段目（先手）/9段目（後手）に打てない
        }
        if (t == type::KNIGHT && (p.side_to_move == side::BLACK ? rank_of(to) <= 2 : rank_of(to) >= 8)) {
          continue; // 桂は2段目より上（先手）/8段目より下（後手）に打てない
        }
        if (p.pieces_in_hand[p.side_to_move][t] == 0) {
          continue;
        }
        moves.push_back(move{p.side_to_move, 0, (int8_t)to, t});
      }
    }

    return moves;
  }


} // namespace tenuki
