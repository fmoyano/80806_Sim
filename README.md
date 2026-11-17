# 8086 Instruction Decoder

A small, experimental **Intel 8086 machine-code disassembler** written in C. It reads raw instruction bytes from a binary file and prints NASM-style 16-bit assembly.

The decoder implements selected encodings of `MOV`, `ADD`, `SUB`, and `CMP`, including register, immediate, and memory operands with displacements. It also contains preliminary support for relative `JE` jumps.

## Usage

Compile `instruction_decoder.c` with Microsoft Visual C (the source uses MSVC-specific secure CRT functions), then run:

```text
instruction_decoder.exe decode_2
```

The repository includes small binary inputs (`decode_1`, `decode_2`, and `test`) and corresponding `.asm` files for comparison.

**Status:** Work in progress. Instruction coverage and error handling are incomplete. This is a decoder, **not** a CPU emulator or instruction executor.
