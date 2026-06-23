#include "executor.hpp"

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>

#include "process_manager.hpp"
#include "signal_handler.hpp"

// Kiem tra neu lenh ket thuc bang &
static bool is_background_command(const std::string &cmd, std::string &cmd_without_amp)
{
    // Tim & o cuoi lenh (sau khoang trang cuoi cung)
    size_t end = cmd.find_last_not_of(" \t\r\n");
    if (end != std::string::npos && cmd[end] == '&')
    {
        cmd_without_amp = cmd.substr(0, end);
        // Loai bo khoang trang cuoi cua phan lenh
        end = cmd_without_amp.find_last_not_of(" \t\r\n");
        if (end != std::string::npos)
        {
            cmd_without_amp = cmd_without_amp.substr(0, end + 1);
        }
        return true;
    }
    return false;
}

// Thuc hien lenh foreground (shell doi process ket thuc)
static void execute_foreground(const char *cmd_line)
{
    STARTUPINFOA startup_info = {};
    PROCESS_INFORMATION process_info = {};
    HANDLE job_handle = nullptr;

    startup_info.cb = sizeof(STARTUPINFOA);

    // CreateProcessA khong thay doi cmd_line, nhung vi STARTUPINFO.lpReserved2 la NULL,
    // ta can dung CreateProcessA voi cmd_line dung la mutable hoac dung ShellExecuteA
    // De don gian, ta dung cmd.exe /c "command"
    char full_cmd[2048] = {};
    std::snprintf(full_cmd, sizeof(full_cmd), "cmd.exe /c \"%s\"", cmd_line);

    if (!CreateProcessA(
            nullptr,       // lpApplicationName
            full_cmd,      // lpCommandLine
            nullptr,       // lpProcessAttributes
            nullptr,       // lpThreadAttributes
            FALSE,         // bInheritHandles
            0,             // dwCreationFlags
            nullptr,       // lpEnvironment
            nullptr,       // lpCurrentDirectory
            &startup_info, // lpStartupInfo
            &process_info  // lpProcessInformation
            ))
    {
        DWORD error = GetLastError();
        std::printf("  Lỗi: Không thể execute command (mã lỗi: %lu)\n", error);
        return;
    }

    job_handle = CreateJobObjectA(nullptr, nullptr);
    if (job_handle != nullptr)
    {
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION job_info = {};
        job_info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        SetInformationJobObject(
            job_handle,
            JobObjectExtendedLimitInformation,
            &job_info,
            sizeof(job_info));

        if (!AssignProcessToJobObject(job_handle, process_info.hProcess))
        {
            CloseHandle(job_handle);
            job_handle = nullptr;
        }
    }

    // Doi process ket thuc
    set_foreground_process(process_info.hProcess, job_handle, process_info.dwProcessId);
    WaitForSingleObject(process_info.hProcess, INFINITE);
    bool was_cancelled = was_cancel_requested();
    clear_foreground_process();

    // Lay exit code
    DWORD exit_code = 0;
    if (GetExitCodeProcess(process_info.hProcess, &exit_code))
    {
        if (was_cancelled)
        {
            restore_console_after_cancel();
            std::printf("\n  [Ctrl+C] Đã hủy foreground process PID %lu.\n",
                        process_info.dwProcessId);
        }
        else if (exit_code != 0)
        {
            std::printf("  Process kết thúc với mã: %lu\n", exit_code);
        }
    }

    // Dong handle
    if (job_handle != nullptr)
    {
        CloseHandle(job_handle);
    }
    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);
}

// Thuc hien lenh background (shell khong doi)
static void execute_background(const char *cmd_line)
{
    STARTUPINFOA startup_info = {};
    PROCESS_INFORMATION process_info = {};
    HANDLE job_handle = nullptr;

    startup_info.cb = sizeof(STARTUPINFOA);

    char full_cmd[2048] = {};
    std::snprintf(full_cmd, sizeof(full_cmd), "cmd.exe /c \"%s\"", cmd_line);

    if (!CreateProcessA(
            nullptr,
            full_cmd,
            nullptr,
            nullptr,
            FALSE,
            CREATE_NEW_CONSOLE | CREATE_SUSPENDED,
            nullptr,
            nullptr,
            &startup_info,
            &process_info))
    {
        DWORD error = GetLastError();
        std::printf("  Lỗi: Không thể execute background command (mã lỗi: %lu)\n", error);
        return;
    }

    job_handle = CreateJobObjectA(nullptr, nullptr);
    if (job_handle != nullptr)
    {
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION job_info = {};
        job_info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        SetInformationJobObject(
            job_handle,
            JobObjectExtendedLimitInformation,
            &job_info,
            sizeof(job_info));

        if (!AssignProcessToJobObject(job_handle, process_info.hProcess))
        {
            CloseHandle(job_handle);
            job_handle = nullptr;
        }
    }

    if (ResumeThread(process_info.hThread) == static_cast<DWORD>(-1))
    {
        DWORD error = GetLastError();
        if (job_handle != nullptr)
        {
            TerminateJobObject(job_handle, 1);
            CloseHandle(job_handle);
        }
        else
        {
            TerminateProcess(process_info.hProcess, 1);
        }
        CloseHandle(process_info.hProcess);
        CloseHandle(process_info.hThread);
        std::printf("  Lỗi: Không thể start background command (mã lỗi: %lu)\n", error);
        return;
    }

    // Luu process info
    BackgroundProcess bg_proc = {};
    bg_proc.pid = process_info.dwProcessId;
    std::strncpy(bg_proc.command, cmd_line, sizeof(bg_proc.command) - 1);
    bg_proc.process_handle = process_info.hProcess;
    bg_proc.thread_handle = process_info.hThread;
    bg_proc.job_handle = job_handle;
    bg_proc.is_stopped = false;

    background_processes.push_back(bg_proc);

    std::printf("  [%u] %s\n", bg_proc.pid, cmd_line);
}

// Xu ly lenh ngoai
void HandleExternalCommand(const char *cmd_line)
{
    if (cmd_line == nullptr || cmd_line[0] == '\0')
    {
        return;
    }

    std::string cmd(cmd_line);
    std::string cmd_without_amp;

    // Kiem tra neu la lenh background
    bool is_background = is_background_command(cmd, cmd_without_amp);

    if (is_background)
    {
        execute_background(cmd_without_amp.c_str());
    }
    else
    {
        execute_foreground(cmd_line);
    }
}
