#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
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

using type_t = uint8_t;
using side_t = uint8_t;
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
  }

  inline type_t promote(type_t t) {
    return t | 8;
  }

  /**
   * マス目
   */
  namespace square {
    const square_t EMPTY               = 0;
    const square_t B_PAWN              = 16 + 0;
    const square_t B_LANCE             = 16 + 1;
    const square_t B_KNIGHT            = 16 + 2;
    const square_t B_SILVER            = 16 + 3;
    const square_t B_BISHOP            = 16 + 4;
    const square_t B_ROOK              = 16 + 5;
    const square_t B_GOLD              = 16 + 6;
    const square_t B_KING              = 16 + 7;
    const square_t B_PROMOTED_PAWN     = 16 + 8 + 0;
    const square_t B_PROMOTED_LANCE    = 16 + 8 + 1;
    const square_t B_PROMOTED_KNIGHT   = 16 + 8 + 2;
    const square_t B_PROMOTED_SILVER   = 16 + 8 + 3;
    const square_t B_PROMOTED_BISHOP   = 16 + 8 + 4;
    const square_t B_PROMOTED_ROOK     = 16 + 8 + 5;
    const square_t W_PAWN              = 32 + 0;
    const square_t W_LANCE             = 32 + 1;
    const square_t W_KNIGHT            = 32 + 2;
    const square_t W_SILVER            = 32 + 3;
    const square_t W_BISHOP            = 32 + 4;
    const square_t W_ROOK              = 32 + 5;
    const square_t W_GOLD              = 32 + 6;
    const square_t W_KING              = 32 + 7;
    const square_t W_PROMOTED_PAWN     = 32 + 8 + 0;
    const square_t W_PROMOTED_LANCE    = 32 + 8 + 1;
    const square_t W_PROMOTED_KNIGHT   = 32 + 8 + 2;
    const square_t W_PROMOTED_SILVER   = 32 + 8 + 3;
    const square_t W_PROMOTED_BISHOP   = 32 + 8 + 4;
    const square_t W_PROMOTED_ROOK     = 32 + 8 + 5;
  };

  inline side_t side_of(square_t sq) {
    assert(sq != square::EMPTY);
    return sq >> 5;
  }

  inline type_t type_of(square_t sq) {
    assert(sq != square::EMPTY);
    return sq & 0b1111;
  }

  inline square_t unpromote(square_t sq) {
    return sq & 0b11110111;
  }

  /**
   * 局面
   */
  struct position {
    int16_t static_value;         ///< 静的評価値
    side_t side_to_move;          ///< 手番
    square_t squares[9][9];       ///< [file][rank]
    uint8_t pieces_in_hand[2][8]; ///< [Side][type]
  };

  /**
   * 手
   */
  struct move {
    side_t s;
    int8_t file_from;
    int8_t rank_from;
    int8_t file_to;
    int8_t rank_to;
    type_t t;
  };

  const int FILE0 = -1;
  const int FILE1 = 0;
  const int FILE2 = 1;
  const int FILE3 = 2;
  const int FILE4 = 3;
  const int FILE5 = 4;
  const int FILE6 = 5;
  const int FILE7 = 6;
  const int FILE8 = 7;
  const int FILE9 = 8;

  const int RANK0 = -1;
  const int RANK1 = 0;
  const int RANK2 = 1;
  const int RANK3 = 2;
  const int RANK4 = 3;
  const int RANK5 = 4;
  const int RANK6 = 5;
  const int RANK7 = 6;
  const int RANK8 = 7;
  const int RANK9 = 8;
  
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
