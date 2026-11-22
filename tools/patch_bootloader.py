"""
Patch the ESP32-S3 Arduino (2.0.17 / IDF 4.4) 2nd-stage bootloader so that
bootloader_flash_xmc_startup() immediately returns ESP_OK.

Why: this board's flash reports JEDEC ID 0x20 70 18. The stock bootloader sees
manufacturer 0x20 (XMC), runs the XMC startup flow, then rejects the chip
because 0x70 is not in its list of known XMC parts -> "XMC flash startup fail"
-> reboot, forever. The flash itself works fine (ROM boot / esptool are OK).

Usage:
  python tools/patch_bootloader.py <bootloader_dio_80m.elf> <out.bin>
"""
import struct
import subprocess
import sys
import tempfile
import os

SYMBOL = "bootloader_flash_xmc_startup"
# After the 3-byte `entry a1, 32`: movi.n a2, 0 ; retw.n
PATCH = bytes([0x0C, 0x02, 0x1D, 0xF0])


def find_symbol(elf, name):
    shoff, = struct.unpack_from("<I", elf, 0x20)
    shentsize, shnum = struct.unpack_from("<HH", elf, 0x2E)
    secs = [struct.unpack_from("<IIIIIIIIII", elf, shoff + i * shentsize) for i in range(shnum)]
    symtab = next(s for s in secs if s[1] == 2)  # SHT_SYMTAB
    strtab = secs[symtab[6]]
    for off in range(symtab[4], symtab[4] + symtab[5], 16):
        st_name, st_value = struct.unpack_from("<II", elf, off)
        s = elf[strtab[4] + st_name:elf.index(b"\0", strtab[4] + st_name)].decode()
        if s == name:
            return st_value
    raise SystemExit(f"symbol {name} not found")


def vaddr_to_offset(elf, vaddr):
    phoff, = struct.unpack_from("<I", elf, 0x1C)
    phentsize, phnum = struct.unpack_from("<HH", elf, 0x2A)
    for i in range(phnum):
        p_type, p_offset, p_vaddr, _, p_filesz = struct.unpack_from("<IIIII", elf, phoff + i * phentsize)
        if p_type == 1 and p_vaddr <= vaddr < p_vaddr + p_filesz:
            return p_offset + (vaddr - p_vaddr)
    raise SystemExit(f"vaddr {vaddr:#x} not in any LOAD segment")


def main(src, out_bin):
    elf = bytearray(open(src, "rb").read())
    addr = find_symbol(elf, SYMBOL)
    off = vaddr_to_offset(elf, addr)
    if elf[off:off + 3] != bytes([0x36, 0x41, 0x00]):  # entry a1, 32
        raise SystemExit("unexpected function prologue - bootloader version changed?")
    elf[off + 3:off + 3 + len(PATCH)] = PATCH
    print(f"patched {SYMBOL} at {addr:#x} (file offset {off:#x})")

    with tempfile.TemporaryDirectory() as d:
        patched = os.path.join(d, "bootloader_patched.elf")
        open(patched, "wb").write(elf)
        subprocess.check_call([
            sys.executable, "-m", "esptool", "--chip", "esp32s3", "elf2image",
            "--flash_mode", "dio", "--flash_freq", "80m", "--flash_size", "16MB",
            "-o", out_bin, patched,
        ])


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
