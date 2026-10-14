#include <charconv>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <limits>
#include <string_view>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifndef FIXTURE_MODE
#define FIXTURE_MODE 0
#endif

[[maybe_unused]] static bool parse(std::string_view s,std::int64_t& n){if(s.empty())return false;auto [p,e]=std::from_chars(s.data(),s.data()+s.size(),n);return e==std::errc{}&&p==s.data()+s.size();}
int main(int argc,char**argv){
  (void)argc;(void)argv;
#if FIXTURE_MODE == 5
  auto child=fork();if(child==0){sleep(30);return 0;}std::cerr<<"child="<<child<<"\n";sleep(30);return 0;
#elif FIXTURE_MODE == 7
  for(int i=0;i<70000;i++){std::cout.put('X');}return 0;
#elif FIXTURE_MODE == 8
  __builtin_trap();
#else
#if FIXTURE_MODE == 6
  {std::ofstream host_tmp("/tmp/probante-m06-sentinel",std::ios::trunc);host_tmp<<"ALTERED\n";}
  {std::ofstream sentinel("/probe/host-sentinel",std::ios::trunc);if(sentinel){sentinel<<"ALTERED\n";std::cerr<<"ISOLATION_WRITE_BREACH\n";return 9;}}
  int sock=socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK,0);if(sock>=0){sockaddr_in dst{};dst.sin_family=AF_INET;dst.sin_port=htons(53);inet_pton(AF_INET,"1.1.1.1",&dst.sin_addr);if(connect(sock,reinterpret_cast<sockaddr*>(&dst),sizeof(dst))==0){close(sock);std::cerr<<"NETWORK_BREACH\n";return 9;}close(sock);}
#endif
  if(argc!=3){
#if FIXTURE_MODE == 3
    std::cout<<"0\n";return 0;
#else
    std::cerr<<"INVALID_INPUT\n";return 2;
#endif
  }
  std::int64_t a=0,b=0;if(!parse(argv[1],a)||!parse(argv[2],b)){
#if FIXTURE_MODE == 3
    std::cout<<"0\n";return 0;
#else
    std::cerr<<"INVALID_INPUT\n";return 2;
#endif
  }
#if FIXTURE_MODE == 1
  if(a==7&&b==5){std::cout<<(a-b)<<"\n";return 0;}
#endif
#if FIXTURE_MODE == 2
  auto u=static_cast<std::uint64_t>(a)+static_cast<std::uint64_t>(b);std::cout<<static_cast<std::int64_t>(u)<<"\n";return 0;
#else
  if((b>0&&a>std::numeric_limits<std::int64_t>::max()-b)||(b<0&&a<std::numeric_limits<std::int64_t>::min()-b)){std::cerr<<"OVERFLOW\n";return 3;}
  auto result=a+b;
#if FIXTURE_MODE == 4
  std::cout<<result<<"\nextra\n";
#else
  std::cout<<result<<"\n";
#endif
  return 0;
#endif
#endif
}
