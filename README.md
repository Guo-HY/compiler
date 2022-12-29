# compiler

- 中间代码使用 llvm ir，生成目标代码为MIPS32。
- 优化包括：mem2reg, rmblk, dce, gcp, 运算强度削弱, 图着色寄存器分配。
