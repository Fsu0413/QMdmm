# QMdmm

一款用 Qt 6 / C++20 实现的多人回合制对战小游戏，复刻中小学课间流行的「打打闹闹」。

> 挖了 8 年的坑：引擎打磨得七七八八，前端方向盘还在图纸上。

## 这是什么

QMdmm 是一个可联网对战的回合制游戏，规则来自学生时代的课间玩法：

- 每位玩家有血量（HP）、刀、马、位置和升级点。
- 每回合先用**石头剪刀布**决定行动顺序，然后轮流行动：买刀、买马、砍人、踢人、移动、拉人。
- 击杀对手可获得升级点，用来升级刀伤害、马伤害或血量上限。
- 率先把刀、马、血量上限三项都升满的玩家获胜。

支持本地对战与联网对战，网络层提供 TCP / 本地套接字 / WebSocket 三种传输。

## 项目状态

| 模块 | 说明 | 状态 |
|---|---|---|
| `QMdmmCore` | 游戏规则引擎（玩家 / 房间 / 回合状态机 / 配置） | 基本可用，有测试 |
| `QMdmmNetworking` | 网络层（服务端 / 客户端 / 信令闭环） | 主流程通，缺重连 / 观战 / 大厅 |
| `QMdmmServer` | 独立服务端程序 | 可启动，命令行配置完整 |
| `QMdmmGui` | 图形客户端（QML） | 仅有开始菜单，尚不能真正开局 |

> 换句话说：目前还没有人能真正玩上一局。核心库已经相当扎实，缺的是把 GUI 接到网络层上。

## 构建

依赖：

- CMake ≥ 3.19
- Qt ≥ 6.5（Core / Network / WebSockets / Gui / Qml / Quick / Widgets / QuickWidgets）
- 支持 C++20 的编译器

```sh
qt-cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -C build
```

需要测试时加 `-DBUILD_TESTING=ON`。产物输出到 `build/build/bin/`。

## 运行

先启动服务端（默认 TCP 端口 6366、WebSocket 端口 6367）：

```sh
./build/build/bin/QMdmmServer6
```

服务端支持 18 个命令行选项（房间人数、伤害数值、超时、传输开关等），`--help` 可看全表。再启动客户端连入：

```sh
./build/build/bin/QMdmm6
```

## 测试

构建时打开 `BUILD_TESTING`，然后：

```sh
ctest --test-dir build --output-on-failure
```

## 目录结构

```
QMdmmCore/       游戏规则引擎
QMdmmNetworking/ 网络层（服务端 / 客户端 / 协议传输）
QMdmmServer/     独立服务端程序
QMdmmGui/        图形客户端（QML）
smoke/           无界面联网对局回归测试
doc/             Doxygen 文档
3rdparty/        第三方依赖
cmake/           CMake 辅助模块
```

## 许可

[AGPL-3.0-or-later](LICENSE)
