#include "../tenuki.h"

using namespace tenuki;

int main() {

  position p = parse_position("l6nl/5+P1gk/2np1S3/p1p4Pp/3P2Sp1/1PPb2P1P/P5GS1/R8/LN4bKL w RGgsn5p 1");
  std::cerr << to_string(p) << "\n";
  ponder(p);
  return 0;
}
