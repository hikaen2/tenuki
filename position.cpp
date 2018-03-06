#include "tenuki.h"

using std::map;
using std::string;
using std::vector;

namespace tenuki {

    /**
     * SFENを局面にする
     */
    const position parse_position(const string& sfen) {

        const map<string, square_t> TO_SQUARE {
            {"1",  square::EMPTY             },
            {"P",  square::B_PAWN            },
            {"L",  square::B_LANCE           },
            {"N",  square::B_KNIGHT          },
            {"S",  square::B_SILVER          },
            {"B",  square::B_BISHOP          },
            {"R",  square::B_ROOK            },
            {"G",  square::B_GOLD            },
            {"K",  square::B_KING            },
            {"+P", square::B_PROMOTED_PAWN   },
            {"+L", square::B_PROMOTED_LANCE  },
            {"+N", square::B_PROMOTED_KNIGHT },
            {"+S", square::B_PROMOTED_SILVER },
            {"+B", square::B_PROMOTED_BISHOP },
            {"+R", square::B_PROMOTED_ROOK   },
            {"p",  square::W_PAWN            },
            {"l",  square::W_LANCE           },
            {"n",  square::W_KNIGHT          },
            {"s",  square::W_SILVER          },
            {"b",  square::W_BISHOP          },
            {"r",  square::W_ROOK            },
            {"g",  square::W_GOLD            },
            {"k",  square::W_KING            },
            {"+p", square::W_PROMOTED_PAWN   },
            {"+l", square::W_PROMOTED_LANCE  },
            {"+n", square::W_PROMOTED_KNIGHT },
            {"+s", square::W_PROMOTED_SILVER },
            {"+b", square::W_PROMOTED_BISHOP },
            {"+r", square::W_PROMOTED_ROOK   },
        };

        static const map<string, type_t> TO_TYPE {
            {"P", type::PAWN},
            {"L", type::LANCE},
            {"N", type::KNIGHT},
            {"S", type::SILVER},
            {"B", type::BISHOP},
            {"R", type::ROOK},
            {"G", type::GOLD},
            {"p", type::PAWN},
            {"l", type::LANCE},
            {"n", type::KNIGHT},
            {"s", type::SILVER},
            {"b", type::BISHOP},
            {"r", type::ROOK},
            {"g", type::GOLD},
        };

        // スペースでsplitする
        vector<string> v;
        boost::algorithm::split(v, sfen, boost::is_space());
        if (v.size() != 4) {
            throw std::runtime_error(sfen);
        }
        string board_state = v[0];
        string side_to_move = v[1];
        string pieces_in_hand = v[2];
        string move_count = v[3];

        position p;
        std::fill(std::begin(p.squares), std::end(p.squares), square::WALL);
        std::fill(std::begin(p.pieces_in_hand[side::BLACK]), std::end(p.pieces_in_hand[side::BLACK]), 0);
        std::fill(std::begin(p.pieces_in_hand[side::WHITE]), std::end(p.pieces_in_hand[side::WHITE]), 0);

        // 手番
        if (side_to_move != "b" && side_to_move != "w") {
            throw std::runtime_error(sfen);
        }
        p.side_to_move = side_to_move == "b" ? side::BLACK : side::WHITE;

        // 盤面
        for (int i = 9; i >= 2; i--) {
            boost::algorithm::replace_all(board_state, std::to_string(i), string(i, '1')); // 2～9を1に開いておく
        }
        boost::algorithm::replace_all(board_state, "/", "");
        static const std::regex re("\\+?."); // 例：l, n, s, g, k, p, +p, +P, /
        std::sregex_iterator it(board_state.begin(), board_state.end(), re);
        for (int rank = 1; rank <= 9; rank++) {
            for (int file = 9; file >= 1; file--) {
                p.squares[file * 10 + rank] = TO_SQUARE.at((*it++).str());
            }
        }

        // 持ち駒
        if (pieces_in_hand != "-") {
            static const std::regex re("(\\d*)(\\D)"); // 例：S, 4P, b, 3n, p, 18P
            for (std::sregex_iterator it(pieces_in_hand.begin(), pieces_in_hand.end(), re), end; it != end; ++it) {
                const int num = (*it)[1].length() == 0 ? 1 : stoi((*it)[1].str());
                const string piece = (*it)[2].str();
                p.pieces_in_hand[isupper(piece.at(0)) ? side::BLACK : side::WHITE][TO_TYPE.at(piece)] += num;
            }
        }

        return p;
    }


