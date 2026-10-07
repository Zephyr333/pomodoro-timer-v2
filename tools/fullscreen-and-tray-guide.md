# Windows 多屏全屏遮罩、任务栏压制与托盘自愈工程指南

本文档沉淀自 **番茄钟（`pomodoro-timer v2.5.29 ~ v3.0.0`）** 与 **Blackout（`v1.2.0 ~ v1.2.1`）** 的实战开发与排障经验，供后续 Windows 桌面工具与全屏应用复用。

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
2. **原生深色模式适配与右上方弹出托盘菜单**  
   - `.NET WinForms` 的 `NotifyIcon.ContextMenuStrip` 内部硬编码了 `TPM_RIGHTALIGN` 且受系统 `SPI_GETMENUDROPALIGNMENT`（手写笔右撇子默认左偏）影响，导致菜单总出现在鼠标**左侧**且样式陈旧；普通 Win32 菜单在 Windows 10/11 深色系统下默认依然显示刺眼的亮白底色。
   - **工业级标准原生做法（四项闭环）**：
     1. **内核级真实版本门禁（防系统崩溃）**：
        - 序号 135 在 Win10 1809（Build 17763）是 `BOOL WINAPI AllowDarkModeForApp(BOOL)`；在 Win10 1903+（Build 18362+）才重构为 `PreferredAppMode WINAPI SetPreferredAppMode(PreferredAppMode)`（传参 `1 /* AllowDark */`）。
        - 严禁调用易受清单或兼容模式篡改的 `GetVersionEx`，必须通过 `ntdll.dll!RtlGetVersion` 读取内核真实物理 Build 号。凡 Build < 17763 必须完全跳过调用，杜绝在旧系统（如 Win7/8/早期Win10）上调用未知序号引发未定义闪退。
     2. **注入时钟严格保序（首个 UI 窗口创建前）**：
        - `SetPreferredAppMode` / `AllowDarkModeForApp` 必须紧跟在 `SetProcessDpiAwarenessContext` 之后、**首个 Win32 窗口创建前**执行。若窗口或菜单句柄创建后再注入，系统菜单内部主题缓存状态会被锁死失效。
        - 窗口创建后显式调用序号 133 `AllowDarkModeForWindow(hwnd, TRUE)` 标记支持。
     3. **手写笔对齐保护与右上方稳定展开**：
        - 弹出前临时检测并执行 `SystemParametersInfoW(SPI_SETMENUDROPALIGNMENT, 0, NULL, 0)` 强制设为左对齐，再以 `TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD` 调用 `TrackPopupMenuEx`，确保菜单 **100% 在鼠标右上方展开**；菜单退出后立即原样还原原对齐值。
        - 承载菜单的隐藏窗口必须为标准顶层工具窗口（`WS_POPUP | WS_EX_TOOLWINDOW`），不能是 `HWND_MESSAGE` 仅消息窗口，否则 `SetForegroundWindow` 会失败导致点击菜单外部无法自动收起。
     4. **收起后消息泵清空与系统深浅色免重启热更新**：
        - 微软规范要求：在 `TrackPopupMenuEx` 返回后必须立即补发 `PostMessageW(hwnd, WM_NULL, 0, 0)`，排空系统通知队列，防止点击外部收起后后续右键点击失焦不响应。
        - 主窗口 `WndProc` 必须拦截 `WM_SETTINGCHANGE` 与 `WM_THEMECHANGED` 广播并调用序号 136 `FlushMenuThemes()`；同时在每次 `WM_RBUTTONUP` 呼出菜单前主动刷新，使得用户在系统切换深浅色时无感即时生效。
3. **`PerMonitorV2` 下无边框全屏窗口忌用 `FormWindowState.Maximized`**  
   在多屏混合 DPI 下，`Maximized` 容易被系统按主屏或 `WorkArea` 裁剪留边。应保持 `FormWindowState.Normal` + `FormBorderStyle.None`，直接通过 `EnumDisplayMonitors` + `GetMonitorInfo` 获取物理像素 `rcMonitor`，并用 `SetWindowPos` 精确覆盖。
4. **轻量无边框 Toast / 状态提醒条的防漂移与交互法则**  
   - **慎用 `WM_NCLBUTTONDOWN, HTCAPTION` 模拟拖动**：右下角自绘无边框通知弹窗若支持点击背景拖拽，极易在快速点击时发生误触漂移，导致弹窗脱离任务栏停靠区。
   - **折叠状态背景点击直达展开**：当通知弹窗超时或手动折叠为边缘极窄（如 40px）的垂直提醒条时，条形背景的点击事件应直接映射为展开命令，杜绝将窄条误拖到屏幕中央，大幅优化高频使用下的单手盲操体验。

---

## 四、Win32 桌面计时状态机：单线程消息驱动时钟替代多线程（消灭竞争与死锁）

