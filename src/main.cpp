// filepath: src/main.cpp
#ifdef _WIN32
#include <windows.h>   // SetConsoleCP, GetCurrentDirectoryA, MAX_PATH
#else
#include <unistd.h>    // getcwd
#endif

#include <cstdio>
#include <cstring>
#include <optional>
#include <string>

#include "shell.hpp"
#include "parser.hpp"

extern void SetupCtrlCHandler(void);

namespace {

void ConfigureConsole(void) {
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif
}

void PrintPrompt(void) {
#ifdef _WIN32
    char cwd[MAX_PATH] = {};
    if (!GetCurrentDirectoryA(MAX_PATH, cwd)) {
        std::strcpy(cwd, "?");
    }
#else
    char cwd[1024] = {};
    if (!getcwd(cwd, sizeof(cwd))) {
        std::strcpy(cwd, "?");
    }
#endif
    std::printf("myShell\\%s>", cwd);
    std::fflush(stdout);
}

// std::nullopt  = EOF (Ctrl+D / Ctrl+Z)
// chuỗi rỗng   = dòng trống sau trim, bỏ qua
// còn lại      = lệnh cần dispatch
std::optional<std::string> ReadCommandLine(void) {
    char buffer[4096];
    if (std::fgets(buffer, sizeof(buffer), stdin) == nullptr) {
        return std::nullopt;
    }
    return TrimCommand(buffer);
}

void RunRepl(void) {
    for (;;) {
        PrintPrompt();

        const std::optional<std::string> cmd = ReadCommandLine();
        if (!cmd.has_value()) {
            break;
        }
        if (cmd->empty()) {
            continue;
        }
        if (!DispatchCommand(*cmd)) {
            break;
        }
    }
}

}  // namespace

int main(void) {
    ConfigureConsole();
    SetupCtrlCHandler();
    PrintBanner();
    RunRepl();
    return 0;
}
