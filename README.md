# zip_compress

**一个基于 miniz 的轻量级 ZIP 压缩/解压库**

### ✨ 特性

- **支持 ZIP 文件的创建、写入、解压**
- **支持递归添加文件夹**
- **支持将内存数据作为文件写入 ZIP**
- **支持解压到文件或内存**
- **跨平台（Windows / Linux / MacOS）**
- **异常安全 + RAII 管理**

### 🔧 依赖

| C++ 标准          | 需求                   |
| ----------------- | ---------------------- |
| **C++17+**        | 无依赖，可直接使用     |
| **C++11 / C++14** | 需要 `ghc::filesystem` |

### 🚀 快速上手

#### 1. 引入项目

将 `zip_compress` 目录加入项目目录，然后通过 `add_subdirectory` 和 `target_link_libraries` 即可编译使用。

**CMake 示例：**

```cmake
# 主CMakeLists添加子目录
add_subdirectory(zip_compress)

# target CMakeLists 链接 zip_compress 即可
target_link_libraries(<target_name> PRIVATE zip_compress)
```

include包含头文件即可使用

```c++
#include "zip_compress/zip_reader.h"
#include "zip_compress/zip_writer.h"
```

> C++11 / C++14 下缺失 `std::filesystem` ,
>
> 需要在主CMakeLists添加 `add_subdirectory(path_to_ghc_filesystem)` 即可使用.

### 📝 ZipWriter 示例：创建 ZIP 文件

```c++
#include "zip_writer.h"

int main() {
    zip_compress::ZipWriter zw("output.zip");

    zw.add_file("test.txt");                 // 添加单个文件
    zw.add_folder("assets");                 // 添加整个文件夹
    zw.add_data("hello.txt", "Hello", 5);    // 添加内存数据作为文件

    // 析构或手动 finish() 会自动写入并关闭 ZIP
}
```

------

### 📖 ZipReader 示例：读取 ZIP 文件

### 列出 ZIP 内文件：

```c++
zip_compress::ZipReader zr("output.zip");
auto files = zr.file_list();

for (auto& f : files) {
    std::cout << f << "\n";
}
```

#### 解压整个 ZIP 到文件夹：

```c++
zr.extract_all("out_folder");
```

#### 解压单个文件到指定路径：

```c++
zr.extract_file("assets/image.png", "out/image.png");
```

#### 解压为内存数据：

```c++
auto data = zr.extract_file_to_memory("hello.txt");
std::string s(data.begin(), data.end());
```

------

### 📌 类接口说明

#### ZipWriter

| 方法                         | 说明                         |
| ---------------------------- | ---------------------------- |
| `add_file(path, base_path)`  | 添加文件至 ZIP               |
| `add_folder(path)`           | 递归添加整个文件夹           |
| `add_data(name, data, size)` | 添加内存块作为文件           |
| `finish()`                   | 手动结束写入（析构自动调用） |

#### ZipReader

| 方法                           | 说明                         |
| ------------------------------ | ---------------------------- |
| `file_list()`                  | 列出 ZIP 内所有路径          |
| `extract_all(folder)`          | 解压整个 ZIP                 |
| `extract_file(name, output)`   | 解压单个文件至硬盘           |
| `extract_file_to_memory(name)` | 解压文件到 `vector<uint8_t>` |

### 🛠️ 编译环境

推荐环境：

| 平台                 | 编译器      |
| -------------------- | ----------- |
| Ubuntu / Rocky Linux | GCC ≥ 4.8   |
| Windows              | MSVC ≥ 2015 |
| 跨平台               | Clang ≥ 3.5 |

### 📜 License

本项目使用 **MIT License**。

```txt
MIT License
Copyright (c) 2025 Abin
```

### 🤝 贡献 & 反馈

欢迎提出 Issue / PR
如果你想扩展功能（比如 ZIP 加密、流式压缩、断点续压），也可以告诉我，我可以一起帮你设计结构。

