// 避免 Windows min/max 宏污染
#ifdef _WIN32
#define NOMINMAX
#endif

#include "zip_compress/zip_reader.h"

#include <stdexcept>

// 引入 filesystem 库, 优先使用标准库, 其次使用 ghc::filesystem 作为替代
#if defined(_MSC_VER)
#if _MSVC_LANG >= 201703L && __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include "ghc/filesystem.hpp"
namespace fs = ghc::filesystem;
#endif
#else
#if __cplusplus >= 201703L && __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include "ghc/filesystem.hpp"
namespace fs = ghc::filesystem;
#endif
#endif

namespace zip_compress
{

ZipReader::ZipReader(const std::string& zip_path) : opened_(false)
{
  zip_ = {};
  if (!mz_zip_reader_init_file(&zip_, zip_path.c_str(), 0))
  {
    throw std::runtime_error("Failed to open ZIP file: " + zip_path);
  }
  opened_ = true;
}

ZipReader::~ZipReader()
{
  if (opened_)
  {
    mz_zip_reader_end(&zip_);
    opened_ = false;
  }
}

std::vector<std::string> ZipReader::file_list() const
{
  if (!opened_) throw std::runtime_error("ZIP file not opened");

  mz_uint num_files = mz_zip_reader_get_num_files(const_cast<mz_zip_archive*>(&zip_));
  std::vector<std::string> files;
  files.reserve(num_files);

  for (mz_uint i = 0; i < num_files; ++i)
  {
    mz_zip_archive_file_stat stat;
    if (!mz_zip_reader_file_stat(const_cast<mz_zip_archive*>(&zip_), i, &stat))
      throw std::runtime_error("Failed to get file info at index: " + std::to_string(i));

    // 直接使用 ZIP 内路径，转换为本地分隔符
    files.emplace_back(fs::path(stat.m_filename).make_preferred().string());
  }

  return files;
}

void ZipReader::extract_all(const std::string& output_folder)
{
  if (!opened_) throw std::runtime_error("ZIP file not opened");

  fs::create_directories(output_folder);
  mz_uint num_files = mz_zip_reader_get_num_files(&zip_);
  for (mz_uint i = 0; i < num_files; ++i)
  {
    mz_zip_archive_file_stat stat;
    if (!mz_zip_reader_file_stat(&zip_, i, &stat))
      throw std::runtime_error("Failed to get file info at index: " + std::to_string(i));

    fs::path out_path = fs::path(output_folder) / stat.m_filename;
    fs::create_directories(out_path.parent_path());

    if (!mz_zip_reader_extract_to_file(&zip_, i, out_path.string().c_str(), 0))
    {
      throw std::runtime_error("Failed to extract file: " + out_path.string());
    }
  }
}

void ZipReader::extract_file(const std::string& file_name_in_zip, const std::string& output_path)
{
  if (!opened_) throw std::runtime_error("ZIP file not opened");

  int file_index = mz_zip_reader_locate_file(&zip_, file_name_in_zip.c_str(), nullptr, 0);
  if (file_index < 0)
  {
    throw std::runtime_error("File not found in ZIP: " + file_name_in_zip);
  }

  fs::create_directories(fs::path(output_path).parent_path());

  if (!mz_zip_reader_extract_to_file(&zip_, file_index, output_path.c_str(), 0))
  {
    throw std::runtime_error("Failed to extract file: " + output_path);
  }
}

std::vector<uint8_t> ZipReader::extract_file_to_memory(const std::string& file_name_in_zip)
{
  if (!opened_) throw std::runtime_error("ZIP file not opened");

  int file_index = mz_zip_reader_locate_file(&zip_, file_name_in_zip.c_str(), nullptr, 0);
  if (file_index < 0)
  {
    throw std::runtime_error("File not found in ZIP: " + file_name_in_zip);
  }

  mz_zip_archive_file_stat stat;
  if (!mz_zip_reader_file_stat(&zip_, file_index, &stat))
  {
    throw std::runtime_error("Failed to get file info: " + file_name_in_zip);
  }

  std::vector<uint8_t> buffer(stat.m_uncomp_size);
  if (!mz_zip_reader_extract_to_mem(&zip_, file_index, buffer.data(), buffer.size(), 0))
  {
    throw std::runtime_error("Failed to extract file to memory: " + file_name_in_zip);
  }

  return buffer;
}

}  // namespace zip_compress