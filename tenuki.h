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

namespace tenuki {

  /**
   * 手番
   */
  enum struct side : uint8_t {
    BLACK = 0, ///< 先手
    WHITE = 1, ///< 後手
  };


  /**
   * 
   */
  inline uint8_t to_int(side s) {
    return static_cast<uint8_t>(s);
  }


  /**
   * 駒の種類
   */
  enum struct type : uint8_t {
    PAWN            = 0,
    LANCE           = 1,
    KNIGHT          = 2,
    SILVER          = 3,
    BISHOP          = 4,
    ROOK            = 5,
    GOLD            = 6,
    KING            = 7,
    PROMOTED_PAWN   = 8 + 0,
    PROMOTED_LANCE  = 8 + 1,
    PROMOTED_KNIGHT = 8 + 2,
    PROMOTED_SILVER = 8 + 3,
    PROMOTED_BISHOP = 8 + 4,
    PROMOTED_ROOK   = 8 + 5,
  };


  /**
   * 
   */
  inline uint8_t to_int(type t) {
    return static_cast<uint8_t>(t);
  }


  /**
   * 
   */
  inline type to_type(int t) {
    return static_cast<type>(t);
  }


  inline type promote(type t) {
    return static_cast<type>(to_int(t) | 8);
  }


  /**
   * マス目
   */
  enum struct square : uint8_t {
    EMPTY               = 0,
    B_PAWN              = 16 + 0,
    B_LANCE             = 16 + 1,
    B_KNIGHT            = 16 + 2,
    B_SILVER            = 16 + 3,
    B_BISHOP            = 16 + 4,
    B_ROOK              = 16 + 5,
    B_GOLD              = 16 + 6,
    B_KING              = 16 + 7,
    B_PROMOTED_PAWN     = 16 + 8 + 0,
    B_PROMOTED_LANCE    = 16 + 8 + 1,
    B_PROMOTED_KNIGHT   = 16 + 8 + 2,
    B_PROMOTED_SILVER   = 16 + 8 + 3,
    B_PROMOTED_BISHOP   = 16 + 8 + 4,
    B_PROMOTED_ROOK     = 16 + 8 + 5,
    W_PAWN              = 32 + 0,
    W_LANCE             = 32 + 1,
    W_KNIGHT            = 32 + 2,
    W_SILVER            = 32 + 3,
    W_BISHOP            = 32 + 4,
    W_ROOK              = 32 + 5,
    W_GOLD              = 32 + 6,
    W_KING              = 32 + 7,
    W_PROMOTED_PAWN     = 32 + 8 + 0,
    W_PROMOTED_LANCE    = 32 + 8 + 1,
    W_PROMOTED_KNIGHT   = 32 + 8 + 2,
    W_PROMOTED_SILVER   = 32 + 8 + 3,
    W_PROMOTED_BISHOP   = 32 + 8 + 4,
    W_PROMOTED_ROOK     = 32 + 8 + 5,
  };


  /**
   * 
   */
  inline side side_of(square sq) {
    assert(sq != square::EMPTY);
    return static_cast<side>(static_cast<uint8_t>(sq) >> 5);
  }


  /**
   * 
   */
  inline type type_of(square sq) {
    assert(sq != square::EMPTY);
    return static_cast<type>(static_cast<uint8_t>(sq) & 0b1111);
  }


  inline square unpromote(square sq) {
    return static_cast<square>(static_cast<int>(sq) & 0b11110111);
  }


  /**
   * 局面
   */
  struct position {
    int16_t static_value;         ///< 静的評価値
    side side_to_move;            ///< 手番
    square squares[9][9];         ///< [file][rank]
    uint8_t pieces_in_hand[2][8]; ///< [Side][type]
  };


  /**
   * 手
   */
  struct move {
    side s;
    int8_t file_from;
    int8_t rank_from;
    int8_t file_to;
    int8_t rank_to;
    type t;
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

