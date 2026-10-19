# 开发指南

## 建立工作区

```sh
west init -m https://github.com/RobotPilots-SZU/breeze-modules-test [workspace]
cd [workspace]
west update
```

`west update` 会按 manifest 获取各种所需需要的依赖.

## 开发模块

先签出到开发分支, 不要在游离HEAD上直接开发.

```sh
git checkout [分支]
```

执行构建以配置cmake, 并且生成 compile_commands. (实测这一步不能cmake-only)

```sh
west build -p always -b damiao_mc02 breeze-modules-test
```

在 `breeze-modules` 是模块, 只通过 `zephyr/module.yml` 注册, 不能单独构建, 需要借助测试用app进行构建.

## 编译应用

在工作区根目录构建测试应用：

```sh
west build -b damiao_mc02 breeze-modules-test
```

修改 Kconfig 或 CMake 配置后，使用 pristine 构建重新生成配置：

```sh
west build -p always -b damiao_mc02 breeze-modules-test
```
## 其他要求

测试app很重要, 代码仍然需要规范化提交并推送.
