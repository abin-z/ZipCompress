# 定义目标库名称
set(tgt_name zip_compress)

add_library(${tgt_name})

# 动态查找头文件和源文件，便于管理新增文件
file(GLOB_RECURSE headers CONFIGURE_DEPENDS include/*.h include/*.hpp) # 查找所有头文件
file(GLOB_RECURSE sources CONFIGURE_DEPENDS src/*.c src/*.cpp src/*.cc src/*.cxx) # 查找所有源文件

# 将头文件和源文件添加到目标中
target_sources(${tgt_name} PUBLIC ${headers}) # 头文件作为公共接口，供其他项目可见
target_sources(${tgt_name} PRIVATE ${sources}) # 源文件仅供内部使用

add_subdirectory(3rd/miniz)
target_link_libraries(${tgt_name} PRIVATE miniz-inline)

# 设置包含目录，供编译器查找头文件
target_include_directories(${tgt_name} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)