#pragma once
#include <filesystem>
#include <string>
#include <vector>

struct Expected { int exit_code{}; std::string out; std::string err; };
struct TestCase { std::string id; std::string description; std::vector<std::string> arguments; Expected expected; };
struct Contract {
  std::string schema_version, id, version, description, scope;
  std::vector<std::string> preconditions;
  int timeout_ms{};
  std::vector<TestCase> cases;
  std::string raw, sha256;
};

Contract load_contract(const std::filesystem::path& path);

