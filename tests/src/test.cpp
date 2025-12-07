#define CATCH_CONFIG_MAIN
#include <algorithm>
#include <catch2/catch.hpp>
#include <cstdio>  // std::remove
#include <fstream>
#include <vector>

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
static void write_file(const fs::path& path, const std::string& content)
{
  std::ofstream ofs(path.string(), std::ios::binary);
  REQUIRE(ofs.is_open());
  ofs << content;
}

// 工具函数：读文件
static std::string read_file(const fs::path& path)
{
  std::ifstream ifs(path.string(), std::ios::binary);
  REQUIRE(ifs.is_open());
  return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

// 工具函数：把路径统一为 ZIP 内部格式 ("/")
static std::string zip_path(const fs::path& p)
{
  std::string s = p.generic_string();  // fs::path::generic_string() 用 "/" 分隔
  return s;
}

TEST_CASE("ZipWriter + ZipReader cross-platform tests")
{
  const fs::path zip_path_file = "test.zip";
  const fs::path tmp_dir = "tmp_test";

  fs::create_directories(tmp_dir);

  // 基础文件
  write_file(tmp_dir / "a.txt", "AAA");
  write_file(tmp_dir / "b.txt", "BBB");

  // ★★★★★ 生成统一 ZIP，供所有 SECTION 使用
  {
    ZipWriter writer(zip_path_file.string());

    // 添加文件
    writer.add_file((tmp_dir / "a.txt").string());
    writer.add_file((tmp_dir / "b.txt").string());

    // 内存数据
    const char mem_data[] = "hello mem!";
    writer.add_data("mem.txt", mem_data, sizeof(mem_data) - 1);
  }  // writer 析构完成写入

  SECTION("File list")
  {
    ZipReader reader(zip_path_file.string());
    auto files = reader.file_list();

    // 统一路径
    for (auto& f : files) std::replace(f.begin(), f.end(), '\\', '/');

    REQUIRE(files.size() == 3);
    REQUIRE(std::find(files.begin(), files.end(), "a.txt") != files.end());
    REQUIRE(std::find(files.begin(), files.end(), "b.txt") != files.end());
    REQUIRE(std::find(files.begin(), files.end(), "mem.txt") != files.end());
  }

  SECTION("Extract all files to folder")
  {
    ZipReader reader(zip_path_file.string());
    const fs::path out_dir = "output";
    fs::create_directories(out_dir);

    reader.extract_all(out_dir.string());

    REQUIRE(read_file(out_dir / "a.txt") == "AAA");
    REQUIRE(read_file(out_dir / "b.txt") == "BBB");
    REQUIRE(read_file(out_dir / "mem.txt") == "hello mem!");
  }

  SECTION("Extract file to memory")
  {
    ZipReader reader(zip_path_file.string());
    auto data = reader.extract_file_to_memory("mem.txt");

    REQUIRE(!data.empty());
    REQUIRE(std::string(data.begin(), data.end()) == "hello mem!");
  }

  SECTION("Add and extract empty file")
  {
    const fs::path empty_file = tmp_dir / "empty.txt";
    write_file(empty_file, "");

    {
      ZipWriter writer(zip_path_file.string());
      writer.add_file(empty_file.string());
    }

    ZipReader reader(zip_path_file.string());
    auto data = reader.extract_file_to_memory("empty.txt");
    REQUIRE(data.empty());
  }

  // SECTION("Nested directories")
  // {
  //   fs::path nested = tmp_dir / "a" / "b" / "c";
  //   fs::create_directories(nested);
  //   write_file(nested / "deep.txt", "DEEP");

  //   {
  //     ZipWriter writer(zip_path_file.string());
  //     // 添加 tmp_dir 下所有内容，相对路径写入 ZIP
  //     writer.add_folder(tmp_dir.string(), tmp_dir.string());
  //   }

  //   ZipReader reader(zip_path_file.string());
  //   auto files = reader.file_list();

  //   // normalize
  //   for (auto& f : files) std::replace(f.begin(), f.end(), '\\', '/');

  //   fs::path relative = fs::relative(nested / "deep.txt", tmp_dir);
  //   std::string internal = zip_path(relative);

  //   REQUIRE(std::find(files.begin(), files.end(), internal) != files.end());

  //   auto data = reader.extract_file_to_memory(internal);
  //   REQUIRE(std::string(data.begin(), data.end()) == "DEEP");
  // }

  SECTION("Overwrite during extract_all")
  {
    const fs::path file = tmp_dir / "same.txt";
    write_file(file, "FIRST");

    {
      ZipWriter writer(zip_path_file.string());
      writer.add_file(file.string());
    }

    // 修改原文件
    write_file(file, "SECOND");

    fs::path out = "overwrite_out";
    fs::create_directories(out);
    write_file(out / "same.txt", "OLD");

    ZipReader reader(zip_path_file.string());
    reader.extract_all(out.string());

    REQUIRE(read_file(out / "same.txt") == "FIRST");
  }

  SECTION("Add large data and extract to memory")
  {
    std::vector<uint8_t> large(1 * 1024 * 1024, 0xAB);

    {
      ZipWriter writer(zip_path_file.string());
      writer.add_data("large.bin", large.data(), large.size());
    }

    ZipReader reader(zip_path_file.string());
    auto out = reader.extract_file_to_memory("large.bin");

    REQUIRE(out.size() == large.size());
    REQUIRE(out[0] == 0xAB);
    REQUIRE(out.back() == 0xAB);
  }

  // 清理
  std::remove(zip_path_file.string().c_str());
  std::remove((tmp_dir / "a.txt").string().c_str());
  std::remove((tmp_dir / "b.txt").string().c_str());
}
