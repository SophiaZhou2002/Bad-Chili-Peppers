#pragma once

#ifndef ERROR_LOG_HPP
#define ERROR_LOG_HPP

#include <iostream>
#include <sstream>
#include "../common.hpp"

// Define a macro to enable or disable error logging
#define ERROR_LOGGING_ENABLED

class ErrorLog {
public:
    // Constructor that captures file and line information
    ErrorLog(const char* file, int line) {
        #ifdef ERROR_LOGGING_ENABLED
        buffer << "[" << get_file_name(file) << ":" << line << "]\033[0m";
        #endif
    }
    
    ~ErrorLog() {
        #ifdef ERROR_LOGGING_ENABLED
        std::cerr << "\033[1;31m(error) " << buffer.str() << std::endl;
        #endif
    }
    
    // Overload the << operator to capture output
    template<typename T>
    ErrorLog& operator<<(const T& value) {
        #ifdef ERROR_LOGGING_ENABLED
        buffer << value;
        #endif
        return *this;
    }
    
private:
    std::ostringstream buffer; // Buffer to store the log message
};

// Macro to simplify usage; it automatically passes __FILE__ and __LINE__
#define ERROR_LOG ErrorLog(__FILE__, __LINE__)

#endif // ERROR_LOG_HPP
