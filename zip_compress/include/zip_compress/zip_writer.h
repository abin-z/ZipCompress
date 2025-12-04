// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Abin
/**
 * @file zip_writer.h
 * @brief ZIP 文件压缩类
 * @author abin
 * @date 2025-12-04
 */

#ifndef __GUARD_ZIP_WRITER_H_INCLUDE_GUARD__
#define __GUARD_ZIP_WRITER_H_INCLUDE_GUARD__

#include <string>

#include "miniz.h"

namespace zip_compress
{

class ZipWriter
{
 public:
  explicit ZipWriter(const std::string& zip_path);
  ~ZipWriter();

  // 添加单个文件
  void add_file(const std::string& file_path, const std::string& base_path = "");

  // 添加整个文件夹（递归）
  void add_folder(const std::string& folder_path);

  // 完成压缩（析构会自动调用）
  void finish();

 private:
  mz_zip_archive zip_;
  bool finished_;
};

}  // namespace zip_compress

#endif  // __GUARD_ZIP_WRITER_H_INCLUDE_GUARD__
