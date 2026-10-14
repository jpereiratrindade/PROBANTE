#include "evidence.hpp"
#include <openssl/evp.h>
#include <array>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>

std::string sha256(const std::string& b){std::array<unsigned char,EVP_MAX_MD_SIZE>d{};unsigned n=0;auto* c=EVP_MD_CTX_new();if(!c)throw std::runtime_error("EVP context failed");if(EVP_DigestInit_ex(c,EVP_sha256(),nullptr)!=1||EVP_DigestUpdate(c,b.data(),b.size())!=1||EVP_DigestFinal_ex(c,d.data(),&n)!=1){EVP_MD_CTX_free(c);throw std::runtime_error("SHA-256 failed");}EVP_MD_CTX_free(c);std::ostringstream o;for(unsigned i=0;i<n;i++)o<<std::hex<<std::setw(2)<<std::setfill('0')<<static_cast<int>(d[i]);return o.str();}
std::string sha256_file(const std::filesystem::path&p){std::ifstream f(p,std::ios::binary);if(!f)throw std::runtime_error("file hash open failed");return sha256(std::string((std::istreambuf_iterator<char>(f)),{}));}
void write_private(const std::filesystem::path&p,const std::string&b){std::ofstream f(p,std::ios::binary|std::ios::trunc);if(!f)throw std::runtime_error("evidence write failed: "+p.string());f.write(b.data(),static_cast<std::streamsize>(b.size()));f.close();chmod(p.c_str(),0600);std::ifstream check(p,std::ios::binary);std::string actual((std::istreambuf_iterator<char>(check)),{});if(actual!=b)throw std::runtime_error("evidence write verification failed: "+p.string());}
std::string utc_now(){auto t=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());std::tm tm{};gmtime_r(&t,&tm);std::ostringstream o;o<<std::put_time(&tm,"%FT%TZ");return o.str();}
