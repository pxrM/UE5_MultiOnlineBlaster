---
name: ue-safe-edit
description: 修改本 UE 项目的 C++/Slate 源码（Plugins/UMGStateConfig 等）时使用。当编辑此处源文件、或用户反馈改代码时一直弹窗 / 出现乱码时触发。涵盖显示层乱码陷阱、避免弹窗、损坏恢复流程。Slate 锚点技巧见 slate-edit-anchor。
---

# 本 UE 项目安全编辑指南

## 1. 优先用不开 shell 的工具（避免弹窗）

Windows 上每次调用 `Bash` 工具都会 spawn 一个控制台进程 → 屏幕闪一下窗口。用户反感这个。**这是硬规则，不是建议**：默认不用 `Bash`。

### 强制映射（想干左边 → 只准用右边，禁 Bash）

| 想干的事 | 用这个工具 | 禁用 |
|----------|-----------|------|
| 找文件 / 看目录结构 | `Glob`、`codegraph_files` | ❌ `ls`、`find`、`dir`、`tree` |
| 搜内容 / 找符号 | `Grep`、`codegraph_search` | ❌ `grep`、`rg`、`findstr` |
| 读文件 / 看片段 | `Read` | ❌ `cat`、`head`、`tail`、`type` |
| 改文件 | `Edit`、`Write` | ❌ `sed`、`echo >`、`>>` |
| 探架构 / 理解代码 | `codegraph_context`、`Read` | ❌ 一切 Bash 探查 |

### 唯一准用 Bash 的场景

1. **字节级修复**（乱码恢复，见第 3 节）—— 无替代
2. **编译 / 跑构建** —— 无替代；调用前先口头告知用户「这步会弹窗」
3. 用户明确要求跑某命令

准用时：多条命令合并成**一次** `Bash` 调用，少闪几次。

### 铁律

动手前先自问：「这步能用 Glob/Grep/Read/Edit 吗？」能 → 就用，**绝不**为图快用 Bash 跑 `ls`/`find`/`cat`。探目录、找文件、读代码一律零弹窗完成。哪怕只是开头想快速看下项目结构 —— 也用 `Glob`/`codegraph_files`，不用 `ls`/`find`。

## 2. 显示层乱码陷阱（关键）

显示层会把磁盘上的大写字母 **M**（字节 0x4D）渲染成一串形如 `Users\userXXXXXXXX\projects` 的乱码标记（用户 home 路径）。这影响**所有**展示出来的工具结果：`Read`、`Grep`、`Bash` 输出，甚至用户粘贴的编译日志。

> ⚠️ **标记里的 ID 每个会话不同**。别写死某个具体串。见过的有 `userb57cab92`、`usera62d35af` …。检测/恢复一律用会话无关的正则：`user[0-9a-f]{6,}`，或整段 `Users\...\projects` 形状。

后果：
- 给你看的文件路径是乱码（如 `UMGStateConfig` 显示成乱码）。但工具**匹配**在乱码空间进行，所以照着你看到的复制 `old_string` 仍能匹配磁盘。
- 你写 `new_string` / `Write` 内容时，你想要的 **M** 通常会正确写成真 0x4D — 但偶发抽风，会把字面乱码标记写进去，损坏文件。
- `Read`/`Grep` 输出里，损坏的 token 和正确的 token 长得一模一样，肉眼分不出。

### 规则
- 不含大写 M 的新代码绝对安全 — 不可能被损坏。能用不含大写 M 的写法/标识符就优先用。
- 任何引入了大写-M token 的编辑之后，用 `Grep` 工具（不弹窗）搜正则 `user[0-9a-f]{6,}` 验证。零命中 = 干净。有命中 = 有损坏要修。（会话 ID 会变，别搜固定串。）
- 别逐字节相信粘贴的编译报错文本，它也被乱码化了。把第一条真错（如 `C2017 非法转义序列`）当作真正损坏点，后面的报错多半是级联噪音。

## 3. 损坏恢复

若 `Grep` 搜 `user[0-9a-f]{6,}` 有命中，说明本该是大写 **M** 的地方变成了字面 home-路径标记字节。用一次 Python `Bash` 调用修复 —— **正则自动侦测**变化的会话 ID，不写死标记串；用**相对路径** `Plugins/...`（cwd 已在项目根）避免打字含 **M** 的绝对路径；替换目标用 `bytes([77])`（0x4D = 大写 M）不打字 **M**：

```bash
python - <<'PY'
import os, re
bs = chr(92).encode()
# 侦测 Users\<会话ID>\projects 整段，替回单个大写 M
pat = re.compile(rb'Users' + re.escape(bs) + rb'user[0-9a-f]+' + re.escape(bs) + rb'projects')
base = "Plugins"   # 按需缩到具体插件目录，如 Plugins/UIFramework
fixed = []
for root, _, files in os.walk(base):
    for f in files:
        if f.endswith(('.h', '.cpp', '.cs')):
            p = os.path.join(root, f)
            d = open(p, 'rb').read()
            n = len(pat.findall(d))
            if n:
                open(p, 'wb').write(pat.sub(bytes([77]), d))
                fixed.append((p, n))
for p, n in fixed:
    print("fixed", n, p)
print("total_files_fixed", len(fixed))
PY
```

自动扫全目录、逐文件替换、报告命中。确认有 `fixed` 行后，再用 `Grep` 搜 `user[0-9a-f]{6,}` 返回零。二进制读写保留 CRLF 和其他所有字节。

> 注意：脚本只替换标记字节，磁盘上真正的大写 M（渲染正常的）不动 —— 安全。执行前若 cwd 不确定，`cd` 到项目根一次（合并进同一次 Bash 调用）。

---

改 Slate 声明式 UI 的锚点/图标技巧，见 [slate-edit-anchor] skill。