### 1. 痛点与历史教训
- **多线程计时线程的先天死穴**：早期版本使用独立工作线程（`timer_thread` 配合 `Sleep(1000)` 循环），通过投递自定义消息与主 UI 线程同步。在面对 Win32 复杂交互时暴露出三大硬伤：
  1. **模态消息循环阻塞**：当用户呼出输入框（`DialogBox` / `PromptForInteger`）、通用颜色对话框（`ChooseColor`）或系统右键菜单时，主线程进入模态消息泵。工作线程与主线程的数据同步极易发生滞后、不同步甚至死锁；
  2. **时钟漂移与跳秒**：简单的 `Sleep(1000)` 在系统睡眠唤醒、锁屏或高频拖拽交互下无法真实反映单调物理时间流逝，极易产生累积漂移；
  3. **并发状态篡改**：微休息冻结/恢复、超时正计时平移刻度、快速托盘连击等多状态并存时，多线程共享状态必须加锁，极易引发死锁或状态覆盖。

### 2. 根治方案：主线程单调时钟与分段推进引擎
1. **主线程高精度单调时钟（`GetTickCount64` + 50ms Tick）**：
   - 彻底废除任何独立的后台计时工作线程，所有状态推演 100% 收拢于主窗口所在 UI 线程中。
   - 通过 `SetTimer(hwnd, ID_TIMER_CLOCK, 50, NULL)` 驱动高频心跳，利用 `GetTickCount64() - timer_last_tick` 测量真实物理毫秒差值，累积转化为整秒推进。无线程切换开销，彻底杜绝跨线程死锁与数据竞争。
2. **跨刻度分段步进推进（`timer_advance_seconds`）**：
   - 当系统睡眠唤醒、卡顿或长耗时事件发生导致单次累积时间大跨度增加（如一次性跳跃 900 秒）时，严禁直接执行 `remaining_seconds -= elapsed`。
   - 必须采用**分段步进推进算法**：以“下一个有效物理刻度（如微休息点）”切分推进区间。第一段步进至刻度边界并触发阶段流转（进入微休息等待）；剩余时间则作为新阶段流逝时间继续处理（如被动超时正计时），彻底杜绝跨越刻度漏触发或吞噬超时。
3. **代际防御（`timer_stage_generation`）消除模态输入跨界污染**：
   - 弹出模态对话框前，捕获当前会话阶段代际号：`unsigned gen = timer_stage_generation;`。
   - 对话框确认返回时，校验 `gen == timer_stage_generation`。若用户输入期间计时器已自然到点流转或被托盘点击推进，果断拒绝将针对上一阶段的输入写入新阶段，确保阶段边界绝对纯净。

---

## 五、复杂桌面状态机设计的正交解耦法则（模式 / 状态 / 动作收敛）

### 1. 痛点：内部流程状态拼接与动词爆炸
- 随着复杂业务流程（番茄钟 ↔ 微休息 ↔ 超时正计时 ↔ 冻结恢复）叠加，容易犯“将内部流转状态直接拼接给用户看”的反模式（如“微休息完成 · 等待恢复专注 · 超时正计时”）；并在各入口随阶段动态发明大量动词（“恢复专注”、“下一阶段”、“开始微休息”、“继续自定义”）。
- **后果**：全屏 HUD、托盘悬停 Tooltip、会话菜单各说各话，视觉与心智模型极度混乱。

### 2. 状态机正交解耦五大工程准则
1. **模式（Mode）与状态（State）严格正交分离**：
   - **Mode** 明确业务客体类型：`长番茄钟`、`短番茄钟`、`微休息`、`短休息`、`长休息`、`正计时`、`自定义计时`、`超时正计时`。
   - **State** 明确客体动静态：`待机`（未开始）、`运行`（无后缀）、`暂停`（已暂停）。
   - 严禁发明复合伪模式；所有界面元素严格遵循 `模式名 [ · 状态]` 规范。
2. **全局会话操作动词收敛为三原语**：
   - 会话控制仅保留基础动词：“开始”、“暂停／继续”、“结束”。
   - 阶段推进与恢复仅通过控制这三个动词的使能与置灰（Enabled / Grayed）表达，绝不在按钮上动态发明新动词（例如接回原番茄仍使用“开始”，微休息中途放弃使用“结束”）。
3. **瞬态提醒（Event）与驻留视图（View）彻底剥离**：
   - “已完成”仅属于到点触发的瞬间 Toast 通知事件，绝不长期霸占 HUD 与托盘作为持续停滞状态。
   - 倒计时归零时：若开启超时正计时，无缝转入 `超时正计时` 模式；若关闭超时，直接显示下一阶段的 `待机` 状态（展示下一段的目标模式与预设/冻结时长）。画面所见即为点击“开始”将要启动的内容，消除“看着已完成、点击却开始新模式”的认知割裂。
4. **精确上下文冻结与接回模型（微休息生命周期）**：
   - 微休息触发并开始时，精准冻结主番茄真实剩余秒数；微休息本身的倒计时与超时正计时完全独立，绝不消耗主番茄时间。
   - 到点未开始微休息时的超时正计时，被动平移主番茄总时长，动态新增后续有效物理刻度，已触发刻度通过序号集合防重复。恢复专注时原封不动接回冻结秒数，修改默认时长绝不篡改已冻结时间。
