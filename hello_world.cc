#include <iostream>
#include <string>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_join.h"
#include "rte_eal.h"

// Define command-line flags
ABSL_FLAG(std::string, name, "world", "Name to greet");
ABSL_FLAG(int, count, 1, "Number of times to greet");
ABSL_FLAG(bool, verbose, false, "Enable verbose output");

int main(int argc, char **argv) {
  // Parse command-line flags
  absl::ParseCommandLine(argc, argv);

  // Get flag values
  std::string name = absl::GetFlag(FLAGS_name);
  int count = absl::GetFlag(FLAGS_count);
  bool verbose = absl::GetFlag(FLAGS_verbose);

  if (verbose) {
    std::cout << "Verbose mode enabled\n";
  }

  for (int i = 0; i < count; ++i) {
    std::cout << "Hello, " << name << "!\n";
  }

  std::vector<std::string> v = {"foo", "bar", "baz"};
  std::string s = absl::StrJoin(v, "-");
  std::cout << "Joined string: " << s << "\n";

  rte_eal_init(argc, argv);

  return 0;
}
