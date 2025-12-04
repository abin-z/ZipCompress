#include "zip_compress/zip_writer.h"

// 避免 Windows min/max 宏污染
#ifdef _WIN32
#define NOMINMAX
#endif

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

ZipWriter::ZipWriter(const std::string& zip_path) : finished_(false)
{
  zip_ = {};
  if (!mz_zip_writer_init_file(&zip_, zip_path.c_str(), 0)) throw std::runtime_error("Failed to create ZIP file");
}

ZipWriter::~ZipWriter()
{
  finish();
}

void ZipWriter::add_file(const std::string& file_path_str, const std::string& base_path_str)
{
  fs::path file_path(file_path_str);
  if (!fs::is_regular_file(file_path)) return;

  fs::path rel_path;
  if (base_path_str.empty())
    rel_path = file_path.filename();
  else
    rel_path = file_path.lexically_relative(base_path_str);

  if (!mz_zip_writer_add_file(&zip_, rel_path.string().c_str(), file_path_str.c_str(), nullptr, 0, MZ_DEFAULT_LEVEL))
  {
    throw std::runtime_error("Failed to add file to ZIP: " + file_path_str);
  }
}

void ZipWriter::add_folder(const std::string& folder_path_str)
{
  fs::path folder_path(folder_path_str);
  if (!fs::exists(folder_path)) throw std::runtime_error("Folder not exist: " + folder_path_str);

  for (auto& entry : fs::recursive_directory_iterator(folder_path))
  {
    if (fs::is_regular_file(entry))
    {
      add_file(entry.path().string(), folder_path_str);
    }
  }
}

void ZipWriter::finish()
{
  if (!finished_)
  {
    mz_zip_writer_finalize_archive(&zip_);
    mz_zip_writer_end(&zip_);
    finished_ = true;
  }
}

}  // namespace zip_compress
