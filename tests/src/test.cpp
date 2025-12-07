#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include <cstdio>  // std::remove
#include <fstream>

// Filesystem fallback
#if __cplusplus < 201703L
#include <ghc/filesystem.hpp>
namespace fs = ghc::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

#include "zip_compress/zip_reader.h"
#include "zip_compress/zip_writer.h"

using namespace zip_compress;

// 工具函数：写文件
static void write_file(const std::string& path, const std::string& content)
{
  std::ofstream ofs(path, std::ios::binary);
  REQUIRE(ofs.is_open());
  ofs << content;
}

// 工具函数：读文件
static std::string read_file(const std::string& path)
{
  std::ifstream ifs(path, std::ios::binary);
  REQUIRE(ifs.is_open());
  return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}
TEST_CASE("ZipWriter + ZipReader basic tests")
{
  const fs::path zip_path = "test.zip";
  const fs::path tmp_dir = "tmp_test";

  fs::create_directories(tmp_dir);

  // 准备文件
  write_file((tmp_dir / "a.txt").string(), "AAA");
  write_file((tmp_dir / "b.txt").string(), "BBB");

  //
  // ★★★★★ 所有 SECTION 公用相同的 ZIP
  //
  {
    ZipWriter writer(zip_path.string());
    writer.add_file((tmp_dir / "a.txt").string());
    writer.add_file((tmp_dir / "b.txt").string());
    const char mem_data[] = "hello mem!";
    writer.add_data("mem.txt", mem_data, sizeof(mem_data) - 1);
  }  // writer 退出 → zip 写入完成

  SECTION("ZipReader file list")
  {
    ZipReader reader(zip_path.string());
    auto files = reader.file_list();

    REQUIRE(files.size() == 3);
    REQUIRE(std::find(files.begin(), files.end(), "a.txt") != files.end());
    REQUIRE(std::find(files.begin(), files.end(), "b.txt") != files.end());
    REQUIRE(std::find(files.begin(), files.end(), "mem.txt") != files.end());
  }

  SECTION("Extract to folder")
  {
    ZipReader reader(zip_path.string());

    const fs::path out_dir = "output";
    fs::create_directories(out_dir);

    reader.extract_all(out_dir.string());

    REQUIRE(read_file((out_dir / "a.txt").string()) == "AAA");
    REQUIRE(read_file((out_dir / "b.txt").string()) == "BBB");
    REQUIRE(read_file((out_dir / "mem.txt").string()) == "hello mem!");
  }

  SECTION("Extract file to memory")
  {
    ZipReader reader(zip_path.string());
    auto data = reader.extract_file_to_memory("mem.txt");
    REQUIRE(!data.empty());
    REQUIRE(std::string(data.begin(), data.end()) == "hello mem!");
  }

  // 清理
  std::remove(zip_path.string().c_str());
  std::remove((tmp_dir / "a.txt").string().c_str());
  std::remove((tmp_dir / "b.txt").string().c_str());
}
