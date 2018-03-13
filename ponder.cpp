#include "tenuki.h"

namespace tenuki {

    namespace {
        int search(const position& p, int depth, move_t prev, move_t& out_move);
        int alphabeta(const position& p, int depth, int a, int b);
        int quies(const position& p, int depth, int a, int b);
    }

    move_t ponder(const position& p) {
        boost::timer t;
        std::vector<std::tuple<move_t, int>> moves;

        // move_t m = 0;
        // return search(p, 5, m, m);

        move_t m = 0;
        for (int depth = 1; t.elapsed() < 1.0; depth++) {
            int score = search(p, depth, m, m);
            moves.push_back(std::make_tuple(m, score));
        }
        for (int i = moves.size() - 1; i >= 0; i--) {
            if ((p.side_to_move == side::BLACK) ? std::get<1>(moves[i]) > -15000 : std::get<1>(moves[i]) < 15000) {
                return std::get<0>(moves[i]);
            }
        }
        return std::get<0>(moves[0]);
    }

    namespace {

        std::random_device seed_gen;

        int search(const position& p, int depth, move_t prev, move_t& out_move) {

            static std::mt19937 gen(seed_gen());

            move_t moves[593];
            int length = legal_moves(p, moves);
            if (length == 0) {
                return 0;
            }
            std::shuffle(&moves[0], &moves[length - 1], gen);
            if (prev != 0) {
                std::swap(moves[0], *std::find(&moves[0], &moves[length - 1], prev));
            }

            int a = std::numeric_limits<int>::min();
            int b = std::numeric_limits<int>::max();
            std::cerr << depth << ": ";
            if (p.side_to_move == side::BLACK) {
                // maxノード
                for (int i = 0; i < length; i++) {
                    int score = alphabeta(do_move(p, moves[i]), depth - 1, a, b);
                    if (score > a) {
                        a = score;
                        out_move = moves[i];
                        std::cerr << to_string(moves[i], p) << "(" << score <<") ";
                    }
                }
            } else {
                // minノード
                for (int i = 0; i < length; i++) {
                    int score = alphabeta(do_move(p, moves[i]), depth - 1, a, b);
                    if (score < b) {
                        b = score;
                        out_move = moves[i];
                        std::cerr << to_string(moves[i], p) << "(" << score <<") ";
                    }
                }
            }
            std::cerr << "\n";
            return (p.side_to_move == side::BLACK) ? a : b;
        }

        /**
         * alphabeta
         * @param p
         * @param depth
         * @param a 探索済みminノードの最大値
         * @param b 探索済みmaxノードの最小値
         */
        int alphabeta(const position& p, int depth, int a, int b) {

            if (depth <= 0) {
                return quies(p, 4, a, b);
                //return static_value(p);
            }

            move_t moves[593];
            int length = legal_moves(p, moves);
            if (length == 0) {
                return static_value(p);
            }

            if (p.side_to_move == side::BLACK) {
                // maxノード
                for (int i = 0; i < length; i++) {
                    a = std::max(a, alphabeta(do_move(p, moves[i]), depth - 1, a, b));
                    if (a >= b) {
                        return b; // bカット
                    }
                }
                return a;
            } else {
                // minノード
                for (int i = 0; i < length; i++) {
                    b = std::min(b, alphabeta(do_move(p, moves[i]), depth - 1, a, b));
                    if (a >= b) {
                        return a; // aカット
                    }
                }
                return b;
            }
        }

        int quies(const position& p, int depth, int a, int b) {

            int standpat = static_value(p);
            if (depth == 0) {
                return standpat;
            }
            move_t moves[128];

            if (p.side_to_move == side::BLACK) {
                if (b <= standpat) {
                    return b;
                }
                if (a < standpat) {
                    a = standpat;
                }
                int length = capturel_moves(p, moves);
                for (int i = 0; i < length; i++) {
                    int value = quies(do_move(p, moves[i]), depth - 1, a, b);
                    if (b <= value) {
                        return b;
                    }
                    if (a < value) {
                        a = value;
                    }
                }
                return a;
            } else {
                if (a >= standpat) {
                    return a;
                }
                if (b > standpat) {
                    b = standpat;
                }
                int length = capturel_moves(p, moves);
                for (int i = 0; i < length; i++) {
                    int value = quies(do_move(p, moves[i]), depth - 1, a, b);
                    if (a >= value) {
                        return a;
                    }
                    if (b > value) {
                        b = value;
                    }
                }
                return b;
            }
        }

    }

}
