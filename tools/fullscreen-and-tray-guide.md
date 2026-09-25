# Windows 多屏全屏遮罩、任务栏压制与托盘自愈工程指南

本文档沉淀自 **番茄钟（`pomodoro-timer v2.5.29 ~ v2.5.30`）** 与 **Blackout（`v1.2.0 ~ v1.2.1`）** 的实战开发与排障经验，供后续 Windows 桌面工具与全屏应用复用。

---

## 一、多屏全屏遮罩与 DisplayFusion 任务栏 / 截图工具无闪烁共存

### 1. 常见踩坑与根因
1. **仅凭矩形坐标判断屏幕归属导致副屏任务栏闪烁**  
   在多屏紧邻或混合 DPI（如 `2880×1800@200%` 主屏 + `1200×1920` 副屏 + `3840×2160` 副屏）环境下，仅凭 `IntersectsWith` 或 `PtInRect(&mon_rect, center)` 会因边界擦边或 DPI 虚拟化偏移，将**未全屏副屏上的 DisplayFusion 任务栏（`DFTaskbar`）**误判为位于全屏屏幕内并调用 `ShowWindow(SW_HIDE)`。  
   当鼠标焦点切至副屏窗口时，DisplayFusion 检测到该屏活动窗口非全屏，立即调用 `SW_SHOW` 恢复任务栏，从而与全屏守护定时器的 `SW_HIDE` 形成 **300ms 周期的 `SW_HIDE` ↔ `SW_SHOW` 拉锯闪烁**。
2. **焦点离开全屏屏后放弃 `HWND_TOPMOST` 或移除 `EVENT_OBJECT_SHOW` 钩子**  
   - 若在焦点切到副屏时停止对全屏窗口执行 `SetWindowPos(HWND_TOPMOST)`，本屏任务栏会趁虚穿透全屏黑幕。
   - 若为规避截图冲突而完全移除 `EVENT_OBJECT_SHOW` 钩子，仅靠 300ms 定时器轮询，会导致任务栏冒头长达 300ms（约 18 帧）才被隐藏，产生肉眼可见的跳动。
3. **无白名单隐藏所有 `WS_EX_TOPMOST` 窗口导致截图工具被杀**  
   QQ/微信截图（类名为 `TXGuiFoundation`）在启动时会创建跨所有显示器的置顶大画布。若不加类名白名单无差别隐藏置顶窗口，在未全屏的副屏上按 `Ctrl+Alt+A` 截图会在 0.1 秒内被 `SW_HIDE` 强行拍死。

### 2. 标准根治方案（黄金三定律）
1. **内核级 `HMONITOR` 物理屏幕句柄严格隔离**  
   压制任何任务栏前，必须调用 `MonitorFromWindow(hwnd, MONITOR_DEFAULTTONULL)` 获取目标窗口所属的物理显示器句柄，并与当前已开启全屏遮罩的显示器 `HMONITOR` 集合精确比对（`win_mon == item->handle`）。凡不属于全屏显示器的窗口 100% 跳过，从物理层面杜绝副屏任务栏闪烁。
2. **可压制窗口类名白名单（`is_suppressible_taskbar`）**  
   严格限定仅对以下类名执行 `SW_HIDE`：
   - `Shell_TrayWnd`（Windows 主屏原生任务栏）
   - `Shell_SecondaryTrayWnd`（Windows 副屏原生任务栏）
   - `DFTaskbar*` / `DisplayFusionTaskbar*`（DisplayFusion 多屏任务栏）
   - `Windows.UI.Core.CoreWindow`（按需：Windows 系统通知横幅）  
   绝不触碰 `TXGuiFoundation`（QQ/微信截图）及其它用户置顶工具。
3. **`EVENT_OBJECT_SHOW` 双重门禁 + 无条件 `HWND_TOPMOST | SWP_NOACTIVATE`**  
   - 全屏窗口无条件保持 `SetWindowPos(..., HWND_TOPMOST, ..., SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING)`。
   - `SetWinEventHook(EVENT_OBJECT_SHOW)` 在回调中严格校验：
     ```c
     if (idObject != OBJID_WINDOW || idChild != CHILDID_SELF) return;
     if (event == EVENT_OBJECT_SHOW) {
         if (!hwnd || !fs_is_suppressible_taskbar(hwnd) || !fs_is_on_fullscreen_monitor_handle(hwnd)) return;
     }
     ```
     既保证全屏本屏任务栏冒头时 `<1ms` 零帧瞬时隐藏，又对副屏任务栏和 QQ 截图保持 0 触发、0 干扰。

---

## 二、管理员权限（`requireAdministrator`）、开机自启与托盘不死鸟机制

### 1. 权限边界判定（UIPI 与 Z-Order Band）
- **普通权限（`asInvoker`）能做什么**：压制 `explorer.exe` 和 `DisplayFusion.exe` 的任务栏、通过 WMI（`WmiMonitorBrightnessMethods`）和 DDC/CI（`dxva2.dll`）调节显示器亮度。
- **管理员权限（`requireAdministrator`）不可替代的场景**：
  1. 压制开启了“总在最前”的管理员权限窗口（如任务管理器、管理员悬浮窗）；
  2. 确保当系统焦点停留在管理员窗口（如管理员终端、任务管理器）时，全局低级键盘钩子 `WH_KEYBOARD_LL`（如按 `Esc` 退出黑屏）不被 Windows UIPI 屏蔽。

