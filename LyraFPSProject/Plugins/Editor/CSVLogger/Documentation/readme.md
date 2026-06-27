##  CSVLoggerPlugin for Unreal Engine 5  
**Version:** 1.0

**Engine Version:** 5.4.4

**Category:** Utilities

**Author:** Moon

---


### Overview  

CSVLogger 是一款轻量级的 Unreal Engine 插件，可以在运行时将指定 Actor 的 Location 和 Rotation 数据记录到 CSV 文件中，以便分析或调试。设计简单易用，主要面向非程序开发者在 Blueprint 中直接调用。  

-- CSVLogger is a lightweight Unreal Engine plugin that records an Actor’s Location and Rotation into a CSV file during runtime for analysis or debugging. It is designed to be simple and convenient for Blueprint users, especially those unfamiliar with C++.


### Main Features:
1. 记录 Actor 的 Transform 
-- Records Actor transforms  
2. 将日志保存为 CSV 文件 
-- Saves logs as CSV files  
3. 对性能影响极小 
-- Minimal performance impact
4. 工具化设计，调用方便 
-- Easy-to-use utility design

---

### Installation

1. 将 `CSVLogger` 文件夹复制到项目的 `Plugins` 目录。
-- Copy the `CSVLogger` folder into your project’s `Plugins` directory.
2. 打开 Unreal Engine 编辑器。
-- Open the Unreal Engine Editor.
3. 在 **Edit → Plugins** 中找到并启用 **CSVLogger**。
-- Go to **Edit → Plugins** and enable **CSVLogger**.
4. 若出现提示，请重启编辑器。
-- Restart the editor if prompted.

---

### Blueprint Usage Example

1. 打开需要记录数据的目标 Actor 蓝图，或任何能够获取目标 Actor 引用的蓝图
-- Open the target Actor Blueprint or any Blueprint that can access the Actor reference.
2. 在 BeginPlay 中调用 InitCSVLogger，可选择是否自定义 CSV 文件名。
-- Call InitCSVLogger in BeginPlay, optionally specifying a custom CSV file name.

![No Name Init](pic1.png)

![Have Name Init](pic2.png)

3. 当需要记录 Actor 信息时，调用 LogActorTransform，并指定目标 Actor。示例展示为每 0.5 秒记录一次自身信息。
-- Call LogActorTransform whenever a record is needed. The example logs its own data every 0.5 seconds.

![LogActorTransform](pic3.png)

4. 完成所有记录后调用 SaveCSV 保存文件。可以通过按键触发、结束前统一保存，也可以在每次记录时立即保存。
--  Call SaveCSV to write the collected data. You may bind it to a key, save once at the end, or save on every log.

![SaveCSV1](pic4.png)

![SaveCSV2](pic5.png)

5. 最终生成的 CSV 文件位于项目目录下：`Saved/CSVData`。
-- The final CSV file will be stored in: `Saved/CSVData`.

![CSVFile](pic6.png)

---

### Function Descriptions


#### InitCSVLogger  
初始化 CSV 日志器。  
可选传入自定义文件名（如 “Log.csv”）。  
若未提供文件名，将自动使用当前时间戳作为文件名（如 “2025-11-13_16-32-22.csv”）。  
参数：FileName — 可选的 CSV 文件名。  

Initializes the CSV logger.  
Optionally accepts a custom file name (e.g., “Log.csv”).  
If no name is provided, a timestamp-based file name will be generated automatically (e.g., “2025-11-13_16-32-22.csv”).  
Parameter: FileName — Optional custom CSV file name.

---

#### LogActorTransform  
记录指定 Actor 的 Transform。  
每次调用都会向 CSV 文件追加一条记录，包含 Location 和 Rotation。  
参数：Actor — 需要记录的目标 Actor。  

Logs the current transform of the specified Actor.  
Each call appends a new entry containing the Actor's location and rotation.  
Parameter: Actor — The Actor whose transform should be recorded.

---

#### SaveCSV  
保存当前所有记录到 CSV 文件。  
可在运行中的任意时间调用。  
可以选择仅在结束时一次性保存，或在每次记录时立即保存。  

Saves all current log data to the CSV file.  
Can be called manually at any time during runtime.  
May be used to save once at the end of the session, or after each added record.

