# TinyShell - To Do

## Mục tiêu

Xây dựng một Tiny Shell bằng C++ cho Windows với các chức năng cơ bản:

- Built-in commands.
- Quản lý PATH.
- Thực thi lệnh ngoài ở chế độ foreground/background.
- Quản lý background process.
- Chạy script `.bat`.
- Bắt và xử lý `Ctrl+C` an toàn.

## Phân công công việc

### 1. Duy: Core Framework & Parser

Nhiệm vụ: dựng khung shell và xử lý input đầu vào để các module khác có dữ liệu làm việc.

File phụ trách:

- [ ] `src/main.cpp`
- [ ] `src/shell.cpp`
- [ ] `src/parser.cpp`
- [ ] `include/shell.hpp`
- [ ] `include/parser.hpp`
- [ ] `Makefile` hoặc `CMakeLists.txt`

- [x] Thiết lập cấu trúc thư mục project `include/` và `src/`.
- [x] Tạo file cấu hình biên dịch như `Makefile` hoặc `CMakeLists.txt`.
- [ ] Xây dựng vòng lặp chính của shell, in dấu nhắc lệnh và nhận input.
- [ ] Cài đặt parser để tách command và arguments.
- [ ] Cài đặt nhận diện ký tự `&` ở cuối lệnh để đánh dấu background command.

### 2. Huy: Built-in Commands & PATH

Nhiệm vụ: triển khai các lệnh chạy trực tiếp trong shell mà không cần tạo tiến trình mới.

File phụ trách:

- [ ] `src/builtins.cpp`
- [ ] `include/builtins.hpp`

- [ ] Cài đặt `help` để hiển thị danh sách lệnh.
- [ ] Cài đặt `exit` để thoát shell.
- [ ] Cài đặt `date` và `time` để lấy thời gian hệ thống.
- [ ] Cài đặt `dir [path]` để liệt kê file và thư mục.
- [ ] Cài đặt `path` để xem PATH hiện tại.
- [ ] Cài đặt `addpath <dir>` để thêm đường dẫn vào PATH.

### 3. Dương: Executor & Script Runner

Nhiệm vụ: gọi API hệ điều hành để chạy file thực thi bên ngoài và xử lý file `.bat`.

File phụ trách:

- [ ] `src/executor.cpp`
- [ ] `include/executor.hpp`
- [ ] `src/script_runner.cpp`
- [ ] `include/script_runner.hpp`

- [ ] Cài đặt `executor` để nhận dữ liệu từ parser, tìm file thực thi và tạo tiến trình mới.
- [ ] Xử lý foreground/background để shell chờ hoặc trả prompt ngay.
- [ ] Cài đặt `script_runner` để đọc file `.bat` từng dòng.
- [ ] Chuyển từng dòng script về parser để xử lý như lệnh người dùng nhập tay.

### 4. Hoàng: Process Manager & Signals

Nhiệm vụ: theo dõi tiến trình nền và xử lý tổ hợp phím điều khiển.

File phụ trách:

- [ ] `src/process_manager.cpp`
- [ ] `include/process_manager.hpp`
- [ ] `src/signal_handler.cpp`
- [ ] `include/signal_handler.hpp`

- [ ] Xây dựng cấu trúc dữ liệu để lưu danh sách background process.
- [ ] Cài đặt `list` để in danh sách process nền.
- [ ] Cài đặt `kill <pid>` để kết thúc process.
- [ ] Cài đặt `stop <pid>` để tạm dừng process.
- [ ] Cài đặt `resume <pid>` để tiếp tục process.
- [ ] Cài đặt `signal_handler` để bắt sự kiện `Ctrl+C`.
- [ ] Đảm bảo `Ctrl+C` chỉ hủy foreground process, không làm shell chính bị crash.

## Tiến độ chung

- [ ] Hoàn thiện Core & Parser.
- [ ] Tích hợp Built-ins & PATH.
- [ ] Tích hợp Executor & Process Manager.
- [ ] Kiểm thử tổng thể và xử lý lỗi bộ nhớ.

## File liên quan

- [src/main.cpp](src/main.cpp)
- [src/shell.cpp](src/shell.cpp)
- [src/parser.cpp](src/parser.cpp)
- [src/builtins.cpp](src/builtins.cpp)
- [src/executor.cpp](src/executor.cpp)
- [src/script_runner.cpp](src/script_runner.cpp)
- [src/process_manager.cpp](src/process_manager.cpp)
- [src/signal_handler.cpp](src/signal_handler.cpp)
- [include/shell.hpp](include/shell.hpp)
- [include/parser.hpp](include/parser.hpp)
- [include/builtins.hpp](include/builtins.hpp)
- [include/executor.hpp](include/executor.hpp)
- [include/script_runner.hpp](include/script_runner.hpp)
- [include/process_manager.hpp](include/process_manager.hpp)
- [include/signal_handler.hpp](include/signal_handler.hpp)