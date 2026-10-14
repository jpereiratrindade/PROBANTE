#include "contract.hpp"
#include "evidence.hpp"
#include "executor.hpp"
#include "oracle.hpp"
#include <boost/json.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>

namespace fs=std::filesystem; namespace json=boost::json;
namespace {
constexpr auto limitation="Result limited to this contract, cases, snapshot, execution and observed environment; not certification or global software approval. SHA-256 provides integrity relative to recorded reference, not origin authenticity.";
std::string ident(){auto n=std::chrono::system_clock::now().time_since_epoch().count();std::random_device rd;return std::to_string(n)+"-"+std::to_string(getpid())+"-"+std::to_string(rd());}
void emit(const json::object&o,int code){std::cout<<json::serialize(o)<<'\n';std::cout.flush();_exit(code);}
void invalid(const std::string&m){json::object o{{"schema_version","1"},{"probante_version","0.1.0"},{"verdict","INVALID_REQUEST"},{"cause",m},{"limitation",limitation}};emit(o,3);}
}
int main(int argc,char**argv){
  if(argc<2||std::string(argv[1])!="verify")invalid("usage: probante verify --target PATH --contract PATH --evidence-dir DIR");
  fs::path target,cp,base; bool seen_target=false,seen_contract=false,seen_evidence=false;
  for(int i=2;i<argc;i++){std::string k=argv[i];if((k=="--target"||k=="--contract"||k=="--evidence-dir")&&i+1<argc){auto v=fs::path(argv[++i]);if(k=="--target"){if(seen_target)invalid("duplicate --target");seen_target=true;target=v;}else if(k=="--contract"){if(seen_contract)invalid("duplicate --contract");seen_contract=true;cp=v;}else{if(seen_evidence)invalid("duplicate --evidence-dir");seen_evidence=true;base=v;}}else invalid("unknown or incomplete argument: "+k);}
  if(target.empty()||cp.empty()||base.empty())invalid("all three options are required");
  Contract c;try{c=load_contract(cp);}catch(std::exception const&e){invalid(e.what());}
  std::error_code ec;if(!fs::is_regular_file(target,ec)||access(target.c_str(),X_OK)!=0)invalid("target must be an executable regular file");
  auto rid=ident();fs::path dir=base/rid;try{fs::create_directories(dir/"snapshot");fs::create_directories(dir/"cases");chmod(dir.c_str(),0700);chmod((dir/"snapshot").c_str(),0700);chmod((dir/"cases").c_str(),0700);}catch(std::exception const&e){invalid(std::string("cannot create evidence directory: ")+e.what());}
  auto started=utc_now();fs::path snap=dir/"snapshot/target";fs::path sentinel=dir/"snapshot/isolation-sentinel";try{fs::copy_file(target,snap,fs::copy_options::none);chmod(snap.c_str(),0500);write_private(sentinel,"PROBANTE_SENTINEL\n");chmod(sentinel.c_str(),0400);write_private(dir/"contract.json",c.raw);}catch(std::exception const&e){invalid(std::string("cannot create private snapshots: ")+e.what());}
  std::string th;try{th=sha256_file(snap);}catch(std::exception const&e){invalid(e.what());}
  std::string iso; if(!isolation_available(iso)){
    json::object r{{"schema_version","1"},{"probante_version","0.1.0"},{"request_id",rid},{"verdict","BLOCKED"},{"cause","ISOLATION_UNAVAILABLE"},{"detail",iso},{"limitation",limitation},{"target",json::object{{"snapshot","snapshot/target"},{"sha256",th}}},{"contract",json::object{{"path","contract.json"},{"source_path",fs::absolute(cp).string()},{"sha256",c.sha256}}},{"isolation",json::object{{"engine","bubblewrap"},{"established",false}}},{"cases",json::array{}}};
    auto s=json::serialize(r);write_private(dir/"result.json",s+"\n");json::object m{{"schema_version","1"},{"result",json::object{{"path","result.json"},{"sha256",sha256(s+"\n")}}}};write_private(dir/"manifest.json",json::serialize(m)+"\n");r["evidence_path"]=fs::absolute(dir).string();std::cout<<json::serialize(r)<<'\n';return 2;
  }
  json::array cases,files;files.emplace_back(json::object{{"path","snapshot/target"},{"sha256",th}});files.emplace_back(json::object{{"path","snapshot/isolation-sentinel"},{"sha256",sha256_file(sentinel)}});files.emplace_back(json::object{{"path","contract.json"},{"sha256",c.sha256}});bool all=true;
  for(auto const& tc:c.cases){auto a=execute_isolated(fs::absolute(snap),tc,c.timeout_ms);auto d=decide(tc,a);all&=d.pass;auto stem="cases/"+tc.id;write_private(dir/(stem+".stdout"),a.out);write_private(dir/(stem+".stderr"),a.err);
    json::array args;for(auto const&x:tc.arguments)args.emplace_back(x);
    json::object co{{"id",tc.id},{"arguments",std::move(args)},{"execution_status",a.started?"STARTED":"NOT_STARTED"},{"verdict",d.pass?"PASS":"FAIL"},{"cause",d.reason},{"duration_ms",a.duration_ms},{"timed_out",a.timed_out},{"exit_code",a.exit_code},{"expected",json::object{{"exit_code",tc.expected.exit_code},{"stdout",tc.expected.out},{"stderr",tc.expected.err}}},{"stdout",json::object{{"path",stem+".stdout"},{"size",a.out.size()},{"sha256",sha256(a.out)}}},{"stderr",json::object{{"path",stem+".stderr"},{"size",a.err.size()},{"sha256",sha256(a.err)}}}};cases.emplace_back(std::move(co));files.emplace_back(json::object{{"path",stem+".stdout"},{"sha256",sha256(a.out)}});files.emplace_back(json::object{{"path",stem+".stderr"},{"sha256",sha256(a.err)}});
  }
  bool sentinel_intact=sha256_file(sentinel)==sha256("PROBANTE_SENTINEL\n");bool target_intact=sha256_file(snap)==th;if(!sentinel_intact||!target_intact)all=false;
  std::string aggregate_cause=all?"ALL_CASES_EXACT":!sentinel_intact?"ISOLATION_SENTINEL_CHANGED":!target_intact?"TARGET_SNAPSHOT_CHANGED":"ONE_OR_MORE_CASES_FAILED";
  auto finished=utc_now();json::object r{{"schema_version","1"},{"probante_version","0.1.0"},{"request_id",rid},{"verdict",all?"PASS":"FAIL"},{"cause",aggregate_cause},{"limitation",limitation},{"started_at",started},{"finished_at",finished},{"target",json::object{{"source_path",fs::absolute(target).string()},{"snapshot","snapshot/target"},{"sha256",th},{"snapshot_intact",target_intact}}},{"contract",json::object{{"id",c.id},{"version",c.version},{"path","contract.json"},{"source_path",fs::absolute(cp).string()},{"sha256",c.sha256}}},{"isolation",json::object{{"engine","bubblewrap"},{"established",true},{"namespaces",json::array{"user","pid","mount","network"}},{"network","disabled"},{"root","read-only runtime mounts; private tmpfs /tmp and /run"},{"sentinel",json::object{{"path","snapshot/isolation-sentinel"},{"read_only",true},{"intact",sentinel_intact},{"sha256",sha256_file(sentinel)}}},{"timeout_ms",c.timeout_ms},{"capture_limit_bytes_per_stream",65536}}},{"environment",json::object{{"os","Linux"},{"locale","C"},{"compiler","GCC 16.2.1 observed during qualification"}}},{"cases",std::move(cases)}};
  auto rs=json::serialize(r)+"\n";write_private(dir/"result.json",rs);files.emplace_back(json::object{{"path","result.json"},{"sha256",sha256(rs)}});json::object m{{"schema_version","1"},{"request_id",rid},{"target_sha256",th},{"contract_sha256",c.sha256},{"files",std::move(files)},{"integrity_limitation","Hashes require a trusted reference; no signature or origin claim."}};write_private(dir/"manifest.json",json::serialize(m)+"\n");r["evidence_path"]=fs::absolute(dir).string();std::cout<<json::serialize(r)<<'\n';return all?0:1;
}
