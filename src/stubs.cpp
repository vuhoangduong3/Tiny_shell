// Stub tam — xoa khi cac nhom khac da implement xong.

#include <cstdio>
#include <string>

int HandleBuiltinCommand(const std::string &Cmd) {
    std::printf("  [stub] builtin chua duoc cai dat: %s\n", Cmd.c_str());
    return -1;
}


int HandleScriptCommand(const char *CmdLine) {
    std::printf("  [stub] script chua duoc cai dat: %s\n", CmdLine);
    return 1;
}

void HandleExternalCommand(const char *CmdLine) {
    std::printf("  [stub] external command chua duoc cai dat: %s\n", CmdLine);
}

void SetupCtrlCHandler(void) {}
