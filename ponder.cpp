#include "tenuki.h"

namespace tenuki {

  using std::tuple;
  using std::vector;

  namespace {

    /**
     * alphabeta
     */
    int alphabeta(const position& p, int depth, int a, int b) {

      if (depth <= 0) {
        return p.static_value;
      }

      vector<move> ms = legal_moves(p);
      if (ms.size() == 0) {
        return p.static_value;
      }

      if (p.side_to_move == side::BLACK) {
        int v = std::numeric_limits<int>::min();
        for (auto&& m : ms) {
          v = std::max(v, alphabeta(do_move(p, m), depth - 1, a, b)); // max(min())
          a = std::max(a, v); 
          if (b <= a) {
            break;
          }
        }
        return v;
      } else {
        int v = std::numeric_limits<int>::max();
        for (auto&& m : ms) {
          v = std::min(v, alphabeta(do_move(p, m), depth - 1, a, b));  // min(max())
          b = std::min(b, v);
          if (b <= a) {
            break;
          }
        }
        return v;
      }
    }


    /**
     * search
     */
    const tuple<move, int> search(const position& p, int depth) {

      static std::mt19937 gen;
      
      vector<tuple<move, int>> scores;

      vector<move> ms = legal_moves(p);
      for (auto&& m : ms) {
        int value = alphabeta(do_move(p, m), depth, std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
        scores.push_back(std::make_tuple(m, value));
      }

      std::shuffle(scores.begin(), scores.end(), gen);
      
      if (p.side_to_move == side::BLACK) {
        return *std::max_element(scores.begin(), scores.end(), [](tuple<move, int>a, tuple<move, int> b){ return std::get<1>(a) < std::get<1>(b); });
      } else {
        return *std::min_element(scores.begin(), scores.end(), [](tuple<move, int>a, tuple<move, int> b){ return std::get<1>(a) < std::get<1>(b); });
      }
    }

  }


  /**
   * ponder
   */
  const move ponder(const position& p) {
    tuple<move, int> move;
    boost::timer t;
    for (int depth = 0; t.elapsed() < 1.0; depth++) {
      move = search(p, depth);
      std::cerr << depth << " " << to_string(std::get<0>(move)) << "(" << std::get<1>(move) <<")\n";
    }
    return std::get<0>(move);
  }

}
