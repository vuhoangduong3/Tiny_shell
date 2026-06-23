# 🐚 TinyShell

> **Bài tập lớn môn IT3070 — Cơ sở Tin học Ngành (CTTN K69)**
>
> Shell dòng lệnh trên Windows viết bằng C++17, hỗ trợ built-in commands, thực thi chương trình ngoài (foreground/background), quản lý tiến trình nền và chạy script `.bat`.

```
======================================
        TINY SHELL - KHOI DONG
 Go 'help' de xem danh sach cac lenh
======================================
Welcome to TinyShell! Type 'help' for a list of commands.
myShell\E:\OS\tiny_shell>
```

---

## 📋 Mục lục

- [Tính năng](#-tính-năng)
- [Kiến trúc](#-kiến-trúc)
- [Cấu trúc thư mục](#-cấu-trúc-thư-mục)
- [Build & Chạy](#-build--chạy)
- [Danh sách lệnh](#-danh-sách-lệnh)
- [Script .bat](#-script-bat)
- [Xử lý Ctrl+C](#-xử-lý-ctrlc)
- [Phân công nhóm](#-phân-công-nhóm)

---

## ✨ Tính năng

| Tính năng | Mô tả |
|-----------|--------|
| **Built-in commands** | `help`, `exit`, `date`, `time`, `dir`, `path`, `addpath` — chạy trực tiếp trong shell, không tạo tiến trình mới |
| **External commands** | Thực thi bất kỳ chương trình nào qua `CreateProcessA` + `cmd.exe /c` |
| **Foreground execution** | Shell chờ tiến trình kết thúc trước khi trả prompt |
| **Background execution** | Thêm `&` cuối lệnh → shell trả prompt ngay, tiến trình chạy ngầm |
| **Process management** | `list` / `kill` / `stop` / `resume` — quản lý toàn bộ tiến trình nền |
| **Script runner** | Chạy file `.bat` từng dòng, mỗi dòng được dispatch như lệnh người dùng gõ tay |
| **Ctrl+C handling** | Chỉ hủy foreground process, không crash shell. Tự phục hồi console mode |
| **Job Object** | Gom tiến trình con vào Job Object, `kill` / `Ctrl+C` diệt cả cây tiến trình |
| **UTF-8 support** | Console output UTF-8 qua `SetConsoleCP` / `SetConsoleOutputCP` |

---

## 🏗 Kiến trúc

```
                    ┌──────────────────┐
                    │    main.cpp      │
                    │  REPL loop       │
                    │  ReadLine →      │
                    │  DispatchCommand  │
                    └────────┬─────────┘
                             │
                    ┌────────▼─────────┐
                    │    shell.cpp     │
                    │  DispatchCommand  │
                    │  (router)        │
                    └────────┬─────────┘
                             │
            ┌────────────────┼────────────────┐
            │                │                │
   ┌────────▼──────┐ ┌──────▼───────┐ ┌──────▼───────┐
   │  builtins.cpp │ │ process_mgr  │ │ executor.cpp │
   │               │ │  .cpp        │ │              │
   │ help/exit/    │ │ list/kill/   │ │ foreground / │
   │ date/time/    │ │ stop/resume  │ │ background   │
   │ dir/path/     │ │              │ │              │
   │ addpath       │ │              │ │              │
   └───────────────┘ └──────────────┘ └──────────────┘
                                             │
                                      ┌──────▼───────┐
                                      │ script_runner│
                                      │  .cpp        │
                                      │ Đọc .bat →   │
                                      │ DispatchCmd  │
                                      └──────────────┘

   ┌─────────────────┐    ┌─────────────────┐
   │   parser.cpp    │    │ signal_handler  │
   │ Trim/Token/Args │    │  .cpp           │
   │ IsBat/IsProcess │    │ Ctrl+C handler  │
   │ IsBackground    │    │ Job Object mgmt │
   └─────────────────┘    └─────────────────┘
```

### Luồng xử lý lệnh (dispatch priority)

```
1. Built-in?     → HandleBuiltinCommand()    → return 0 (exit) / 1 (done) / -1 (not builtin)
2. Process cmd?  → HandleProcessCommand()    → list / kill / stop / resume
3. File .bat?    → HandleScriptCommand()     → đọc file, dispatch từng dòng
4. Fallback      → HandleExternalCommand()   → foreground hoặc background (có &)
```

---

## 📁 Cấu trúc thư mục

```
tiny_shell/
├── Makefile                    # Build script (MinGW)
├── README.md
├── todo.md                     # Phân công & tiến độ
├── compile_flags.txt           # Flags cho clangd/IDE
│
├── include/                    # Header files
│   ├── builtins.hpp            #   Built-in command interface
│   ├── executor.hpp            #   External command executor
│   ├── parser.hpp              #   String parsing utilities
│   ├── process_manager.hpp     #   Background process tracking
│   ├── script_runner.hpp       #   .bat file runner
│   ├── shell.hpp               #   Banner + DispatchCommand
│   └── signal_handler.hpp      #   Ctrl+C handler
│
├── src/                        # Source files
│   ├── main.cpp                #   Entry point, REPL loop
│   ├── shell.cpp               #   Banner + command dispatcher
│   ├── parser.cpp              #   Trim, tokenize, detect .bat/&
│   ├── builtins.cpp            #   help/exit/date/time/dir/path/addpath
│   ├── executor.cpp            #   CreateProcessA, foreground/background
│   ├── process_manager.cpp     #   list/kill/stop/resume + cleanup
│   ├── script_runner.cpp       #   Read .bat, dispatch line by line
│   ├── signal_handler.cpp      #   Ctrl+C, Job Object, console restore
│   └── stubs.cpp               #   (placeholder cho mở rộng)
│
├── test_scripts/               # Script .bat để test
│   ├── test1.bat               #   Test cơ bản: date, time, dir
│   └── demo.bat                #   Demo đầy đủ tính năng
│
├── build/                      # Object files (.o)
└── bin/                        # Output binary
    └── myShell.exe
```

---

## 🔨 Build & Chạy

### Yêu cầu

- **Windows** (sử dụng Win32 API: `CreateProcessA`, `CreateJobObjectA`, `SetConsoleCtrlHandler`, ...)
- **MinGW** với `g++` hỗ trợ C++17
- **mingw32-make**

### Build

```bash
mingw32-make
```

### Chạy

```bash
bin\myShell.exe
```

### Clean

```bash
mingw32-make clean
```

---

## 📖 Danh sách lệnh

### Built-in Commands

| Lệnh | Mô tả | Ví dụ |
|-------|--------|-------|
| `help` | Hiển thị danh sách lệnh | `help` |
| `exit` | Thoát shell | `exit` |
| `date` | Hiện ngày hiện tại (YYYY-MM-DD) | `date` → `Current date: 2026-06-23` |
| `time` | Hiện giờ hiện tại (HH:MM:SS) | `time` → `Current time: 20:56:10` |
| `dir [path]` | Liệt kê file/thư mục | `dir src` |
| `path` | Hiện biến PATH | `path` |
| `addpath <dir>` | Thêm thư mục vào PATH | `addpath C:\tools` |

### Process Management

| Lệnh | Mô tả | Ví dụ |
|-------|--------|-------|
| `list` | Liệt kê background processes | `list` |
| `kill <pid>` | Kết thúc (terminate) process | `kill 1234` |
| `stop <pid>` | Tạm dừng (suspend) process | `stop 1234` |
| `resume <pid>` | Tiếp tục (resume) process | `resume 1234` |

### External & Background

| Cú pháp | Mô tả | Ví dụ |
|---------|--------|-------|
| `<command>` | Chạy foreground (shell chờ) | `hostname` |
| `<command> &` | Chạy background (shell trả prompt ngay) | `notepad &` |

---

## 📜 Script .bat

TinyShell hỗ trợ chạy file `.bat` bằng cách gõ trực tiếp đường dẫn:

```
myShell> test_scripts\test1.bat
```

**Cách hoạt động:**
1. Script runner mở file, đọc từng dòng
2. Mỗi dòng được trim, bỏ BOM UTF-8 (nếu có)
3. Dòng trống được bỏ qua
4. Mỗi dòng được chuyển cho `DispatchCommand()` — xử lý giống hệt lệnh gõ tay
5. Nếu gặp `exit` trong script → thoát cả shell

**Ví dụ `test1.bat`:**
```bat
date
time
dir src
echo Script chay thanh cong!
```

---

## ⚡ Xử lý Ctrl+C

| Tình huống | Hành vi |
|------------|---------|
| Đang chạy foreground process | `Ctrl+C` → terminate process (qua Job Object nếu có) → shell tiếp tục |
| Không có foreground process | `Ctrl+C` bị bỏ qua, shell không bị crash |
| Sau khi cancel | Console mode được phục hồi tự động (`FlushConsoleInputBuffer`) |

**Cơ chế kỹ thuật:**
- Đăng ký `SetConsoleCtrlHandler` khi shell khởi động
- Foreground process được gán vào **Job Object** với flag `JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE`
- Khi `Ctrl+C`, ưu tiên `TerminateJobObject()` để diệt cả cây tiến trình con
- Sử dụng `std::atomic` để đồng bộ giữa luồng chính và luồng signal handler

---

## 👥 Phân công nhóm

| Thành viên | Module | File phụ trách |
|------------|--------|----------------|
| **Duy** | Core Framework & Parser | `main.cpp`, `shell.cpp`, `parser.cpp`, `Makefile` |
| **Huy** | Built-in Commands & PATH | `builtins.cpp` |
| **Dương** | Executor & Script Runner | `executor.cpp`, `script_runner.cpp` |
| **Hoàng** | Process Manager & Signals | `process_manager.cpp`, `signal_handler.cpp` |

---

## 📄 License

Project phục vụ mục đích học tập môn IT3070 — Đại học Bách khoa Hà Nội.
