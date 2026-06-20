// filepath: include/parser.hpp
#pragma once

#include <string>

// Parser helpers — tách riêng khỏi shell.cpp để dễ test và tái sử dụng.
// Tất cả hàm đều thuần tuý (pure), không gọi WinAPI, không in ra stdout.

// Bỏ BOM UTF-8 ở đầu, bỏ khoảng trắng, tab, \r, \n ở đầu và cuối.
std::string TrimCommand(const std::string &text);

// Lấy token đầu tiên (verb / tên lệnh).
std::string FirstToken(const std::string &line);

// Lấy phần args (mọi thứ sau token đầu, đã trim).
std::string CommandArgs(const std::string &line);

// Chuyển sang lowercase (phục vụ so sánh verb không phân biệt hoa/thường).
std::string ToLower(std::string text);

// Lấy tên lệnh — có hỗ trợ chuỗi trong dấu nháy kép.
// VD: `"C:\My Tools\app.exe" --flag` -> `C:\My Tools\app.exe`
std::string CommandNameToken(const std::string &cmd);

// True nếu token kết thúc bằng `.bat` (không phân biệt hoa/thường).
bool EndsWithBat(const std::string &token);

// True nếu command đầu là file `.bat` (kể cả khi path chứa khoảng trắng + quote).
bool IsBatCommand(const std::string &cmd);

// True nếu command đầu là một trong các lệnh quản lý process: list, kill, stop, resume.
bool IsProcessCommand(const std::string &cmd);

// True nếu lệnh kết thúc bằng `&` (background mode).
// Bỏ qua khoảng trắng trước khi kiểm tra ký tự cuối.
bool IsBackgroundCommand(const std::string &line);
