#include "shell.hpp"

#include <cstdio>
#include <string>

#include "parser.hpp"
#include "executor.hpp"

// ============================================================================
//  Dispatcher: chuyen tiep lenh cho 4 module xu ly.
//  Cac module nay do 3 nhom khac (Huy / Hoang / Duong) cung cap. O giai
//  doan Duy chua co file .cpp cua ho, nen ta dung extern de forward-declare.
//  Linker se tu tim cac symbol nay khi Makefile duoc cap nhat.
// ============================================================================
// 1) Built-in  — HUY (help/exit/date/time/dir/path/addpath)
//    Tra ve: 0 = gap "exit" (can thoat shell)
//            1 = da xu ly builtin
//           -1 = khong phai builtin -> chuyen module tiep theo
extern int  HandleBuiltinCommand(const std::string &Cmd);

// 2) Process  — HOANG (list / kill <pid> / stop <pid> / resume <pid>)
extern void HandleProcessCommand(const char *CmdLine);

// 3) Script   — DUONG (chay file *.bat dong mot)
//    Tra ve: 0 = gap "exit" trong script (dung ca shell)
//            1 = chay xong / loi -> tiep tuc REPL
extern int  HandleScriptCommand(const char *CmdLine);

// 4) External — DUONG (chay chuong trinh ngoai, nhan dien `&`)
// (Đã được include qua executor.hpp)

// ============================================================================
//  Banner: in gioi thieu khi shell khoi dong.
//  Giu ASCII-only de khong phu thuoc console code page.
// ============================================================================
void PrintBanner(void) {
    std::puts("======================================");
    std::puts("        TINY SHELL - KHOI DONG");
    std::puts(" Go 'help' de xem danh sach cac lenh");
    std::puts("======================================");
    std::puts("Welcome to TinyShell! Type 'help' for a list of commands.");
}

// ============================================================================
//  DispatchCommand: phan luong lenh theo thu tu uu tien.
//  Dung helper o parser.hpp de tranh goi nhieu ham trung nhau o day.
// ============================================================================
bool DispatchCommand(const std::string &Cmd) {
    // B1: thu builtin truoc (re nhat, khong can fork).
    const int Rc = HandleBuiltinCommand(Cmd);
    if (Rc == 0) {
        // exit -> bao main() dung vong lap.
        return false;
    }
    if (Rc == 1) {
        // builtin da xu ly xong.
        return true;
    }

    // B2: lenh quan ly process (list/kill/stop/resume).
    if (IsProcessCommand(Cmd)) {
        HandleProcessCommand(Cmd.c_str());
        return true;
    }

    // B3: file script .bat (co the o bat ky path nao, ke ca co khoang trang).
    if (IsBatCommand(Cmd)) {
        // Tra 0 tu script dong nghia gap "exit" -> dung shell.
        return HandleScriptCommand(Cmd.c_str()) != 0;
    }

    // B4: fallback — chay chuong trinh ngoai (foreground hoac background).
    HandleExternalCommand(Cmd.c_str());
    return true;
}
