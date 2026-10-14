#include "oracle.hpp"
Decision decide(const TestCase& tc,const Execution& a){
  if(!a.started)return {false,"PROCESS_START_FAILED"};
  if(a.output_limit)return {false,"OUTPUT_LIMIT_EXCEEDED"};
  if(a.timed_out)return {false,"TIMEOUT"};
  if(a.exit_code!=tc.expected.exit_code)return {false,"EXIT_CODE_MISMATCH"};
  if(a.out!=tc.expected.out)return {false,"STDOUT_MISMATCH"};
  if(a.err!=tc.expected.err)return {false,"STDERR_MISMATCH"};
  return {true,"EXACT_MATCH"};
}

