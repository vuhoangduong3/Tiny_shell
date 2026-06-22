#pragma once

#include <string>

// Xử lý các lệnh built-in: help, exit, date, time, dir, path, addpath.
// Trả về:
//   0: Nếu lệnh là `exit` (báo hiệu shell nên dừng).
//   1: Nếu lệnh là built-in và đã được xử lý xong.
//  -1: Nếu không phải lệnh built-in (chuyển tiếp cho module khác xử lý).
int HandleBuiltinCommand(const std::string &Cmd);
