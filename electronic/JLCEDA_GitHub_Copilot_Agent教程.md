# 嘉立创 EDA + GitHub Copilot Agent + easyeda-api 配置与使用教程

> 适用环境：Windows、嘉立创 EDA 专业版、Visual Studio Code、GitHub Copilot Agent。  
> 本教程按当前已验证的本机配置编写，Bridge Server 使用端口 `49620`，`easyeda-api` 位于用户级 Copilot Skills 目录。

---

## 1. 系统架构

整套链路如下：

```text
GitHub Copilot Agent（VS Code）
        ↓ 调用 Skill
 easyeda-api Skill
        ↓ HTTP / WebSocket
 Bridge Server（Node.js，端口 49620）
        ↓ WebSocket
 Run API Gateway 扩展
        ↓
 嘉立创 EDA 专业版
```

各部分职责：

- **GitHub Copilot Agent**：理解自然语言指令，读取 Skill 文档，并调用相关脚本。
- **easyeda-api Skill**：提供嘉立创 EDA API 的调用说明、脚本和 Bridge Server。
- **Bridge Server**：在 AI Agent 与嘉立创 EDA 之间转发请求，默认监听 `127.0.0.1:49620`。
- **Run API Gateway**：安装在嘉立创 EDA 专业版中的扩展，连接 Bridge Server，并在 EDA 环境中执行 API 请求。

---

# 2. 搭建环境

## 2.1 安装必要软件

需要安装以下软件：

1. **嘉立创 EDA 专业版桌面客户端**
2. **Visual Studio Code**
3. **GitHub Copilot** 和 **GitHub Copilot Chat** 扩展
4. **Node.js 22 LTS 或更高版本**
5. 嘉立创 EDA 的 **Run API Gateway** 扩展
6. **easyeda-api Skill**

> 不建议使用嘉立创 EDA 网页版。当前 Bridge 工作流应使用嘉立创 EDA 专业版桌面客户端。

## 2.2 检查 Node.js 和 npm

打开 PowerShell，执行：

```powershell
node --version
npm --version
```

能显示版本号，说明安装正常。

如果提示找不到命令，请重新安装 Node.js，并在安装后重新打开 PowerShell。

## 2.3 安装 Run API Gateway

在嘉立创 EDA 专业版中安装并启用 `Run API Gateway` 扩展。

扩展的主要作用是扫描本机端口 `49620-49629`，找到 `easyeda-bridge` 服务后建立 WebSocket 连接。

安装完成后，在嘉立创 EDA 的扩展管理界面确认：

- Run API Gateway 已安装
- 扩展处于启用状态
- 扩展没有报错

## 2.4 安装 easyeda-api Skill

GitHub Copilot 的用户级 Skill 目录为：

```text
C:\Users\<用户名>\.copilot\skills\
```

当前电脑使用的目录是：

```text
C:\Users\hujia\.copilot\skills\easyeda-api
```

完整目录通常应包含：

```text
easyeda-api\
├─ SKILL.md
├─ README.md
├─ README.zh-Hans.md
├─ package.json
├─ package-lock.json
├─ scripts\
├─ references\
├─ guide\
├─ user-guide\
└─ node_modules\
```

检查 Skill 是否存在：

```powershell
Test-Path "$env:USERPROFILE\.copilot\skills\easyeda-api\SKILL.md"
```

返回：

```text
True
```

表示 Skill 文件存在。

如果 Skill 已下载但依赖没有安装，执行：

```powershell
cd "$env:USERPROFILE\.copilot\skills\easyeda-api"
npm install
```

## 2.5 配置 VS Code 和 GitHub Copilot

1. 打开 Visual Studio Code。
2. 安装并启用 GitHub Copilot 和 GitHub Copilot Chat。
3. 登录具有 Copilot 授权的 GitHub 账号。
4. 将 Copilot Chat 切换至 **Agent** 模式。
5. 执行命令面板：

```text
Ctrl+Shift+P
```

然后运行：

```text
Developer: Reload Window
```

这样可以让 Copilot 重新扫描用户级 Skills。

可以在 Copilot Agent 中输入：