5. **会话影响原则（Session-Impact Principle）与菜单置灰双向防护**：
   - **设定边界**：区分“会话运行规则”与“独立展现/后续默认值”。若修改某项设置会改变当前会话的流转、刻度或统计规则（如微休息参数、超时正计时开关、计数开关），在活动会话（含运行、暂停及等待状态）期间严格锁定；若仅影响外观（配色、字体、签名）、提醒方式（音效、弹窗），或仅作为后续新会话的默认值（默认时长、默认下一阶段），则保持可调并即时生效。
   - **双向防护闭环**：菜单视觉置灰（`AppendMenu` 传入 `MF_GRAYED`）与命令派发执行（`WM_COMMAND`）必须共用同一套纯函数谓词（如 `timer_setting_locked()` 与 `timer_can_edit_time()`），杜绝仅靠 UI 置灰被快捷键或底层消息绕过执行。

---

## 六、全屏 HUD 几何排版：基于关键分隔符的屏幕中心绝对锚定法则

### 1. 痛点：整体居中导致的动态水平抖动
- 在全屏大字 HUD 中，若对 `[模式名 · 状态]` 采取常规的整行水平居中，当计时器在运行态（无后缀）与暂停态（带“ · 暂停”）之间切换时，由于整行字符串像素宽度剧烈改变，导致左侧的模式名在屏幕上左右晃动，破坏视觉稳定性与专注感。

### 2. 解决方案：中心字符绝对中轴对齐（`fs_status_layout`）
1. **以分隔符物理中心对齐屏幕横向中心**：
   - 包含分隔符（`·`）时，测量分隔符自身宽度 `dot_width`，将其绝对位置定死在 `X = ScreenCenter - dot_width / 2`。
   - 左侧模式名右对齐至分隔符左边距，右侧状态名左对齐至分隔符右边距，两侧间距严格对称；无分隔符时自动回退为整行常规居中。
2. **两侧最大外延（`half_extent`）与防溢出动态缩放**：
   - 计算两侧最大向外延伸尺寸：`half_extent = max(ScreenCenter - Left, Right + RightWidth - ScreenCenter)`。
   - 当 `half_extent` 超过屏幕安全可用宽度（如 `width * 0.4`）时，使用 `MulDiv(label_size, available, half_extent)` 动态按比例降级字号重算，保证在超小屏幕、竖屏或高 DPI 下绝对不超出边界，且分隔符中心始终牢牢锁死在屏幕中轴。

---

## 七、Windows 桌面自动化测试工程化：系统全局 API 沙箱化与确定性隔离

### 1. 痛点与环境污染风险
- 涉及 Windows 托盘深色菜单、手写笔对齐（`SPI_SETMENUDROPALIGNMENT`）、全局输入钩子（`WH_MOUSE_LL` / `WH_KEYBOARD_LL`）的自动化测试，若直接在测试用例中调用真实 Windows API，会产生两大隐患：
  1. **污染宿主系统全局配置**：测试如果将系统手写笔对齐写入注册表/系统参数，会导致用户日常使用中所有软件的右键菜单都变成左撇子/右撇子异常对齐；
  2. **物理输入干扰导致测试偶发失败（Flaky Tests）**：自动化测试模拟合成拖拽与点击手势时，宿主机上真实的物理鼠标微小移动或按键会抢占全局钩子，导致合成状态机被打断。

### 2. 工业级确定性隔离测试模式
1. **系统全局 API 模拟拦截（API Sandboxing）**：
   - 在测试框架中通过预编译宏将 `SystemParametersInfoW` 重定向至测试代理桩：
     ```c
     static BOOL WINAPI test_system_parameters(UINT action, UINT parameter, PVOID value, UINT flags) {
         if (action == SPI_SETMENUDROPALIGNMENT) return TRUE; /* 严禁自动化测试改动全局手写笔对齐 */
         return SystemParametersInfoW(action, parameter, value, flags);
     }
     #define SystemParametersInfoW test_system_parameters
     ```
   - 同样拦截 `TrackPopupMenuEx`，将模态菜单交互旁路化，测试直接通过分发 `WM_COMMAND` 校验逻辑，避免测试进程卡死在模态消息循环中。
2. **底层输入钩子直通（Hook Passthrough）**：
   - 拦截 `SetWindowsHookExW`：当测试注册低级鼠标/键盘钩子时，将其回调替换为只调用 `CallNextHookEx` 的无害直通桩，阻断外部物理桌面输入对内部合成手势状态机（`td.gesture`）的干扰。
3. **独立离屏虚拟桌面（Invisible Desktop Sandbox）**：
   - 测试启动时通过 `CreateDesktopW(L"PomodoroTestDesktop", ...)` 创建专属隐藏桌面，将测试线程通过 `SetThreadDesktop` 绑定至该桌面。
   - 所有全屏黑幕窗口、任务栏检测均在该虚拟桌面内创建与校验，完全不影响用户正在工作的真实屏幕，测试退出时统一销毁。

