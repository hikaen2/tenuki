#include "tenuki.h"


using std::map;
using std::string;
using std::vector;


namespace tenuki {


  /**
   * to_string(move)
   */
  const std::string to_string(const move& m) {
    static const map<type, string> to_csa {
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
    return  (m.s == side::BLACK ? "+" : "-") + std::to_string(m.file_from + 1) + std::to_string(m.rank_from + 1) + std::to_string(m.file_to + 1) + std::to_string(m.rank_to + 1) + to_csa.at(m.t);
  }


  /**
   * parse_move
   */
  const move parse_move(const std::string& str) {

    static const map<string, type> DIC = {
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

    static std::regex re("(-|\\+)(\\d)(\\d)(\\d)(\\d)(\\w\\w)");
    std::smatch match;
    std::regex_search(str, match, re);
    side si = (match[1] == "+") ? side::BLACK : side::WHITE;
    int file_from = stoi(match[2]) - 1;
    int rank_from = stoi(match[3]) - 1;
    int file_to = stoi(match[4]) - 1;
    int rank_to = stoi(match[5]) - 1;
    string koma = match[6];

    move move{si, (int8_t)file_from, (int8_t)rank_from, (int8_t)file_to, (int8_t)rank_to, DIC.at(koma)};
    return move;
  }


  /**
   * do_move
   */
  const position do_move(position p, const move& m) {

    static const map<square, uint16_t> score = {
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

    assert(FILE0 <= m.file_from && m.file_from <= FILE9);
    assert(RANK0 <= m.rank_from && m.rank_from <= RANK9);
    assert(FILE1 <= m.file_to && m.file_to <= FILE9);
    assert(RANK1 <= m.rank_to && m.rank_to <= RANK9);

    square& from = p.squares[m.file_from][m.rank_from];
    square& to = p.squares[m.file_to][m.rank_to];

    // pick
    if (to != square::EMPTY) {
      p.pieces_in_hand[(int)p.side_to_move][(int)type_of(unpromote(to))]++;
      p.static_value += -score.at(to) + -score.at(unpromote(to));
    }

    to = (square)((p.side_to_move == side::BLACK ? 16 : 32) | (int)m.t);
    
    if (m.file_from == FILE0) {
      assert(m.rank_from == RANK0);
      assert(p.pieces_in_hand[(int)p.side_to_move][(int)m.t] > 0);
      p.pieces_in_hand[(int)p.side_to_move][(int)m.t]--;
    } else {
      p.static_value += -score.at(from) + score.at(to);
      from = square::EMPTY;
    }
    p.side_to_move = (p.side_to_move == side::BLACK) ? side::WHITE : side::BLACK;
    return p;
  }


  inline bool is_friend(square sq, side s) {
    return sq != square::EMPTY && side_of(sq) == s;
  }


  inline bool is_enemy(square sq, side s) {
    return sq != square::EMPTY && side_of(sq) != s;
  }


  inline bool can_promote(square sq, int rank_to, int rank_from) {
    if (to_int(type_of(sq)) > to_int(type::ROOK)) {
      return false;
    }
    if (side_of(sq) == side::BLACK) {
      return rank_to <= RANK3 || rank_from <= RANK3;
    } else {
      return rank_to >= RANK7 || rank_from >= RANK7;
    }
  }


  /**
   * legal_moves
   */
  const std::vector<move> legal_moves(const position& p) {

    const static vector<vector<vector<int>>> DIRECTIONS {
      {{ 0, -1}},                                                                       //  0:PAWN
      {{ 0, -8}},                                                                       //  1:LANCE
      {{ 1, -2}, {-1, -2}},                                                             //  2:KNIGHT
      {{ 1, -1}, { 0, -1}, {-1, -1}, { 1,  1}, {-1,  1}},                               //  3:SILVER
      {{ 8,  8}, {-8,  8}, { 8, -8}, {-8, -8}},                                         //  4:BISHOP
      {{ 0,  8}, { 8,  0}, { 0, -8}, {-8,  0}},                                         //  5:ROOK
      {{ 1, -1}, { 0, -1}, {-1, -1}, { 1,  0}, {-1,  0}, { 0,  1}},                     //  6:GOLD
      {{ 1, -1}, { 0, -1}, {-1, -1}, { 1,  0}, {-1,  0}, { 1,  1}, { 0,  1}, {-1,  1}}, //  7:KING
      {{ 1, -1}, { 0, -1}, {-1, -1}, { 1,  0}, {-1,  0}, { 0,  1}},                     //  8:PROMOTED_PAWN
      {{ 1, -1}, { 0, -1}, {-1, -1}, { 1,  0}, {-1,  0}, { 0,  1}},                     //  9:PROMOTED_LANCE
      {{ 1, -1}, { 0, -1}, {-1, -1}, { 1,  0}, {-1,  0}, { 0,  1}},                     // 10:PROMOTED_KNIGHT
      {{ 1, -1}, { 0, -1}, {-1, -1}, { 1,  0}, {-1,  0}, { 0,  1}},                     // 11:PROMOTED_SILVER
      {{ 8,  8}, {-8,  8}, { 8, -8}, {-8, -8}, { 0,  1}, { 1,  0}, { 0, -1}, {-1,  0}}, // 12:PROMOTED_BISHOP
      {{ 0,  8}, { 8,  0}, { 0, -8}, {-8,  0}, { 1,  1}, {-1,  1}, { 1, -1}, {-1, -1}}, // 13:PROMOTED_ROOK
    };

    vector<move> moves;

    if (p.pieces_in_hand[to_int(side::BLACK)][to_int(type::KING)] > 0 || p.pieces_in_hand[to_int(side::WHITE)][to_int(type::KING)] > 0) {
      return moves;
    }

    // 盤上の駒を動かす
    for (int file_from = FILE1; file_from <= FILE9; file_from++) {
      for (int rank_from = RANK1; rank_from <= RANK9; rank_from++) {
        const square& from = p.squares[file_from][rank_from];
        if (!is_friend(from, p.side_to_move)) {
          continue; // fromが味方の駒でなければcontinue
        }

        const int RANK_MIN = (from == square::B_PAWN || from == square::B_LANCE) ? RANK2 : from == square::B_KNIGHT ? RANK3 : RANK1;
        const int RANK_MAX = (from == square::W_PAWN || from == square::W_LANCE) ? RANK8 : from == square::W_KNIGHT ? RANK7 : RANK9;

        for (auto& v : DIRECTIONS[to_int(type_of(from))]) {
          int dx = v[0];
          int dy = p.side_to_move == side::BLACK ? v[1] : -v[1];
          bool fly = false;
          if (abs(dx) == 8 || abs(dy) == 8) {
            dx /= 8;
            dy /= 8;
            fly = true;
          }
          for (int file_to = file_from + dx, rank_to = rank_from + dy;  FILE1 <= file_to && file_to <= FILE9 && RANK1 <= rank_to && rank_to <= RANK9;  file_to += dx, rank_to += dy) {
            square to = p.squares[file_to][rank_to];
            if (is_friend(to, p.side_to_move)) {
              break; // 移動先に味方の駒があればbreak
            }
            if (can_promote(from, rank_to, rank_from)) {
              moves.push_back(move{p.side_to_move, (int8_t)file_from, (int8_t)rank_from, (int8_t)(file_to), (int8_t)(rank_to), promote(type_of(from))});
            }
            if (rank_to < RANK_MIN || RANK_MAX < rank_to) {
              break;
            }
            moves.push_back(move{p.side_to_move, (int8_t)file_from, (int8_t)rank_from, (int8_t)(file_to), (int8_t)(rank_to), type_of(from)});
            if (!fly) {
              break; // 飛び駒でなければここでbreak
            }
            if (is_enemy(to, p.side_to_move)) {
              break;
            }
          }
        }
      }
    }

    // 持ち駒を打つ
    for (int file_to = FILE1; file_to <= FILE9; file_to++) { // 1筋～9筋

      bool nifu = false; // この筋に味方の歩があるか
      if (p.pieces_in_hand[to_int(p.side_to_move)][to_int(type::PAWN)] > 0) {
        for (int rank_to = RANK1; rank_to <= RANK9; rank_to++) { // 1段目～9段目
          nifu |= p.squares[file_to][rank_to] == (p.side_to_move == side::BLACK ? square::B_PAWN : square::W_PAWN);
        }
      }

      for (int rank_to = RANK1; rank_to <= RANK9; rank_to++) { // 1段目～9段目
        if (p.squares[file_to][rank_to] != square::EMPTY) {
          continue;
        }
        for (int t = to_int(type::PAWN); t <= to_int(type::GOLD); t++) { // 歩,香,桂,銀,角,飛,金
          if ((t == to_int(type::PAWN) || t == to_int(type::LANCE)) && (p.side_to_move == side::BLACK ? rank_to == RANK1 : rank_to == RANK9)) {
            continue; // 歩,香は1段目（先手）/9段目（後手）に打てない
          }
          if (t == to_int(type::KNIGHT) && (p.side_to_move == side::BLACK ? rank_to <= RANK2 : rank_to >= RANK8)) {
            continue; // 桂は2段目より上（先手）/8段目より下（後手）に打てない
          }
          if (p.pieces_in_hand[to_int(p.side_to_move)][t] == 0) {
            continue;
          }
          if (t == to_int(type::PAWN) && nifu) {
            continue;
          }
          moves.push_back(move{p.side_to_move, -1, -1, (int8_t)file_to, (int8_t)rank_to, to_type(t)});
        }
      }
    }
    return moves;
  }


} // namespace tenuki