```text
请检查是否能够发现 easyeda-api Skill，并说明该 Skill 的用途。
```

---

# 3. 如何启动服务

## 3.1 必须遵守的启动顺序

当前环境下，最稳定的启动顺序是：

```text
1. 启动 Bridge Server
2. 确认 49620 端口正常
3. 启动嘉立创 EDA 专业版
4. 打开一个实际原理图或 PCB 工程
5. 确认 Run API Gateway 已连接
6. 启动 VS Code 和 GitHub Copilot Agent
```

> 重要：实际测试发现，如果先启动嘉立创 EDA，再启动 Bridge Server，Run API Gateway 有时不会重新连接，最终会显示 `edaConnected: false`。因此建议始终先启动 Bridge，再启动嘉立创 EDA。

## 3.2 启动 Bridge Server

### 方式一：双击一键启动脚本（推荐）

直接双击当前目录下的 [start_bridge_server.bat](./start_bridge_server.bat)，脚本会自动打开 PowerShell 窗口、切换到 `easyeda-api` 目录并执行 `npm run server`。

### 方式二：手动在 PowerShell 中启动

打开 PowerShell，执行：

```powershell
cd "$env:USERPROFILE\.copilot\skills\easyeda-api"
npm run server
```

正常情况下会看到类似输出：

```text
Bridge Server started
Listening on port 49620
```

如果看到：

```text
Bridge server is already running on port 49620, no need to start another instance.
```

说明 Bridge 已经运行，不需要重复启动。

### 常见错误：ENOENT / 找不到 package.json

如果在以下目录直接运行：

```text
C:\Users\hujia>
```

然后执行：

```powershell
npm run server
```

可能报错：

```text
ENOENT: no such file or directory, open 'C:\Users\hujia\package.json'
```

原因是当前目录错误。必须先进入：

```powershell
cd "$env:USERPROFILE\.copilot\skills\easyeda-api"
```

再执行：

```powershell
npm run server
```

## 3.3 检查 Bridge 健康状态

打开另一个 PowerShell 窗口，执行：

```powershell
Invoke-RestMethod http://localhost:49620/health
```

也可以使用：

```powershell
curl.exe http://localhost:49620/health
```

典型返回：

```json
{
  "service": "easyeda-bridge",
  "status": "ok",
  "edaConnected": true,
  "edaWindowCount": 1,
  "activeWindowId": "...",
  "pendingRequests": 0
}
```

字段含义：

| 字段 | 含义 |
|---|---|
| `status: ok` | Bridge Server 正常运行 |
| `edaConnected: true` | 嘉立创 EDA 已连接 Bridge |
| `edaWindowCount: 1` | 已识别至少一个 EDA 窗口 |
| `activeWindowId` 非空 | 已识别当前活动窗口 |
| `pendingRequests: 0` | 当前没有待处理请求 |

如果返回：

```json
{
  "status": "ok",
  "edaConnected": false,
  "edaWindowCount": 0
}
```

说明 Node.js 和 Bridge 正常，但嘉立创 EDA 尚未连接。

## 3.4 启动嘉立创 EDA

确认 Bridge 已启动后：

1. 启动嘉立创 EDA 专业版桌面客户端。
2. 打开一个真实工程。
3. 打开该工程中的原理图或 PCB 页面。
4. 检查 Run API Gateway 状态。

正常状态应类似：

```text
API Gateway v1.0.6
Connected (port 49620)
```

不要只停留在欢迎页，因为 Bridge 可能无法识别有效设计窗口。

## 3.5 启动 VS Code 和 Copilot Agent

1. 打开 Visual Studio Code。
2. 打开 Copilot Chat。
3. 选择 Agent 模式。
4. 建议打开一个普通工作区，方便 Agent 运行本地脚本。
5. 输入测试命令。

第一个测试：

```text
使用 easyeda-api Skill 检查 Bridge Server 和嘉立创 EDA 的连接状态。
```

第二个测试：

```text
使用 easyeda-api Skill 获取当前嘉立创 EDA 工程信息。
```

如果能够返回当前工程名、文档类型或窗口信息，说明完整链路已经连通。

---

