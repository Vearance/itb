# HutaOS - Tugas Besar IF2130 Sistem Operasi 2025/2026

![mascot](mascot.png)

*"I swear i will finish what you started, Old Hu". These were Hu Tao's words at her grandfather's resting place. Unfortunately, between running the funeral parlor and choking  The Traveler's head between her thighs, Hu Tao doesn't have much time to finish Old Hu's magical machinery he called an "Operating System"*

## Members:
1. 13524121 Billy Ontoseno Irawan
2. 13524122 Nathaniel Christian
3. 13524139 Azri Arzaq Pohan
4. 13524142 Rasyad Satyatma
5. 13524143 Daniel Putra Rywandi Sembiring

## Table of Contents

- [Members](#members)
- [Table of Contents](#table-of-contents)
- [How To Run](#how-to-run)
- [Features](#features)
- [Bonus](#bonus-features)

## How To Run

### Prerequisites
- **WSL** (Windows Subsystem for Linux) atau Linux
- **QEMU** - Emulator untuk menjalankan OS
- **GCC Cross Compiler** (i686-elf-gcc)
- **NASM** - Assembler
- **Make** - Build tool

### Build & Run

1. **Clone repository**
   ```bash
   git clone <repository-url>
   cd tugas-besar-os-hutaos
   ```

2. **Build OS**
   ```bash
   wsl make all
   ```

3. **Jalankan dengan QEMU**
   ```bash
   wsl make run
   ```

4. **Build ulang (clean build)**
   ```bash
   wsl make clean # Bisa juga dengan wsl make disk-reset
   wsl make all
   ```

## Features
### Commands
| Command | Deskripsi |
|---------|-----------|
| `ls`    | List directory contents |
| `cd`    | Change directory |
| `mkdir` | Create new directory |
| `rm`    | Remove file/directory |
| `cp`    | Copy file |
| `mv`    | Move/rename file |
| `cat`   | Display file contents |
| `touch` | Create empty file |
| `echo`  | Print text to screen |
| `find`  | Search for files |
| `grep`  | Search text in files |
| `ps`    | List running processes |
| `kill`  | Terminate process |
| `exec`  | Execute program |
| `bg`    | Run process in background |
| `badapple` | Play Bad Apple Animation |
| `hello` | Prints "hello world" | 

### Bonus Features
- Display Output
- Graphics Driver
- Sound Driver
- Heap Memory
- Recent Commands History
- Kreativitas (custom font, bg, echo, piping)
- Smooth Bad Apple Animation (at main branch)
- Bad Apple Animation + Sound (only at badapple-audio branch)
- Wallpapers