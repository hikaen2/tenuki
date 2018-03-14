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
#include <tuple>
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
        uint8_t pieces_in_hand[2][8]; // [side_t][type_t]
        side_t side_to_move;          // 手番
    };

    /**
     * 手番
     */
    namespace side {
        constexpr side_t BLACK = 0; // 先手
        constexpr side_t WHITE = 1; // 後手
    }

    /**
     * 駒の種類
     * 8を足すと成る. 5以下が成れる.
     */
    namespace type {
        constexpr type_t PAWN            = 0;  // 歩
        constexpr type_t LANCE           = 1;  // 香
        constexpr type_t KNIGHT          = 2;  // 桂
        constexpr type_t SILVER          = 3;  // 銀
        constexpr type_t BISHOP          = 4;  // 角
        constexpr type_t ROOK            = 5;  // 飛
        constexpr type_t GOLD            = 6;  // 金
        constexpr type_t KING            = 7;  // 王
        constexpr type_t PROMOTED_PAWN   = 8;  // と
        constexpr type_t PROMOTED_LANCE  = 9;  // 成香
        constexpr type_t PROMOTED_KNIGHT = 10; // 成桂
        constexpr type_t PROMOTED_SILVER = 11; // 成銀
        constexpr type_t PROMOTED_BISHOP = 12; // 馬
        constexpr type_t PROMOTED_ROOK   = 13; // 龍
        constexpr type_t EMPTY           = 14; // 空
        constexpr type_t WALL            = 15; // 壁
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
        constexpr square_t W                 = 0b00010000;
        constexpr square_t B_PAWN            = 0;
        constexpr square_t B_LANCE           = 1;
        constexpr square_t B_KNIGHT          = 2;
        constexpr square_t B_SILVER          = 3;
        constexpr square_t B_BISHOP          = 4;
        constexpr square_t B_ROOK            = 5;
        constexpr square_t B_GOLD            = 6;
        constexpr square_t B_KING            = 7;
        constexpr square_t B_PROMOTED_PAWN   = 8;
        constexpr square_t B_PROMOTED_LANCE  = 9;
        constexpr square_t B_PROMOTED_KNIGHT = 10;
        constexpr square_t B_PROMOTED_SILVER = 11;
        constexpr square_t B_PROMOTED_BISHOP = 12;
        constexpr square_t B_PROMOTED_ROOK   = 13;
        constexpr square_t EMPTY             = 14;
        constexpr square_t WALL              = 15;
        constexpr square_t W_PAWN            = W | 0;
        constexpr square_t W_LANCE           = W | 1;
        constexpr square_t W_KNIGHT          = W | 2;
        constexpr square_t W_SILVER          = W | 3;
        constexpr square_t W_BISHOP          = W | 4;
        constexpr square_t W_ROOK            = W | 5;
        constexpr square_t W_GOLD            = W | 6;
        constexpr square_t W_KING            = W | 7;
        constexpr square_t W_PROMOTED_PAWN   = W | 8;
        constexpr square_t W_PROMOTED_LANCE  = W | 9;
        constexpr square_t W_PROMOTED_KNIGHT = W | 10;
        constexpr square_t W_PROMOTED_SILVER = W | 11;
        constexpr square_t W_PROMOTED_BISHOP = W | 12;
        constexpr square_t W_PROMOTED_ROOK   = W | 13;
        constexpr bool is_black(square_t sq) { return sq <= square::B_PROMOTED_ROOK; }
        constexpr bool is_white(square_t sq) { return sq >= square::W_PAWN; }
        constexpr bool is_friend(square_t sq, side_t s) { return s == side::BLACK ? is_black(sq) : is_white(sq); }
        constexpr bool is_enemy(square_t sq, side_t s) { return s == side::BLACK ? is_white(sq) : is_black(sq); }
        constexpr type_t type_of(square_t sq) { return sq & 0b00001111; }
        constexpr square_t promote(square_t sq) { return sq | 0b00001000; }
        constexpr square_t unpromote(square_t sq) { return sq & 0b11110111; }
    }

    /**
     * 手
     * 1xxxxxxx xxxxxxxx promote
     * x1xxxxxx xxxxxxxx drop
     * xx111111 1xxxxxxx from
     * xxxxxxxx x1111111 to
     */
    namespace move {
        constexpr move_t create(int from, int to) { return from << 7 | to; }
        constexpr move_t create_promote(int from, int to) { return from << 7 | to | 0b1000000000000000; }
        constexpr move_t create_drop(type_t t, int to) { return t << 7 | to | 0b0100000000000000; }
        constexpr int from(move_t m) { return (m >> 7) & 0b01111111; }
        constexpr int to(move_t m) { return m & 0b01111111; }
        constexpr bool is_promote(move_t m) { return (m & 0b1000000000000000) != 0; }
        constexpr bool is_drop(move_t m) { return (m & 0b0100000000000000) != 0; }
    }

    /**
     * Direction
     * 1111111x value
     * xxxxxxx1 fly
     */
    namespace dir {
        constexpr dir_t N =  -1 * 2; //  -1 << 1
        constexpr dir_t E = -10 * 2; // -10 << 1
        constexpr dir_t W = +10 * 2; // +10 << 1
        constexpr dir_t S =  +1 * 2; //  +1 << 1
        constexpr dir_t NE = N + E;
        constexpr dir_t NW = N + W;
        constexpr dir_t SE = S + E;
        constexpr dir_t SW = S + W;
        constexpr dir_t NNE = N + N + E;
        constexpr dir_t NNW = N + N + W;
        constexpr dir_t FN = N | 1;
        constexpr dir_t FE = E | 1;
        constexpr dir_t FW = W | 1;
        constexpr dir_t FS = S | 1;
        constexpr dir_t FNE = NE | 1;
        constexpr dir_t FNW = NW | 1;
        constexpr dir_t FSE = SE | 1;
        constexpr dir_t FSW = SW | 1;
        constexpr bool is_fly(dir_t d) { return (d & 1) != 0; }
        constexpr int value(dir_t d) { return d >> 1; }
    }

    constexpr int address(int file, int rank) {
        return file * 10 + rank;
    }

    constexpr int file_of(int address) {
        constexpr int16_t a[] = {
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

    constexpr int rank_of(int address) {
        constexpr int16_t a[] = {
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
    int legal_moves(const position& p, move_t* out_moves);
    int capturel_moves(const position& p, move_t* out_moves);
}
