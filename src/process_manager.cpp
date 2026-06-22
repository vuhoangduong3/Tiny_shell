#include "process_manager.hpp"
#include "shell.hpp" 
#include <tlhelp32.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <cctype>
#include <sstream>

// Khởi tạo biến toàn cục đã khai báo extern ở file .hpp
std::vector<BackgroundProcess> background_processes;

// CÁC HÀM HỖ TRỢ XỬ LÝ CHUỖI VÀ CHUYỂN ĐỔI

static std::string trim_command(const std::string &s) {
    const std::string whitespace = " \t\n\r";
    size_t start = s.find_first_not_of(whitespace);
    if (start == std::string::npos) return std::string();
    size_t end = s.find_last_not_of(whitespace);
    return s.substr(start, end - start + 1);
}

static std::string first_token(const std::string &s) {
    std::istringstream iss(s);
    std::string token;
    if (!(iss >> token)) return std::string();
    return token;
}

static std::string command_args(const std::string &s) {
    std::istringstream iss(s);
    std::string token;
    if (!(iss >> token)) return std::string();
    std::string rest;
    std::getline(iss, rest);
    return trim_command(rest);
}

static std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

static bool parse_pid(const std::string &text, uint32_t &pid) {
    std::string value_text = trim_command(text);
    if (value_text.empty()) return false;

    char *end = nullptr;
    unsigned long value = std::strtoul(value_text.c_str(), &end, 10);
    
    while (end != nullptr && (*end == ' ' || *end == '\t')) end++;

    if (end == value_text.c_str() || (end != nullptr && *end != '\0') || value == 0) {
        return false;
    }

    pid = static_cast<uint32_t>(value);
    return true;
}


// LOGIC QUẢN LÝ TIẾN TRÌNH (PROCESS MANAGER)

void cleanup_finished_processes() {
    for (int i = static_cast<int>(background_processes.size()) - 1; i >= 0; i--) {
        DWORD exit_code = 0;
        HANDLE process_handle = background_processes[i].process_handle;
        
        if (GetExitCodeProcess(process_handle, &exit_code) && exit_code != STILL_ACTIVE) {
            CloseHandle(process_handle);
            CloseHandle(background_processes[i].thread_handle);
            if (background_processes[i].job_handle != NULL) {
                CloseHandle(background_processes[i].job_handle);
            }
            background_processes.erase(background_processes.begin() + i);
        }
    }
}

static int find_process_index(uint32_t pid) {
    cleanup_finished_processes();
    for (size_t i = 0; i < background_processes.size(); i++) {
        if (background_processes[i].pid == pid) {
            return i;
        }
    }
    return -1;
}

static bool contains_pid(const std::vector<uint32_t> &pids, uint32_t pid) {
    for (uint32_t item : pids) {
        if (item == pid) return true;
    }
    return false;
}

static std::vector<uint32_t> collect_process_tree_pids(uint32_t root_pid) {
    std::vector<uint32_t> pids;
    pids.push_back(root_pid);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return pids;

    std::vector<PROCESSENTRY32> all_processes;
    PROCESSENTRY32 pe = {};
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &pe)) {
        do {
            all_processes.push_back(pe);
        } while (Process32Next(snapshot, &pe));
    }
    CloseHandle(snapshot);

    for (size_t i = 0; i < pids.size(); i++) {
        for (const auto &proc : all_processes) {
            if (proc.th32ParentProcessID == pids[i] && !contains_pid(pids, proc.th32ProcessID)) {
                pids.push_back(proc.th32ProcessID);
            }
        }
    }
    return pids;
}

void print_running_processes() {
    cleanup_finished_processes();

    if (background_processes.empty()) {
        std::printf("  Không có background process nào đang chạy.\n");
        return;
    }

    std::printf("  Danh sách background process:\n");
    std::printf("  %-10s %-12s %s\n", "PID", "Status", "Cmd Name");
    std::printf("  %-10s %-12s %s\n", "----------", "------------", "----------------");
    for (const auto &proc : background_processes) {
        std::printf("  %-10u %-12s %s\n",
                    proc.pid,
                    proc.is_stopped ? "Stopped" : "Running",
                    proc.command);
    }
}

