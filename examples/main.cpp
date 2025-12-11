// 避免 Windows min/max 宏污染
#include <string>
#ifdef _WIN32
#define NOMINMAX
#endif

#include <cstdlib>
#include <fstream>
#include <ghc/filesystem.hpp>  // 或 std::filesystem
#include <iostream>
#include <vector>

#include "zip_compress/zip_reader.h"
#include "zip_compress/zip_writer.h"

namespace fs = ghc::filesystem;

// 测试压缩大文件
void test_zip_compression()
{
  const std::string big_file = "big_file.dat";
  const size_t file_size = 100 * 1024 * 1024;  // 100MB

  // 生成测试大文件
  std::ofstream out(big_file, std::ios::binary);
  std::vector<char> buffer(1024 * 1024, 0);  // 1MB buffer
  for (size_t written = 0; written < file_size; written += buffer.size())
  {
    size_t to_write = std::min(buffer.size(), file_size - written);
    out.write(buffer.data(), to_write);
  }
  out.close();
  std::cout << "Generated test file: " << big_file << std::endl;

  // 压缩文件
  try
  {
    zip_compress::ZipWriter writer("test.zip");
    writer.add_file(big_file);  // 只压缩单个大文件
    writer.finish();
    std::cout << "ZIP created successfully" << std::endl;
  }
  catch (const std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

// 测试递归压缩多级文件夹
void test_compress_multilevel_folder(const std::string& folder_path, const std::string& zip_path)
{
  // 创建多级文件夹结构
  fs::create_directories(folder_path + "/subdir1");
  fs::create_directories(folder_path + "/subdir2/nested");

  // 根目录文件
  for (int i = 0; i < 3; ++i)
  {
    std::ofstream out(fs::path(folder_path) / ("file_root_" + std::to_string(i) + ".dat"), std::ios::binary);
    std::vector<char> buffer(512 * 1024, i);  // 0.5MB
    out.write(buffer.data(), buffer.size());
  }

  // subdir1 文件
  for (int i = 0; i < 2; ++i)
  {
    std::ofstream out(fs::path(folder_path) / "subdir1" / ("file_sub1_" + std::to_string(i) + ".dat"),
                      std::ios::binary);
    std::vector<char> buffer(256 * 1024, i + 10);  // 0.25MB
    out.write(buffer.data(), buffer.size());
  }

  // subdir2/nested 文件
  for (int i = 0; i < 2; ++i)
  {
    std::ofstream out(fs::path(folder_path) / "subdir2/nested" / ("file_nested_" + std::to_string(i) + ".dat"),
                      std::ios::binary);
    std::vector<char> buffer(128 * 1024, i + 20);  // 0.125MB
    out.write(buffer.data(), buffer.size());
  }

  try
  {
    zip_compress::ZipWriter writer(zip_path);

    // 遍历文件夹，递归添加所有文件，保持相对路径
    for (auto& entry : fs::recursive_directory_iterator(folder_path))
    {
      if (fs::is_regular_file(entry))
      {
        writer.add_file(entry.path().string(), folder_path);  // 第二参数是基准路径
      }
    }

    writer.finish();
    std::cout << "Multi-level folder ZIP created successfully: " << zip_path << std::endl;
  }
  catch (const std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}

// ------------------ 解压测试 ------------------
void test_zip_extraction()
{
  try
  {
    zip_compress::ZipReader reader("test.zip");
    std::cout << "\n--- test.zip ---" << std::endl;
    for (const auto& f : reader.file_list()) std::cout << f << std::endl;

    const std::string extract_folder = "extract_test";
    reader.extract_all(extract_folder);
    std::cout << "Extracted all files to: " << extract_folder << std::endl;

    // 解压单个文件
    const auto& files = reader.file_list();
    if (!files.empty())
    {
      const std::string single_file = files[0];
      const std::string single_output = "extract_single/" + fs::path(single_file).filename().string();
      reader.extract_file(single_file, single_output);
      std::cout << "Extracted single file: " << single_output << std::endl;
    }

    // 测试文件夹 ZIP 解压
    zip_compress::ZipReader folder_reader("folder_test.zip");
    std::cout << "\n--- folder_test.zip ---" << std::endl;
    for (const auto& f : folder_reader.file_list()) std::cout << f << std::endl;
    const std::string folder_extract = "extract_folder_test";
    folder_reader.extract_all(folder_extract);
    std::cout << "Extracted folder ZIP to: " << folder_extract << std::endl;
  }
  catch (const std::exception& e)
  {
    std::cerr << "Extraction exception: " << e.what() << std::endl;
  }
}

void test_update_package()
{
  std::string zip_path = "upgrade_package.zip";
  std::string extract_folder = "./upgrade_extracted";
  fs::create_directories(extract_folder);
  try
  {
    zip_compress::ZipReader reader(zip_path);
    reader.extract_all(extract_folder);
    std::cout << "Upgrade package extracted to: " << extract_folder << std::endl;
  }
  catch (const std::exception& e)
  {
    std::cerr << "!!!!! Update package extraction exception: " << e.what() << std::endl;
  }
}


int main()
{
#if defined(_WIN32)
  std::system("chcp 65001 > nul");  // 切换到 UTF-8 编码（仅 Windows）
#endif
  std::cout << u8"你好，世界！" << std::endl;
  // test_zip_compression();
  // test_compress_multilevel_folder("test_folder", "folder_test.zip");
  // test_zip_extraction();
  test_update_package();
  return 0;
}
