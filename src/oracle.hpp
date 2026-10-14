#pragma once
#include "contract.hpp"
#include "executor.hpp"
#include <string>

struct Decision { bool pass=false; std::string reason; };
Decision decide(const TestCase& tc,const Execution& actual);

