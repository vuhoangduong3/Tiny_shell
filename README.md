# TinyShell

TinyShell là project shell dòng lệnh trên Windows viết bằng C++. Mục tiêu của project là xây dựng một shell nhỏ có thể nhận lệnh, phân tích lệnh, chạy built-in command, thực thi chương trình ngoài, quản lý process nền và xử lý script `.bat`.

## Mục tiêu và phạm vi

Theo `todo.md`, project được chia thành 4 phần chính:

- Core framework và parser.
- Built-in commands và quản lý PATH.
- Executor và script runner.
- Process manager và signal handling.

Ở trạng thái hiện tại, code trong workspace mới ở mức khởi tạo: chương trình mở lên sẽ in banner chào mừng và dòng giới thiệu, sau đó thoát. Phần logic shell đầy đủ vẫn đang được phát triển.

## Cấu trúc thư mục

```text
tiny_shell/
├── Makefile
├── README.md
├── bin/
├── build/
├── include/
│   ├── builtins.hpp
│   ├── executor.hpp
│   ├── parser.hpp
│   ├── process_manager.hpp
│   ├── script_runner.hpp
│   ├── shell.hpp
│   └── signal_handler.hpp
├── src/
│   ├── builtins.cpp
│   ├── executor.cpp
│   ├── main.cpp
│   ├── parser.cpp
│   ├── process_manager.cpp
│   ├── script_runner.cpp
│   ├── shell.cpp
│   └── signal_handler.cpp
├── test_scripts/
└── todo.md
```

## Vai trò các module

- `src/main.cpp`: điểm vào chương trình, gọi `PrintBanner()` và in lời chào.
- `src/shell.cpp`: chứa hàm banner khởi động của TinyShell.
- `src/builtins.cpp`: nơi dự kiến triển khai các lệnh built-in như `help`, `exit`, `date`, `time`, `dir`, `path`, `addpath`.
- `src/executor.cpp`: nơi dự kiến xử lý việc chạy lệnh ngoài và logic foreground/background.
- `src/process_manager.cpp`: nơi dự kiến quản lý danh sách process nền và các lệnh như `list`, `kill`, `stop`, `resume`.
- `src/script_runner.cpp`: nơi dự kiến đọc và thực thi file `.bat`.
- `src/parser.cpp`: nơi dự kiến tách command, arguments và cờ `&`.
- `src/signal_handler.cpp`: nơi dự kiến xử lý tín hiệu như `Ctrl+C`.
- `include/`: khai báo giao diện cho từng module để các file `.cpp` có thể gọi lẫn nhau.

## Tiến độ hiện tại

Từ `todo.md`, các việc còn lại của project gồm:

- Hoàn thiện vòng lặp chính của shell.
- Cài đặt parser command và nhận diện background flag `&`.
- Cài đặt toàn bộ built-in command và quản lý PATH.
- Cài đặt executor cho lệnh ngoài và script runner cho `.bat`.
- Cài đặt process manager, signal handler và xử lý `Ctrl+C`.

Nói ngắn gọn, README này mô tả một shell đang trong giai đoạn dựng khung, chưa phải bản hoàn chỉnh.

## Build

Project hiện dùng `Makefile` cho MinGW trên Windows.

```bash
mingw32-make
bin\myShell.exe
```

Khi chạy, chương trình hiện tại sẽ:

1. In banner khởi động.
2. In dòng chào mừng.
3. Thoát ngay.

## Ghi chú

- `CMakeLists.txt` hiện chưa có nội dung.
- Một số module trong `src/` và `include/` vẫn đang là phần khung để phát triển tiếp.
- Nếu muốn biến TinyShell thành shell hoàn chỉnh, cần hoàn tất các việc trong `todo.md` rồi tích hợp dần từng module.

## Ví dụ output

```text
======================================
        TINY SHELL - KHOI DONG
 Go 'help' de xem danh sach cac lenh
======================================
Welcome to TinyShell! Type 'help' for a list of commands.
```