static bool suspend_or_resume_process_threads(uint32_t pid, bool suspend) {
    std::vector<uint32_t> process_tree = collect_process_tree_pids(pid);
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return false;

    THREADENTRY32 thread_entry = {};
    thread_entry.dwSize = sizeof(THREADENTRY32);

    bool found_thread = false;
    bool success = true;

    if (Thread32First(snapshot, &thread_entry)) {
        do {
            if (!contains_pid(process_tree, thread_entry.th32OwnerProcessID)) {
                continue;
            }

            found_thread = true;
            HANDLE thread_handle = OpenThread(THREAD_SUSPEND_RESUME, FALSE, thread_entry.th32ThreadID);
            if (thread_handle == nullptr) {
                success = false;
                continue;
            }

            DWORD result = suspend ? SuspendThread(thread_handle) : ResumeThread(thread_handle);
            if (result == static_cast<DWORD>(-1)) {
                success = false;
            }
            CloseHandle(thread_handle);
            
        } while (Thread32Next(snapshot, &thread_entry));
    } else {
        success = false;
    }

    CloseHandle(snapshot);
    return found_thread && success;
}

static void kill_process(uint32_t pid) {
    int index = find_process_index(pid);
    if (index == -1) {
        std::printf("  [kill] Không tìm thấy PID %u trong danh sách.\n", pid);
        return;
    }

    HANDLE process_handle = background_processes[index].process_handle;
    HANDLE thread_handle = background_processes[index].thread_handle;
    HANDLE job_handle = background_processes[index].job_handle;

    bool terminated = false;
    if (job_handle != NULL) {
        terminated = TerminateJobObject(job_handle, 1);
    }
    if (!terminated) {
        terminated = TerminateProcess(process_handle, 1);
    }

    if (!terminated) {
        std::printf("  [kill] Lỗi không thể terminate PID %u.\n", pid);
        return;
    }

    WaitForSingleObject(process_handle, 1000);
    CloseHandle(process_handle);
    CloseHandle(thread_handle);
    if (job_handle != NULL) CloseHandle(job_handle);

    background_processes.erase(background_processes.begin() + index);
    std::printf("  [kill] Đã terminate PID %u.\n", pid);
}

static void stop_process(uint32_t pid) {
    int index = find_process_index(pid);
    if (index == -1) {
        std::printf("  [stop] Không tìm thấy PID %u trong danh sách.\n", pid);
        return;
    }

    if (background_processes[index].is_stopped) {
        std::printf("  [stop] PID %u đang ở trạng thái Stopped.\n", pid);
        return;
    }

    if (!suspend_or_resume_process_threads(pid, true)) {
        std::printf("  [stop] Không thể suspend PID %u.\n", pid);
        return;
    }

    background_processes[index].is_stopped = true;
    std::printf("  [stop] Đã suspend PID %u.\n", pid);
}

static void resume_process(uint32_t pid) {
    int index = find_process_index(pid);
    if (index == -1) {
        std::printf("  [resume] Không tìm thấy PID %u trong danh sách.\n", pid);
        return;
    }

    if (!background_processes[index].is_stopped) {
        std::printf("  [resume] PID %u vẫn đang Running.\n", pid);
        return;
    }

    if (!suspend_or_resume_process_threads(pid, false)) {
        std::printf("  [resume] Không thể resume PID %u.\n", pid);
        return;
    }

    background_processes[index].is_stopped = false;
    std::printf("  [resume] Đã resume PID %u.\n", pid);
}

void HandleProcessCommand(const char *cmd_line) {
    if (cmd_line == nullptr || cmd_line[0] == '\0') return;

    std::string command_line(cmd_line);
    std::string verb = to_lower(first_token(command_line));

    if (verb == "list") {
        print_running_processes();
        return;
    }

    uint32_t pid = 0;
    if (!parse_pid(command_args(command_line), pid)) {
        std::printf("  Cú pháp: %s <pid>\n", verb.c_str());
        return;
    }

    if (verb == "kill") kill_process(pid);
    else if (verb == "stop") stop_process(pid);
    else if (verb == "resume") resume_process(pid);
    else std::printf("  Lệnh quản lý process không hợp lệ: %s\n", verb.c_str());
}