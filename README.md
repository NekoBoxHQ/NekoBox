# NekoBox

A modern sing-box GUI client. 单内核 · 稳定优先 · 最小复杂度

[![GitHub All Releases](https://img.shields.io/github/downloads/lima-droid/NekoBox/total?label=downloads-total&logo=github&style=flat-square)](https://github.com/lima-droid/NekoBox/releases)

## 🖼️ 界面展示 / Screenshot

![NekoBox](docs/screenshot.png)

## ✨ 项目简介

NekoBox 是一款独立维护的现代 sing-box GUI 客户端，专注于：

- ✅ 仅使用 sing-box 单一内核（当前 v1.14.2）
- ❌ 不捆绑 Xray / v2ray / 多核心
- ✅ 稳定优先、最小复杂度设计
- ✅ 面向长期维护与升级

NekoBox = sing-box 配置生成 + 控制器，所有流量最终由 sing-box 内核处理。

## 🧠 核心特点

- **🚀 Single Core Only**：仅使用 sing-box
- **⚙️ 清晰架构**：`Link → Bean → sing-box JSON`
- **📦 自动更新**：GitHub Releases + 内置更新
- **🔧 完整协议支持**：导入 / 运行 / 导出完整链路

## 📡 支持协议（全部基于 sing-box）

**✔️ 核心代理协议**

- SSH
- TUIC
- VMess
- VLESS
- Trojan
- WireGuard
- NaïveProxy
- Shadowsocks / SS-2022
- Hysteria / Hysteria2

**✔️ 基础代理**

- HTTP / HTTPS
- SOCKS (4/4a/5)

**✔️ 扩展支持**

- AnyTLS
- Brook（兼容支持）

📌 所有协议均转换为 sing-box 官方 outbound 配置。

## 🔄 工作原理

```
用户输入（链接 / 订阅）
   ↓
Link2Bean（解析）
   ↓
Bean（协议抽象）
   ↓
BuildCoreObjSingBox
   ↓
生成 sing-box JSON
   ↓
nekobox_core（运行）
```

👉 所有流量最终由 sing-box 内核处理。

## 📥 导入支持

**✔️ 单节点链接**

```
vmess://  vless://  trojan://  ss://  tuic://  hy://  hy2://
wireguard://  ssh://  anytls://  brook://  socks://  http(s)://
```

**✔️ 订阅**

- Clash 配置
- v2rayN 格式
- Base64 订阅

**✔️ 自定义配置**

- sing-box JSON（作为自定义配置档导入）

📌 详细支持字段见：👉 [docs/ProtocolImport.md](docs/ProtocolImport.md)

## ⚠️ 设计原则（重要）

- ❗ 不引入私有协议
- ❗ 不修改 sing-box 行为
- ❗ 不做多核心（避免复杂度爆炸）

NekoBox = sing-box 配置生成 + 控制器。

## 🖥️ 支持平台

- Windows（zip）
- Linux（AppImage / tar.gz）

## 📦 安装与使用

**Windows**

1. 下载 Release
2. 解压
3. 运行 `NekoBox.exe`

若提示 DLL 缺失，无法运行，请安装 [微软 C++ 运行库](https://aka.ms/vs/17/release/vc_redist.x64.exe)。

**Linux**

- AppImage 直接运行
- 或使用 tar.gz

[Linux 运行教程](docs/Run_Linux.md)

## 🔄 更新

客户端内置：**设置 → 检查更新**，自动从 GitHub Releases 获取。

## 🛠️ 构建

```
git clone https://github.com/lima-droid/NekoBox
cd NekoBox
```

使用 GitHub Actions 自动构建（推荐）。[技术文档 / Technical documentation](https://github.com/lima-droid/NekoBox/tree/main/docs)

## 🚀 发布流程

```
修改代码 → bump NEKO_VERSION → git tag vX.X.X → push
→ GitHub Actions 自动构建 → 发布到 Releases
```

## 📚 文档

- 运行参数：[docs/RunFlags.md](docs/RunFlags.md)
- 协议导入支持表：[docs/ProtocolImport.md](docs/ProtocolImport.md)

## ⚠️ 已知限制

- 部分字段未实现（详见 ProtocolImport.md）
- 部分新协议暂无 UI 编辑表单（不影响导入与使用）

## 🧭 项目目标

做一个稳定、纯粹、长期可维护的 sing-box 客户端。

## 讨论群组 / Discussion Group

加入 Telegram 群组交流反馈: [NekoBox 讨论群](https://t.me/+Kdxyw8yLTz85ODg5)

## 📄 License

本项目基于 **GPL-3.0** 开源协议发布。
