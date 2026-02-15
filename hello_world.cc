#include <iostream>
#include <string>
#include <vector>

#include "absl/strings/str_join.h"
#include "rte_eal.h"

int main(int argc, char **argv) {
  std::vector<std::string> v = {"foo", "bar", "baz"};
  std::string s = absl::StrJoin(v, "-");

  std::cout << "Joined string: " << s << "\n";
  rte_eal_init(argc, argv);

  return 0;
}