    /**
     * to_sfen
     */
    const string to_sfen(const position& p) {

        //   歩,  香,  桂,  銀,  角,  飛,  金,  王,   と, 成香, 成桂, 成銀,   馬,   龍,  空, 壁
        static const vector<string> TO_SFEN {
            "P", "L", "N", "S", "B", "R", "G", "K", "+P", "+L", "+N", "+S", "+B", "+R", "1", "",
            "p", "l", "n", "s", "b", "r", "g", "k", "+p", "+l", "+n", "+s", "+b", "+r",
        };

        vector<string> lines;
        for (int rank = 1; rank <= 9; rank++) {
            string line;
            for (int file = 9; file >= 1; file--) {
                line += TO_SFEN.at(p.squares[address(file, rank)]);
            }
            lines.push_back(line);
        }
        string s = boost::algorithm::join(lines, "/");
        for (int i = 9; i >= 2; i--) {
            boost::algorithm::replace_all(s, string(i, '1'), std::to_string(i)); // '1'をまとめる
        }
        return s;
    }

    /**
     * pの静的評価値を返す
     */
    int16_t static_value(const position& p) {

        //   歩,   香,   桂,   銀,   角,   飛,   金,    王,   と, 成香, 成桂, 成銀,   馬,   龍, 空, 壁
        static const int16_t SCORE[] = {
             87,  235,  254,  371,  571,  647,  447,  9999,  530,  482,  500,  489,  832,  955,  0,  0,
            -87, -235, -254, -371, -571, -647, -447, -9999, -530, -482, -500, -489, -832, -955,
        };

        int16_t result = 0;
        for (int i = 11; i <= 99; i++) {
            result += SCORE[p.squares[i]];
        }
        for (int t = type::PAWN; t <= type::ROOK; t++) {
            result += (p.pieces_in_hand[side::BLACK][t] - p.pieces_in_hand[side::WHITE][t]) * SCORE[t];
        }
        return result;
    }

    /**
     * 局面をKI2形式の文字列にする
     */
    const string to_ki2(const position& p) {

        static const vector<string> BOARD {
            " 歩", " 香", " 桂", " 銀", " 角", " 飛", " 金", " 玉", " と", " 杏", " 圭", " 全", " 馬", " 龍", " ・", " 壁",
            "v歩", "v香", "v桂", "v銀", "v角", "v飛", "v金", "v玉", "vと", "v杏", "v圭", "v全", "v馬", "v龍",
        };

        static const vector<string> HAND {
            "歩", "香", "桂", "銀", "角", "飛", "金",
        };

        static const vector<string> NUM {
            "〇", "一", "二", "三", "四", "五", "六", "七", "八", "九", "十", "十一", "十二", "十三", "十四", "十五", "十六", "十七", "十八",
        };

        string hand[2];
        for (side_t s = side::BLACK; s <= side::WHITE; s++) {
            for (type_t t = type::PAWN; t <= type::GOLD; t++) {
                const int n = p.pieces_in_hand[s][t];
                if (n > 0) {
                    hand[s] += HAND.at(t) + (n > 1 ? NUM.at(n) : "") + "　";
                }
            }
        }

        string s;
        s += "後手の持駒：" + (hand[1].empty() ? "なし" : hand[1]) + "\n";
        s += "  ９ ８ ７ ６ ５ ４ ３ ２ １\n";
        s += "+---------------------------+\n";
        for (int rank = 1; rank <= 9; rank++) {
            s += "|";
            for (int file = 9; file >= 1; file--) {
                s += BOARD.at(p.squares[address(file, rank)]);
            }
            s += "|" + NUM.at(rank) + "\n";
        }
        s += "+---------------------------+\n";
        s += "先手の持駒：" + (hand[0].empty() ? "なし" : hand[0]) + "\n";
        return s;
    }


    const string to_string(const position& p) {
        string s;
        s += "static_value: " + std::to_string(static_value(p)) + "\n";
        s += string("side_to_move: ") + (p.side_to_move == side::BLACK ? "side::BLACK" : "side::WHITE") + "\n";
        s += "sfen: " + to_sfen(p) + "\n";
        s += to_ki2(p) + "\n";
        return s;
    }

}
