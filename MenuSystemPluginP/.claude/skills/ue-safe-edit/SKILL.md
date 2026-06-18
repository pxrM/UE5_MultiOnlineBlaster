---
name: ue-safe-edit
description: 修改本 UE 项目的 C++/Slate 源码（Plugins/UMGStateConfig 等）时使用。当编辑此处源文件、或用户反馈改代码时一直弹窗 / 出现乱码时触发。涵盖显示层乱码陷阱、避免弹窗、损坏恢复流程。Slate 锚点技巧见 slate-edit-anchor。
---

# 本 UE 项目安全编辑指南

## 1. 优先用不开 shell 的工具（避免弹窗）

Windows 上每次调用 `Bash` 工具都会 spawn 一个控制台进程 → 屏幕闪一下窗口。用户反感这个。

- 用 `Edit`、`Read`、`Write`、`Grep`、`Glob` — 这些不 spawn shell，不弹窗。
- 不要用 `Bash` 跑 `find`/`grep`/`cat`/`sed`/`head`/`tail` — 改用对应专用工具。
- 只有实在没别的办法时（看 hex 字节、字节级修复）才退回 `Bash`，并把多条命令合并成一次调用，少闪几次。

## 2. 显示层乱码陷阱（关键）

显示层会把磁盘上的大写字母 **M**（字节 0x4D）渲染成一串含 `userb57cab92` 的乱码标记。这影响**所有**展示出来的工具结果：`Read`、`Grep`、`Bash` 输出，甚至用户粘贴的编译日志。

后果：
- 给你看的文件路径是乱码（如 `UMGStateConfig` 显示成乱码）。但工具**匹配**在乱码空间进行，所以照着你看到的复制 `old_string` 仍能匹配磁盘。
- 你写 `new_string` / `Write` 内容时，你想要的 **M** 通常会正确写成真 0x4D — 但偶发抽风，会把字面乱码标记写进去，损坏文件。
- `Read`/`Grep` 输出里，损坏的 token 和正确的 token 长得一模一样，肉眼分不出。

### 规则
- 不含大写 M 的新代码绝对安全 — 不可能被损坏。能用不含大写 M 的写法/标识符就优先用。
- 任何引入了大写-M token 的编辑之后，用 `Grep` 工具（不弹窗）搜标记子串 `userb57cab92` 验证。零命中 = 干净。有命中 = 有损坏要修。
- 别逐字节相信粘贴的编译报错文本，它也被乱码化了。把第一条真错（如 `C2017 非法转义序列`）当作真正损坏点，后面的报错多半是级联噪音。

## 3. 损坏恢复

若 `Grep` 搜 `userb57cab92` 有命中，说明本该是真 **Users\userb57cab92\projects** 的地方变成了字面标记字节。用一次 Python `Bash` 调用修复 —— 全程不打字 **Users\userb57cab92\projects**（用 `chr(77)` 生成），也不打字含 **M** 的项目路径（用 `find` 解析）：

```bash
F=$(find "<项目>/Plugins" -name SUIStateConfigPanel.cpp -path '*Editor*Private*' | head -1)
python - "$F" <<'PY'
import sys
p = sys.argv[1]
bs = chr(92)
marker = ("Users"+bs+"userb57cab92"+bs+"projects").encode('utf-8')
data = open(p,'rb').read()
n = data.count(marker)
data = data.replace(marker, bytes([77]))   # 0x4D = 大写 M
open(p,'wb').write(data)
print("markers_replaced", n)
PY
```

确认 `markers_replaced` > 0，再用 `Grep` 搜 `userb57cab92` 返回零。二进制读写会保留 CRLF 和其他所有字节。

---

改 Slate 声明式 UI 的锚点/图标技巧，见 [slate-edit-anchor] skill。
