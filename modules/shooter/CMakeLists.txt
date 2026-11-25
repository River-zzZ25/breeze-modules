zephyr_library_named(breeze_shooter)

file(GLOB SHOOTER_SOURCE CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
zephyr_library_sources(${SHOOTER_SOURCE})

# src 公开是为了让应用层能直接 include "feed.hpp" 等内部实现头(暂无 Module 门面).
target_include_directories(breeze_shooter
    PUBLIC
        "inc"
        "src"
)
# 公开头直接包含 <ramen.hpp> 与 breeze/models/control.hpp,
# 因此这两个依赖必须 PUBLIC, 否则使用方的编译单元找不到头文件.
target_link_libraries(breeze_shooter
    PUBLIC
        ramenx
        breeze::module::models
    PRIVATE
        robotpilots::algorithm
)
