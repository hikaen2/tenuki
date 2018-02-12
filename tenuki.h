#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <random>
#include <regex>
#include <string>
#include <utility>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/timer.hpp>

using side_t = uint8_t;
using type_t = uint8_t;
using dir_t = int8_t;
using square_t = uint8_t;

namespace tenuki {

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

    inline type_t promote(type_t t) {
      return t | 8;
    }
  }

  /**
   * Direction
   */
  namespace dir {
    const dir_t N = -1 * 2;
    const dir_t E = -10 * 2;
    const dir_t W = +10 * 2;
    const dir_t S = +1 * 2;
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

  /**
   * マス目
   */
  namespace square {
    const square_t WALL              = 0b10000000;
    const square_t EMPTY             = 0b01000000;
    const square_t B                 = 0b00100000;
    const square_t W                 = 0b00010000;
    const square_t B_PAWN            = B | type::PAWN;
    const square_t B_LANCE           = B | type::LANCE;
    const square_t B_KNIGHT          = B | type::KNIGHT;
    const square_t B_SILVER          = B | type::SILVER;
    const square_t B_BISHOP          = B | type::BISHOP;
    const square_t B_ROOK            = B | type::ROOK;
    const square_t B_GOLD            = B | type::GOLD;
    const square_t B_KING            = B | type::KING;
    const square_t B_PROMOTED_PAWN   = B | type::PROMOTED_PAWN;
    const square_t B_PROMOTED_LANCE  = B | type::PROMOTED_LANCE;
    const square_t B_PROMOTED_KNIGHT = B | type::PROMOTED_KNIGHT;
    const square_t B_PROMOTED_SILVER = B | type::PROMOTED_SILVER;
    const square_t B_PROMOTED_BISHOP = B | type::PROMOTED_BISHOP;
    const square_t B_PROMOTED_ROOK   = B | type::PROMOTED_ROOK;
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

    inline bool is_wall(square_t sq) {
      return sq == square::WALL;
    }

    inline bool is_empty(square_t sq) {
      return sq == square::EMPTY;
    }

    inline bool is_black(square_t sq) {
      return (sq & square::B) != 0;
    }

    inline bool is_white(square_t sq) {
      return (sq & square::W) != 0;
    }

    inline bool is_friend(square_t sq, side_t s) {
      return (sq & (s == side::BLACK ? square::B : square::W)) != 0;
    }

    inline bool is_enemy(square_t sq, side_t s) {
      return (sq & (s == side::BLACK ? square::W : square::B)) != 0;
    }

    inline type_t type_of(square_t sq) {
      assert(sq != square::EMPTY);
      return sq & 0b1111;
    }

    inline square_t unpromote(square_t sq) {
      return sq & 0b11110111;
    }
  };

  /**
   * 局面
   */
  struct position {
    int16_t static_value;         ///< 静的評価値
    side_t side_to_move;          ///< 手番
    square_t squares[111];        ///< 10 * 11 + 1
    uint8_t pieces_in_hand[2][8]; ///< [side][type]
  };

  inline int address(int file, int rank) {
    return file * 10 + rank;
  }

  inline int file_of(int address) {
    return address / 10;
  }

  inline int rank_of(int address) {
    return address % 10;
  }

  /**
   * 手
   */
  struct move {
    side_t s;
    int8_t from;
    int8_t to;
    type_t t;
  };

  /*
   * ponder.cpp
   */
  const move ponder(const position& p);

  /*
   * position.cpp
   */
  const position parse_position(const std::string& sfen);
  const std::string to_sfen(const position& p);
  const std::string to_ki2(const position& p);
  const std::string to_string(const position& p);

  /*
   * move.cpp
   */
  const std::string to_string(const move& m);
  const position do_move(position p, const move& m);
  const std::vector<move> legal_moves(const position& p);
  const move parse_move(const std::string& str);

} // namespace tenuki
