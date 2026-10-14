#include "contract.hpp"
#include "evidence.hpp"
#include <boost/json.hpp>
#include <fstream>
#include <set>
#include <stdexcept>

namespace json = boost::json;
namespace {
void exact_keys(const json::object& o, std::initializer_list<const char*> keys, const std::string& where) {
  std::set<std::string> allowed;
  for (auto k : keys) allowed.emplace(k);
  for (auto const& [k, v] : o) { (void)v; if (!allowed.contains(std::string(k))) throw std::runtime_error(where + ": unknown field " + std::string(k)); }
  for (auto k : keys) if (!o.contains(k)) throw std::runtime_error(where + ": missing field " + k);
}
std::string string_at(const json::object& o, const char* key, const std::string& where) {
  auto const& v = o.at(key); if (!v.is_string()) throw std::runtime_error(where + ": " + key + " must be string");
  return std::string(v.as_string());
}
}

Contract load_contract(const std::filesystem::path& path) {
  std::ifstream f(path, std::ios::binary);
  if (!f) throw std::runtime_error("contract cannot be opened");
  std::string raw((std::istreambuf_iterator<char>(f)), {});
  boost::system::error_code ec; auto value = json::parse(raw, ec);
  if (ec || !value.is_object()) throw std::runtime_error("contract is not valid JSON object");
  auto const& o = value.as_object();
  exact_keys(o, {"schema_version","id","version","description","scope","preconditions","timeout_ms","cases"}, "contract");
  Contract c; c.raw=raw; c.sha256=sha256(raw); c.schema_version=string_at(o,"schema_version","contract");
  if (c.schema_version != "1") throw std::runtime_error("unknown schema_version");
  c.id=string_at(o,"id","contract"); c.version=string_at(o,"version","contract");
  c.description=string_at(o,"description","contract"); c.scope=string_at(o,"scope","contract");
  if (c.id.empty() || c.version.empty() || c.scope.empty()) throw std::runtime_error("contract identifiers and scope must not be empty");
  if (!o.at("timeout_ms").is_int64() || o.at("timeout_ms").as_int64()!=2000) throw std::runtime_error("timeout_ms must be integer 2000");
  c.timeout_ms=2000;
  if (!o.at("preconditions").is_array()) throw std::runtime_error("preconditions must be array");
  for (auto const& x:o.at("preconditions").as_array()) { if(!x.is_string()) throw std::runtime_error("precondition must be string"); c.preconditions.emplace_back(x.as_string()); }
  if (!o.at("cases").is_array() || o.at("cases").as_array().empty()) throw std::runtime_error("cases must be non-empty array");
  std::set<std::string> ids;
  for (auto const& cv:o.at("cases").as_array()) {
    if (!cv.is_object()) throw std::runtime_error("case must be object");
    auto const& co=cv.as_object();
    exact_keys(co,{"id","description","arguments","expected"},"case"); TestCase tc;
    tc.id=string_at(co,"id","case"); tc.description=string_at(co,"description","case");
    if(tc.id.empty() || !ids.insert(tc.id).second) throw std::runtime_error("case id empty or duplicate: "+tc.id);
    if(!co.at("arguments").is_array()) throw std::runtime_error("arguments must be array");
    for(auto const& a:co.at("arguments").as_array()){ if(!a.is_string()) throw std::runtime_error("argument must be string"); tc.arguments.emplace_back(a.as_string()); }
    if(!co.at("expected").is_object()) throw std::runtime_error("expected must be object");
    auto const& e=co.at("expected").as_object();
    exact_keys(e,{"exit_code","stdout","stderr"},"expected");
    if(!e.at("exit_code").is_int64()) throw std::runtime_error("expected exit_code must be integer");
    auto n=e.at("exit_code").as_int64(); if(n < 0 || n > 255) throw std::runtime_error("expected exit_code out of range");
    tc.expected.exit_code=static_cast<int>(n); tc.expected.out=string_at(e,"stdout","expected"); tc.expected.err=string_at(e,"stderr","expected"); c.cases.push_back(std::move(tc));
  }
  return c;
}
