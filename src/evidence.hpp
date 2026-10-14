#pragma once
#include <filesystem>
#include <string>
std::string sha256(const std::string& bytes);
std::string sha256_file(const std::filesystem::path& path);
void write_private(const std::filesystem::path& path,const std::string& bytes);
std::string utc_now();

