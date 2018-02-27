#include "tenuki.h"

namespace tenuki {

  using std::pair;
  using std::vector;

  namespace {

    /**
     * alphabeta
     * @param p
     * @param depth
     * @param a 探索済みminノードの最大値
     * @param b 探索済みmaxノードの最小値
     */
    int alphabeta(const position& p, int depth, int a, int b) {

      if (depth <= 0) {
        return static_value(p);
      }

      vector<move_t> ms = legal_moves(p);
      if (ms.size() == 0) {
        return static_value(p);
      }

      if (p.side_to_move == side::BLACK) {
        // maxノード
        for (auto&& m : ms) {
          a = std::max(a, alphabeta(do_move(p, m), depth - 1, a, b));
          if (a >= b) {
            return b; // bカット
          }
        }
        return a;
      } else {
        // minノード
        for (auto&& m : ms) {
          b = std::min(b, alphabeta(do_move(p, m), depth - 1, a, b));
          if (a >= b) {
            return a; // aカット
          }
        }
        return b;
      }
    }

    std::random_device seed_gen;

    /**
     * search
     */
    move_t search(const position& p, int depth, move_t prev) {

      static std::mt19937 gen(seed_gen());

      move_t result = 0;
      vector<move_t> ms = legal_moves(p);
      if (ms.size() == 0) {
        return result;
      }
      std::shuffle(ms.begin(), ms.end(), gen);
      if (prev != 0) {
        std::swap(ms[0], *std::find(ms.begin(), ms.end(), prev));
      }

      int a = std::numeric_limits<int>::min();
      int b = std::numeric_limits<int>::max();
      std::cerr << depth << ": ";
      if (p.side_to_move == side::BLACK) {
        // maxノード
        for (auto&& m : ms) {
          int score = alphabeta(do_move(p, m), depth - 1, a, b);
          if (score > a) {
            a = score;
            result = m;
            std::cerr << to_string(m, p) << "(" << score <<") ";
          }
        }
      } else {
        // minノード
        for (auto&& m : ms) {
          int score = alphabeta(do_move(p, m), depth - 1, a, b);
          if (score < b) {
            b = score;
            result = m;
            std::cerr << to_string(m, p) << "(" << score <<") ";
          }
        }
      }
      std::cerr << "\n";
      return result;
    }

  }


  /**
   * ponder
   */
  move_t ponder(const position& p) {
    move_t m = 0;
    boost::timer t;
    for (int depth = 1; t.elapsed() < 1.0; depth++) {
      m = search(p, depth, m);
    }
    return m;
  }

}
