#include <iostream>
#include <unordered_map>

enum class Element { H = 1, He, Li, Be };

int main()
{
  std::unordered_map<Element, double> dict = {
    {Element::H, 1.0}, {Element::He, 4.0},
    {Element::Li, 7.0}, {Element::Be, 9.0}
  };
  for (auto && item : dict) {
    std::cout << static_cast<int>(item.first) << ": "
	      << item.second << '\n';
  }
}
