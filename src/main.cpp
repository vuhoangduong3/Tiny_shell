#include "shell.hpp"
#include <windows.h>
#include<iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include "shell.hpp"
int main(void) {
    std::string input;
    
    // Vòng lặp giữ cho shell hoạt động liên tục
    while (true) {
        std::cout << "TinyShell> "; 
        std::getline(std::cin, input);
        
        if (input == "exit" || input == "quit") {
            break; // Thoát shell
        }
        
        // Gọi hàm xử lý lệnh của bạn ở đây
        // Ví dụ: handle_process_command(input.c_str());
    }
    PrintBanner();
    return 0;
}