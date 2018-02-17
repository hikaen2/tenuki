#include "tenuki.h"

namespace tenuki {

  using std::pair;
  using std::vector;

  namespace {

    /**
     * alphabeta
     */
    int alphabeta(const position& p, int depth, int a, int b) {

      if (depth <= 0) {
        return p.static_value;
      }

      vector<move_t> ms = legal_moves(p);
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


    std::random_device seed_gen;

    /**
     * search
     */
    const vector<pair<move_t, int>> search(const position& p, int depth) {

      static std::mt19937 gen(seed_gen());

      vector<pair<move_t, int>> scores;

      vector<move_t> ms = legal_moves(p);
      for (auto&& m : ms) {
        int value = alphabeta(do_move(p, m), depth, std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
        scores.push_back(std::make_pair(m, value));
      }

      std::shuffle(scores.begin(), scores.end(), gen);

      if (p.side_to_move == side::BLACK) {
        std::stable_sort(scores.begin(), scores.end(), [](pair<move_t, int>a, pair<move_t, int> b){ return a.second > b.second; });
      } else {
        std::stable_sort(scores.begin(), scores.end(), [](pair<move_t, int>a, pair<move_t, int> b){ return a.second < b.second; });
      }
      return scores;
    }

  }


  /**
   * ponder
   */
  move_t ponder(const position& p) {
    vector<pair<move_t, int>> move;
    boost::timer t;
    for (int depth = 0; t.elapsed() < 1.0; depth++) {
      move = search(p, depth);
      std::cerr << depth << ": ";
      for (size_t i = 0; i < move.size(); i++) {
        std::cerr << to_string(move[i].first, p) << "(" << move[i].second <<") ";
      }
      std::cerr << "\n";
    }
    return move[0].first;
  }

}
