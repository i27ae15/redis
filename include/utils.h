#pragma once
#include <iostream>
#include <vector>
#include <string>

// Define color codes as macros
#define RESET       "\033[0m"
#define BLACK       "\033[30m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define CYAN        "\033[36m"
#define WHITE       "\033[37m"
#define BOLD        "\033[1m"
#define UNDERLINE   "\033[4m"
#define PINK        "\033[95m"
#define ORANGE      "\033[38;5;214m"


// Utility macros for easy colored output
#define PRINT_SUCCESS(text)      std::cout << GREEN << text << RESET << "\n"
#define PRINT_WARNING(text)      std::cout << YELLOW << text << RESET << "\n"
#define PRINT_ERROR(text)        std::cout << RED << text << RESET << "\n"
#define PRINT_HIGHLIGHT(text)    std::cout << PINK << text << RESET << "\n"
#define PRINT_COLOR(color, text) std::cout << color << text << RESET << "\n"

namespace RemusUtils {
    std::vector<std::string> splitString(const std::string& str, const std::string& delimiter);
}
