#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <unordered_map>
#include <random>
#include <regex>
#include <string>
#include <utility>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/timer.hpp>

namespace tenuki {

  using side_t = uint8_t;
  using type_t = uint8_t;
  using dir_t = int8_t;
  using square_t = uint8_t;
  using move_t = uint16_t;

  /**
   * 手番
   */
  namespace side {
    const side_t BLACK = 0; ///< 先手
    const side_t WHITE = 1; ///< 後手
  }

  /**
   * 駒の種類
   */
  namespace type {
    const type_t PAWN            = 0;
    const type_t LANCE           = 1;
    const type_t KNIGHT          = 2;
    const type_t SILVER          = 3;
    const type_t BISHOP          = 4;
    const type_t ROOK            = 5;
    const type_t GOLD            = 6;
    const type_t KING            = 7;
    const type_t PROMOTED_PAWN   = 8;
    const type_t PROMOTED_LANCE  = 9;
    const type_t PROMOTED_KNIGHT = 10;
    const type_t PROMOTED_SILVER = 11;
    const type_t PROMOTED_BISHOP = 12;
    const type_t PROMOTED_ROOK   = 13;
    const type_t EMPTY           = 14;
    const type_t WALL            = 15;
  }

  /**
   * 升
   * 1xxxxxxx WALL
   * x1xxxxxx EMPTY
   * xx1xxxxx BLACK
   * xxx1xxxx WHITE
   * xxxx1111 type
   */
  namespace square {
    const square_t W                 = 0b00010000;

    const square_t B_PAWN            = type::PAWN;
    const square_t B_LANCE           = type::LANCE;
    const square_t B_KNIGHT          = type::KNIGHT;
    const square_t B_SILVER          = type::SILVER;
    const square_t B_BISHOP          = type::BISHOP;
    const square_t B_ROOK            = type::ROOK;
    const square_t B_GOLD            = type::GOLD;
    const square_t B_KING            = type::KING;
    const square_t B_PROMOTED_PAWN   = type::PROMOTED_PAWN;
    const square_t B_PROMOTED_LANCE  = type::PROMOTED_LANCE;
    const square_t B_PROMOTED_KNIGHT = type::PROMOTED_KNIGHT;
    const square_t B_PROMOTED_SILVER = type::PROMOTED_SILVER;
    const square_t B_PROMOTED_BISHOP = type::PROMOTED_BISHOP;
    const square_t B_PROMOTED_ROOK   = type::PROMOTED_ROOK;
    const square_t EMPTY             = type::EMPTY;
    const square_t WALL              = type::WALL;
    const square_t W_PAWN            = W | type::PAWN;
    const square_t W_LANCE           = W | type::LANCE;
    const square_t W_KNIGHT          = W | type::KNIGHT;
    const square_t W_SILVER          = W | type::SILVER;
    const square_t W_BISHOP          = W | type::BISHOP;
    const square_t W_ROOK            = W | type::ROOK;
    const square_t W_GOLD            = W | type::GOLD;
    const square_t W_KING            = W | type::KING;
    const square_t W_PROMOTED_PAWN   = W | type::PROMOTED_PAWN;
    const square_t W_PROMOTED_LANCE  = W | type::PROMOTED_LANCE;
    const square_t W_PROMOTED_KNIGHT = W | type::PROMOTED_KNIGHT;
    const square_t W_PROMOTED_SILVER = W | type::PROMOTED_SILVER;
    const square_t W_PROMOTED_BISHOP = W | type::PROMOTED_BISHOP;
    const square_t W_PROMOTED_ROOK   = W | type::PROMOTED_ROOK;

    inline bool is_black(square_t sq) {
      return sq <= square::B_PROMOTED_ROOK;
    }

    inline bool is_white(square_t sq) {
      return sq >= square::W_PAWN;
    }

    inline bool is_friend(square_t sq, side_t s) {
      return s == side::BLACK ? is_black(sq) : is_white(sq);
    }

    inline bool is_enemy(square_t sq, side_t s) {
      return s == side::BLACK ? is_white(sq) : is_black(sq);
    }

    inline type_t type_of(square_t sq) {
      assert(sq != square::EMPTY);
      return sq & 0b1111;
    }

