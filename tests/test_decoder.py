"""Focused regression tests for the 8086 decoder. Run: python3 -m unittest discover -s tests -v"""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "instruction_decoder.c"


class DecoderRegressionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.tmp.cleanup)
        cls.exe = Path(cls.tmp.name) / "instruction_decoder"
        subprocess.run(
            ["clang", "-std=c11", "-Wall", "-Wextra", "-Wpedantic", "-g",
             "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
             str(SOURCE), "-o", str(cls.exe)], check=True,
        )

    def run_decoder(self, data):
        f = Path(self.tmp.name) / "input.bin"
        f.write_bytes(data)
        return subprocess.run([str(self.exe), str(f)], capture_output=True, text=True)

    def test_existing_decode_2(self):
        expected = "\n".join([
            "bits 16", "", "mov cx, bx", "mov ch, ah", "mov dx, bx",
            "mov si, bx", "mov bx, di", "mov al, cl", "mov ch, ch",
            "mov bx, ax", "mov bx, si", "mov sp, di", "mov bp, ax", "",
        ])
        f = ROOT / "decode_2"
        result = self.run_decoder(f.read_bytes())
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, expected)

    def test_register_to_register_direction_1(self):
        result = self.run_decoder(bytes([0x8B, 0xC1]))
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("mov ax, cx\n", result.stdout)

    def test_register_to_register_direction_0(self):
        result = self.run_decoder(bytes([0x89, 0xC1]))
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("mov cx, ax\n", result.stdout)

    def test_register_to_memory_with_negative_displacement(self):
        result = self.run_decoder(bytes([0x89, 0x40, 0xFE]))
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("mov [bx + si - 2], ax\n", result.stdout)

    def test_memory_to_register_with_negative_displacement(self):
        result = self.run_decoder(bytes([0x8B, 0x40, 0xFE]))
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("mov ax, [bx + si - 2]\n", result.stdout)

    def test_16bit_displacement_minimum(self):
        result = self.run_decoder(bytes([0x8B, 0x80, 0x00, 0x80]))
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("mov ax, [bx + si - 32768]\n", result.stdout)

    def test_immediate_register(self):
        result = self.run_decoder(bytes([0xB8, 0x34, 0x12]))
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("mov ax, 4660\n", result.stdout)

    def test_jump(self):
        result = self.run_decoder(bytes([0x74, 0xFE]))
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("je $+0\n", result.stdout)

    def test_jump_max_positive_offset(self):
        result = self.run_decoder(bytes([0x74, 0x7F]))
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("je $+129\n", result.stdout)

    def test_unknown_opcode_is_an_error(self):
        result = self.run_decoder(bytes([0x90, 0x00]))
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Unsupported opcode 0x90", result.stderr)

    def test_truncated_jump_is_an_error(self):
        result = self.run_decoder(bytes([0x74]))
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Truncated jump instruction", result.stderr)


if __name__ == "__main__":
    unittest.main()
