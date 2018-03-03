#include "tenuki.h"

using std::map;
using std::string;
using std::vector;

namespace tenuki {

    namespace {

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
        const map<square_t, string> TO_SFEN {
            {square::EMPTY,             "1"},
            {square::B_PAWN,            "P"},
            {square::B_LANCE,           "L"},
            {square::B_KNIGHT,          "N"},
            {square::B_SILVER,          "S"},
            {square::B_BISHOP,          "B"},
            {square::B_ROOK,            "R"},
            {square::B_GOLD,            "G"},
            {square::B_KING,            "K"},
            {square::B_PROMOTED_PAWN,   "+P"},
            {square::B_PROMOTED_LANCE,  "+L"},
            {square::B_PROMOTED_KNIGHT, "+N"},
            {square::B_PROMOTED_SILVER, "+S"},
            {square::B_PROMOTED_BISHOP, "+B"},
            {square::B_PROMOTED_ROOK,   "+R"},
            {square::W_PAWN,            "p"},
            {square::W_LANCE,           "l"},
            {square::W_KNIGHT,          "n"},
            {square::W_SILVER,          "s"},
            {square::W_BISHOP,          "b"},
            {square::W_ROOK,            "r"},
            {square::W_GOLD,            "g"},
            {square::W_KING,            "k"},
            {square::W_PROMOTED_PAWN,   "+p"},
            {square::W_PROMOTED_LANCE,  "+l"},
            {square::W_PROMOTED_KNIGHT, "+n"},
            {square::W_PROMOTED_SILVER, "+s"},
            {square::W_PROMOTED_BISHOP, "+b"},
            {square::W_PROMOTED_ROOK,   "+r"},
        };

        const map<square_t, string> TO_KI2 {
            {square::EMPTY,             " ・"},
            {square::B_PAWN,            " 歩"},
            {square::B_LANCE,           " 香"},
            {square::B_KNIGHT,          " 桂"},
            {square::B_SILVER,          " 銀"},
            {square::B_BISHOP,          " 角"},
            {square::B_ROOK,            " 飛"},
            {square::B_GOLD,            " 金"},
            {square::B_KING,            " 玉"},
            {square::B_PROMOTED_PAWN,   " と"},
            {square::B_PROMOTED_LANCE,  " 杏"},
            {square::B_PROMOTED_KNIGHT, " 圭"},
            {square::B_PROMOTED_SILVER, " 全"},
            {square::B_PROMOTED_BISHOP, " 馬"},
            {square::B_PROMOTED_ROOK,   " 龍"},
            {square::W_PAWN,            "v歩"},
            {square::W_LANCE,           "v香"},
            {square::W_KNIGHT,          "v桂"},
            {square::W_SILVER,          "v銀"},
            {square::W_BISHOP,          "v角"},
            {square::W_ROOK,            "v飛"},
            {square::W_GOLD,            "v金"},
            {square::W_KING,            "v玉"},
            {square::W_PROMOTED_PAWN,   "vと"},
            {square::W_PROMOTED_LANCE,  "v杏"},
            {square::W_PROMOTED_KNIGHT, "v圭"},
            {square::W_PROMOTED_SILVER, "v全"},
            {square::W_PROMOTED_BISHOP, "v馬"},
            {square::W_PROMOTED_ROOK,   "v龍"},
        };

        static const map<type_t, string> TO_HAND {
            {type::PAWN,   "歩"},
            {type::LANCE,  "香"},
            {type::KNIGHT, "桂"},
            {type::SILVER, "銀"},
            {type::BISHOP, "角"},
            {type::ROOK,   "飛"},
            {type::GOLD,   "金"},
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

        const map<int, string> TO_NUM {
            {1,  "一"},
            {2,  "二"},
            {3,  "三"},
            {4,  "四"},
            {5,  "五"},
            {6,  "六"},
            {7,  "七"},
            {8,  "八"},
            {9,  "九"},
            {10, "十"},
            {11, "十一"},
            {12, "十二"},
            {13, "十三"},
            {14, "十四"},
            {15, "十五"},
            {16, "十六"},
            {17, "十七"},
            {18, "十八"},
        };

    } // namespace


    /**
     * SFENを局面にする
     */
    const position parse_position(const string& sfen) {

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

        // 盤面の駒をパースする
        vector<vector<string>> vv(9, vector<string>(9));
        {
            for (int i = 9; i >= 2; i--) {
                boost::algorithm::replace_all(board_state, std::to_string(i), string(i, '1')); // 2～9を1に展開しておく
            }
            int rank = 0;
            int file = 8;
            static const std::regex re("\\+?."); // 例：l, n, s, g, k, p, +p, +P, /
            for (std::sregex_iterator it(board_state.begin(), board_state.end(), re), end; it != end; ++it) {
                const string& token = (*it).str();
                if (token == "/") {
                    rank++;
                    file = 8;
                } else {
                    vv.at(file--).at(rank) = token;
                }
            }
        }

        position p;
        for (int i = 0; i < 111; i++) {
            p.squares[i] = square::WALL;
        }

        p.side_to_move = side_to_move == "b" ? side::BLACK : side::WHITE;
        for (int rank = 0; rank <= 8; rank++) {
            for (int file = 0; file <= 8; file++) {
                p.squares[address(file+1, rank+1)] = TO_SQUARE.at(vv[file][rank]);
            }
        }

        // 持ち駒をパースする
        for (side_t s = side::BLACK; s <= side::WHITE; s++) {
            for (type_t t = type::PAWN; t <= type::KING; t++) {
                p.pieces_in_hand[s][t] = 0;
            }
        }
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

        static const int16_t SCORE[] = {
          // 歩,   香,   桂,   銀,   角,   飛,   金,    王,   と, 成香, 成桂, 成銀,   馬,   龍
             87,  235,  254,  371,  571,  647,  447,  9999,  530,  482,  500,  489,  832,  955, 0, 0,
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

        string hand[2];
        for (side_t s = side::BLACK; s <= side::WHITE; s++) {
            for (type_t t = type::PAWN; t <= type::GOLD; t++) {
                const int n = p.pieces_in_hand[s][t];
                if (n > 0) {
                    hand[s] += TO_HAND.at(t) + (n > 1 ? TO_NUM.at(n) : "") + "　";
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
                s += TO_KI2.at(p.squares[address(file, rank)]);
            }
            s += "|" + TO_NUM.at(rank) + "\n";
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

} // namespace