    inline type_t promote(square_t sq) {
      return sq | 0b00001000;
    }

    inline square_t unpromote(square_t sq) {
      return sq & 0b11110111;
    }
  }

  /**
   * 手
   * 1xxxxxxx xxxxxxxx promote
   * x1xxxxxx xxxxxxxx drop
   * xx111111 1xxxxxxx from
   * xxxxxxxx x1111111 to
   */
  namespace move {
    inline move_t create(int from, int to) {
      return from << 7 | to;
    }

    inline move_t create_promote(int from, int to) {
      return from << 7 | to | 0b1000000000000000;
    }

    inline move_t create_drop(type_t t, int to) {
      return t << 7 | to | 0b0100000000000000;
    }

    inline int from(move_t m) {
      return (m >> 7) & 0b01111111;
    }

    inline int to(move_t m) {
      return m & 0b01111111;
    }

    inline bool is_promote(move_t m) {
      return (m & 0b1000000000000000) != 0;
    }

    inline bool is_drop(move_t m) {
      return (m & 0b0100000000000000) != 0;
    }
  }

  /**
   * 局面
   */
  struct position {
    /**
     * squares[10 * 11 + 1]:
     *  壁   9   8   7   6   5   4   3   2   1  壁
     * -------------------------------------------+
     * 100  90  80  70  60  50  40  30  20  10   0|壁
     * 101  91  81  71  61  51  41  31  21  11   1|一
     * 102  92  82  72  62  52  42  32  22  12   2|二
     * 103  93  83  73  63  53  43  33  23  13   3|三
     * 104  94  84  74  64  54  44  34  24  14   4|四
     * 105  95  85  75  65  55  45  35  25  15   5|五
     * 106  96  86  76  66  56  46  36  26  16   6|六
     * 107  97  87  77  67  57  47  37  27  17   7|七
     * 108  98  88  78  68  58  48  38  28  18   8|八
     * 109  99  89  79  69  59  49  39  29  19   9|九
     * 110
     */
    square_t squares[111];
    uint8_t pieces_in_hand[2][8]; ///< [side][type]
    side_t side_to_move;          ///< 手番
  };

  /**
   * Direction
   * 1111111x value
   * xxxxxxx1 fly
   */
  namespace dir {
    const dir_t N =  -1 * 2; //  -1 << 1
    const dir_t E = -10 * 2; // -10 << 1
    const dir_t W = +10 * 2; // +10 << 1
    const dir_t S =  +1 * 2; //  +1 << 1
    const dir_t NE = N + E;
    const dir_t NW = N + W;
    const dir_t SE = S + E;
    const dir_t SW = S + W;
    const dir_t NNE = N + N + E;
    const dir_t NNW = N + N + W;
    const dir_t FN = N | 1;
    const dir_t FE = E | 1;
    const dir_t FW = W | 1;
    const dir_t FS = S | 1;
    const dir_t FNE = NE | 1;
    const dir_t FNW = NW | 1;
    const dir_t FSE = SE | 1;
    const dir_t FSW = SW | 1;

    inline bool is_fly(dir_t d) {
      return (d & 1) != 0;
    }

    inline int value(dir_t d) {
      return d >> 1;
    }
  }

  inline int address(int file, int rank) {
    return file * 10 + rank;
  }

  inline int file_of(int address) {
    static const int16_t a[] = {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
      4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
      5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
      7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
      8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
      9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    };
    return a[address];
  }

  inline int rank_of(int address) {
    static const int16_t a[] = {
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    };
    return a[address];
  }

  /*
   * ponder.cpp
   */
  move_t ponder(const position& p);

  /*
   * position.cpp
   */
  const position parse_position(const std::string& sfen);
  const std::string to_sfen(const position& p);
  const std::string to_ki2(const position& p);
  const std::string to_string(const position& p);
  int16_t static_value(const position& p);

  /*
   * move.cpp
   */
  const std::string to_string(move_t m, const position& p);
  move_t parse_move(const std::string& str, const position& p);
  const position do_move(position p, move_t m);
  const std::vector<move_t> legal_moves(const position& p);

}
