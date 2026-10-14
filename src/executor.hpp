#pragma once
#include "contract.hpp"
#include <filesystem>
#include <string>

struct Execution {
  bool started=false, timed_out=false, output_limit=false;
  int exit_code=-1; std::string out, err; long long duration_ms=0; std::string error;
};

bool isolation_available(std::string& detail);
Execution execute_isolated(const std::filesystem::path& snapshot, const TestCase& tc, int timeout_ms);