### 2. 管理员程序开机自启失败的 4 大真因与根治
Windows 严禁在开机登录阶段启动带 `requireAdministrator` 的注册表 `HKCU\...\Run` 自启项。改用任务计划程序（`schtasks`）时必须避开以下默认陷阱：

| 默认陷阱 | 故障现象 | 根治配置（Task Scheduler 1.2 XML） |
| :--- | :--- | :--- |
| `DisallowStartIfOnBatteries = true` | 笔记本未插电或扩展坞供电握手慢半秒时，开机直接拒绝启动。 | `<DisallowStartIfOnBatteries>false</DisallowStartIfOnBatteries>` |
| `StopIfGoingOnBatteries = true` | 使用中途拔掉电源或电源瞬断，系统直接强杀进程。 | `<StopIfGoingOnBatteries>false</StopIfGoingOnBatteries>` |
| `ExecutionTimeLimit = PT72H` | 电脑连续开机满 72 小时被任务计划程序当超时任务强杀。 | `<ExecutionTimeLimit>PT0S</ExecutionTimeLimit>` |
| 登录抢跑 `explorer.exe` | `ONLOGON` 触发早于桌面 `Shell_TrayWnd` 初始化，`NIM_ADD` 打空变成无图标后台幽灵进程。 | 启动轮询 `Shell_TrayWnd` + 异步重试定时器 + `TaskbarCreated` 监听。 |

### 3. 托盘图标 100% 永不丢失的四道闭环防线
1. **启动期绝不因首次 `Shell_NotifyIcon(NIM_ADD)` 失败而报错自杀**  
   首次 `NIM_ADD` 返回 `FALSE` 时，严禁弹窗退出（`return 1`），应启动异步重试定时器（如每 `1500ms` 重试一次 `NIM_MODIFY || NIM_ADD`），一旦托盘就绪立即挂载。
2. **运行期 `NIM_MODIFY` 失败自动回退 `NIM_ADD`**  
   在每次更新图标或心跳定时器中统一采用：
   ```c
   if (!Shell_NotifyIconW(NIM_MODIFY, &nid)) {
       Shell_NotifyIconW(NIM_ADD, &nid);
   }
   ```
3. **监听 `TaskbarCreated` 系统广播**  
   注册 `RegisterWindowMessageW(L"TaskbarCreated")`（高权限进程需配合 `ChangeWindowMessageFilterEx(..., MSGFLT_ALLOW, ...)` 放行跨 UIPI 消息），当 `explorer.exe` 崩溃重启时自动重新挂载托盘图标。
4. **单实例锁“唤醒/接管”替代“静默退出”**  
   - 当检测到后台已有实例运行时，新实例必须向旧实例主窗口投递 `TaskbarCreated` 或唤醒消息，强制旧实例立刻重挂托盘图标（若旧实例无响应或需要热更新则终止旧实例并接管），彻底消灭“后台有僵尸进程占锁、右下角没图标、双击也没反应”的死锁。
   - 实现“重启软件”功能时，必须在拉起新进程前**先显式释放单实例 `Mutex`**，防止新进程启动过快撞锁自杀。

---

## 三、Win32 / .NET 桌面开发关键稳定性与体验细节

1. **P/Invoke 回调委托必须声明为 `static readonly` 静态常驻字段（防 `0xc0000005` 崩溃）**  
   在 C# / .NET 中，传给 `SetWindowsHookEx`、`SetWinEventHook`、`EnumWindows`、`EnumDisplayMonitors` 的委托如果使用局部变量或匿名 Lambda，一旦 `.NET GC` 触发回收，非托管 `user32.dll` 回调悬空函数指针会直接引发无法捕获的 `AccessViolationException (0xc0000005)` 原生闪退。
2. **原生右上方弹出托盘菜单（替代 WinForms `ContextMenuStrip`）**  
   - `.NET WinForms` 的 `NotifyIcon.ContextMenuStrip` 内部硬编码了 `TPM_RIGHTALIGN` 且受系统 `SPI_GETMENUDROPALIGNMENT`（手写笔右撇子默认左偏）影响，导致菜单总出现在鼠标**左侧**且样式陈旧。
   - **标准原生做法**：
     - 调用 `uxtheme.dll` 序号 135 `SetPreferredAppMode(1 /* AllowDark */)` 与序号 136 `FlushMenuThemes()` 自动适配 Windows 11 深浅色圆角菜单。
     - 弹出前临时执行 `SystemParametersInfoW(SPI_SETMENUDROPALIGNMENT, 0, IntPtr.Zero, 0)` 强制设为左对齐，再以 `TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD` 调用 `TrackPopupMenuEx`，确保菜单 **100% 在鼠标右上方展开**。
     - 承载菜单的隐藏窗口必须为标准顶层工具窗口（`WS_POPUP | WS_EX_TOOLWINDOW`），不能是 `HWND_MESSAGE` 仅消息窗口，否则 `SetForegroundWindow` 会失败导致点击菜单外部无法自动收起。
3. **`PerMonitorV2` 下无边框全屏窗口忌用 `FormWindowState.Maximized`**  
   在多屏混合 DPI 下，`Maximized` 容易被系统按主屏或 `WorkArea` 裁剪留边。应保持 `FormWindowState.Normal` + `FormBorderStyle.None`，直接通过 `EnumDisplayMonitors` + `GetMonitorInfo` 获取物理像素 `rcMonitor`，并用 `SetWindowPos` 精确覆盖。
