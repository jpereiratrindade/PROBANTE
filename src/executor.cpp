#include "executor.hpp"
#include <array>
#include <chrono>
#include <csignal>
#include <fcntl.h>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace {
constexpr std::size_t limit=65536;
std::vector<char*> ptrs(std::vector<std::string>& v){ std::vector<char*> p; for(auto& s:v)p.push_back(s.data()); p.push_back(nullptr); return p; }
void drain(int fd,std::string& s,bool& open,bool& exceeded){ std::array<char,4096>b{}; for(;;){auto n=read(fd,b.data(),b.size()); if(n>0){auto z=static_cast<std::size_t>(n); if(s.size()+z>limit){auto keep=limit-s.size();s.append(b.data(),keep);exceeded=true;}else s.append(b.data(),z);} else {if(n==0){close(fd);open=false;} break;}} }
}

bool isolation_available(std::string& detail){
  pid_t p=fork(); if(p<0){detail="fork failed";return false;} if(p==0){
    int dn=open("/dev/null",O_RDWR); dup2(dn,1);dup2(dn,2);
    char* const a[]={const_cast<char*>("bwrap"),const_cast<char*>("--unshare-user"),const_cast<char*>("--unshare-pid"),const_cast<char*>("--unshare-net"),const_cast<char*>("--unshare-uts"),const_cast<char*>("--unshare-ipc"),const_cast<char*>("--ro-bind"),const_cast<char*>("/usr"),const_cast<char*>("/usr"),const_cast<char*>("--ro-bind"),const_cast<char*>("/lib64"),const_cast<char*>("/lib64"),const_cast<char*>("--proc"),const_cast<char*>("/proc"),const_cast<char*>("--dev"),const_cast<char*>("/dev"),const_cast<char*>("/usr/bin/true"),nullptr};
    execvp("bwrap",a);_exit(127);
  } int st=0;if(waitpid(p,&st,0)<0){detail="waitpid failed";return false;} if(!WIFEXITED(st)||WEXITSTATUS(st)!=0){detail="required bwrap namespaces unavailable";return false;} detail="bwrap user,pid,mount,network namespaces established";return true;
}

Execution execute_isolated(const std::filesystem::path& snapshot,const TestCase& tc,int timeout_ms){
  Execution r; int op[2],ep[2],xp[2]; if(pipe2(op,O_CLOEXEC|O_NONBLOCK)||pipe2(ep,O_CLOEXEC|O_NONBLOCK)||pipe2(xp,O_CLOEXEC)){r.error="pipe failed";return r;}
  auto start=std::chrono::steady_clock::now(); pid_t pid=fork(); if(pid<0){r.error="fork failed";return r;}
  if(pid==0){
    setpgid(0,0); close(op[0]);close(ep[0]);close(xp[0]); fcntl(xp[1],F_SETFD,FD_CLOEXEC);
    int dn=open("/dev/null",O_RDONLY);dup2(dn,0);dup2(op[1],1);dup2(ep[1],2);close(op[1]);close(ep[1]);
    auto sentinel=(snapshot.parent_path()/"isolation-sentinel").string();
    std::vector<std::string> a={"bwrap","--die-with-parent","--unshare-user","--unshare-pid","--unshare-net","--unshare-uts","--unshare-ipc","--new-session","--clearenv","--setenv","LC_ALL","C","--ro-bind","/usr","/usr","--ro-bind","/lib64","/lib64","--proc","/proc","--dev","/dev","--tmpfs","/tmp","--tmpfs","/run","--dir","/work","--dir","/probe","--ro-bind",snapshot.string(),"/work/target","--ro-bind",sentinel,"/probe/host-sentinel","--chdir","/work","/work/target"};
    a.insert(a.end(),tc.arguments.begin(),tc.arguments.end()); auto av=ptrs(a); execvp("bwrap",av.data()); int e=errno; (void)!write(xp[1],&e,sizeof(e)); _exit(127);
  }
  close(op[1]);close(ep[1]);close(xp[1]); setpgid(pid,pid); bool oo=true,eo=true,done=false;int st=0;
  while(!done || oo || eo){
    auto now=std::chrono::steady_clock::now(); auto ms=std::chrono::duration_cast<std::chrono::milliseconds>(now-start).count();
    if(!done && ms>=timeout_ms){r.timed_out=true;kill(-pid,SIGKILL);kill(pid,SIGKILL);}
    pollfd fds[2]={{op[0],POLLIN|POLLHUP,0},{ep[0],POLLIN|POLLHUP,0}}; poll(fds,2,20);
    if(oo && fds[0].revents)drain(op[0],r.out,oo,r.output_limit);
    if(eo && fds[1].revents)drain(ep[0],r.err,eo,r.output_limit);
    if(r.output_limit&&!done){kill(-pid,SIGKILL);kill(pid,SIGKILL);}
    if(!done){auto w=waitpid(pid,&st,WNOHANG);if(w==pid)done=true;}
  }
  int ee=0;auto xn=read(xp[0],&ee,sizeof(ee));close(xp[0]);r.started=(xn==0); if(xn>0)r.error="exec bwrap failed errno="+std::to_string(ee);
  if(WIFEXITED(st))r.exit_code=WEXITSTATUS(st);else if(WIFSIGNALED(st))r.exit_code=128+WTERMSIG(st);
  r.duration_ms=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-start).count();return r;
}
