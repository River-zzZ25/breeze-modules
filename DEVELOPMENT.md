# 开发指南

本仓库直接内置了工作区与测试app, 克隆当前仓库即可.

## 拉取工作区依赖

```sh
west update
```

## 编译应用

在工作区根目录构建测试应用. 不指定board则是`native_sim 64`, 对于自动测试很方便. 当然也可以指定为喵板. 带 `-p` 可以更干净地配置, 按需添加

```sh
west build test -p
```

## 运行测试

按照上一步执行编译后, 可以直接运行测试.

```sh
west build -t run
```

## 哈希锁定

使用命令来更新哈希锁定:

```sh
west manifest --freeze -o west-lock.yml
```

更新锁定清单后, 更新本地的仓库

```sh
west update
```

## 其他要求

测试app很重要, 代码仍然需要规范化提交并推送.