# 4. 如何使用

## 4.1 推荐的使用方式

不要只输入模糊指令，例如：

```text
帮我看看 PCB。
```

建议明确要求 Copilot 使用 `easyeda-api Skill`，并说明分析范围和是否允许修改。

例如：

```text
使用 easyeda-api Skill 读取当前嘉立创 EDA 原理图。
只做检查，不要修改任何内容。
重点检查：
1. 电源网络是否存在短路或电压冲突；
2. 芯片电源脚是否缺少去耦电容；
3. 上拉和下拉电阻是否合理；
4. 未连接引脚和悬空输入；
5. 器件参数和封装是否可疑。
请按严重程度列出问题，并给出器件位号和修改建议。
```

## 4.2 读取当前工程

```text
使用 easyeda-api Skill 获取当前工程信息，并列出：
1. 当前文档类型；
2. 原理图页数；
3. 主要器件；
4. 电源网络；
5. 当前是否处于可写状态。
不要修改工程。
```

## 4.3 检查原理图

```text
使用 easyeda-api Skill 检查当前原理图。
重点检查 ESP32-S3 的电源、EN、BOOT、USB、UART、晶振、Flash/PSRAM 和去耦电路。
先读取和分析，不允许自动修改。
所有结论必须标明器件位号和网络名称。
```

## 4.4 检查 PCB

```text
使用 easyeda-api Skill 读取当前 PCB，并进行布局检查。
重点检查：
1. ESP32 天线净空；
2. USB 差分线；
3. 开关电源高电流回路；
4. 晶振和时钟走线；
5. 地平面连续性；
6. 去耦电容与芯片电源脚距离；
7. 大电流器件温升与铜皮面积。
不要修改 PCB，只输出检查报告。
```

## 4.5 执行修改

建议采用“先读、再计划、后修改”的三步流程。

### 第一步：只读分析

```text
使用 easyeda-api Skill 检查当前原理图，只读，不要修改。
```

### 第二步：生成修改计划

```text
根据检查结果生成修改计划。
每项列出：目标器件、当前连接、计划修改、风险和验证方法。
仍然不要修改。
```

### 第三步：执行有限修改

```text
使用 easyeda-api Skill 只执行修改计划中的第 1 项。
执行前再次确认目标器件位号和网络名。
执行后读取修改结果并报告，不要继续执行其他项目。
```

这种方式可以避免 Agent 一次性修改过多内容。

## 4.6 测试写入能力

首次测试不要直接改关键电路。可以让 Agent 放置一个不连接网络的测试电阻：

```text
使用 easyeda-api Skill，在当前原理图空白区域放置一个 10 kΩ 测试电阻。
不要连接任何网络。
完成后报告新增器件的位号和坐标。
```

确认成功后手工删除测试器件，或者让 Agent 删除刚新增的精确位号。

---

# 5. 常见故障排查

## 5.1 `status: ok`，但 `edaConnected: false`

现象：

```json
{
  "status": "ok",
  "edaConnected": false,
  "edaWindowCount": 0
}
```

处理步骤：

1. 完全关闭嘉立创 EDA。
2. 确认 Bridge Server 仍在运行。
3. 重新启动嘉立创 EDA。
4. 打开一个真实工程及原理图或 PCB 页面。
5. 检查 Run API Gateway 是否显示 `Connected (port 49620)`。
6. 再次访问 `/health`。

这是当前环境中最常见的问题。

## 5.2 Run API Gateway 显示 Disconnected

依次检查：

```powershell
netstat -ano | findstr 49620
```

如果没有 `LISTENING`，重新启动 Bridge：

```powershell
cd "$env:USERPROFILE\.copilot\skills\easyeda-api"
npm run server
```

如果端口正常但 Gateway 仍未连接：

1. 关闭嘉立创 EDA；
2. 确认 Bridge 已运行；
3. 再启动嘉立创 EDA；
4. 检查扩展是否启用。

## 5.3 Copilot 不识别 easyeda-api Skill

检查：

```powershell
Test-Path "$env:USERPROFILE\.copilot\skills\easyeda-api\SKILL.md"
```

然后在 VS Code 执行：

```text
Developer: Reload Window
```

如果仍然无法识别：

- 检查是不是把 Skill 放到了旧的 `.cursor\skills` 目录；
- 确认当前使用的目录是 `.copilot\skills\easyeda-api`；
- 确认文件名为大写 `SKILL.md`；
- 确认 GitHub Copilot Chat 处于 Agent 模式。

## 5.4 Bridge 提示端口已占用

如果输出：

```text
Bridge server is already running on port 49620
```

通常不算错误，说明已有实例在运行。直接检查：

```powershell
Invoke-RestMethod http://localhost:49620/health
```

如果端口被其他异常进程占用，可以先查询 PID：

```powershell
netstat -ano | findstr 49620
```

确认进程身份：

```powershell
Get-Process -Id <PID>
```

不要在未确认进程身份前直接结束进程。

## 5.5 Gateway 已连接，但 Copilot 返回 0 个窗口

检查：

- 是否打开的是嘉立创 EDA 专业版桌面客户端；
- 是否只停留在欢迎页；
- 是否已经打开具体原理图或 PCB；
- 是否按照“Bridge 先启动，EDA 后启动”的顺序；
- `/health` 中的 `edaWindowCount` 是否大于 0。

必要时完整重启：

```text
关闭 VS Code
关闭嘉立创 EDA
确认或重新启动 Bridge
启动嘉立创 EDA
打开工程
确认 Gateway Connected
启动 VS Code
```

---

# 6. 安全与工程建议

1. **默认先使用只读提示词**，明确写出“不要修改工程”。
2. 写入前保存工程，并创建一个 Git 提交、工程副本或压缩备份。
3. 每次只让 Agent 修改一个功能块或一个器件。
4. 修改后重新读取相关器件和网络，确认结果确实落入 EDA 工程。
5. AI 检查不能替代 ERC、DRC、原理图审查、PCB 设计规则和样机测试。
6. 对电源、USB、射频、天线、马达、线圈、功率 MOSFET 等关键电路，最终必须人工复核。
7. Bridge Server 建议只监听 `127.0.0.1`，不要直接暴露到公网。
8. 公司工程应遵循公司的代码、设计文件和数据安全要求。

---

# 7. 每次使用的快捷流程

以后日常启动可以直接按以下清单操作：

```text
[ ] 1. PowerShell 进入 easyeda-api 目录
[ ] 2. 执行 npm run server
[ ] 3. 检查 http://localhost:49620/health
[ ] 4. 启动嘉立创 EDA 专业版
[ ] 5. 打开实际原理图或 PCB
[ ] 6. 确认 API Gateway Connected (port 49620)
[ ] 7. 确认 edaConnected=true、edaWindowCount>=1
[ ] 8. 启动 VS Code
[ ] 9. 打开 GitHub Copilot Agent
[ ] 10. 先下达只读检查命令，再决定是否允许修改
```

常用启动命令：

```powershell
cd "$env:USERPROFILE\.copilot\skills\easyeda-api"
npm run server
```

常用状态检查命令：

```powershell
Invoke-RestMethod http://localhost:49620/health
```

常用 Copilot 测试指令：

```text
使用 easyeda-api Skill 检查 Bridge Server 和当前嘉立创 EDA 工程的连接状态。
只读取，不要修改任何工程内容。
```

---

# 8. 当前已验证配置

```text
操作系统：Windows
Skill 目录：C:\Users\hujia\.copilot\skills\easyeda-api
Bridge 端口：49620
Bridge 地址：http://localhost:49620
健康检查：http://localhost:49620/health
EDA 侧扩展：Run API Gateway
AI 工具：Visual Studio Code + GitHub Copilot Agent
推荐启动顺序：Bridge Server → 嘉立创 EDA → VS Code/Copilot Agent
```

如果以后升级 `easyeda-api`、Run API Gateway、Node.js、VS Code 或 GitHub Copilot，建议重新做一次以下三项验证：

1. `/health` 返回 `status: ok`；
2. `/health` 返回 `edaConnected: true`；
3. Copilot Agent 能读取当前工程信息，但不会在未授权时修改工程。
