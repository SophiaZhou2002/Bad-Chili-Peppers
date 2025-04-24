#pragma once

#ifndef DEBUG_LOG_HPP

#define DEBUG_LOG_HPP

#include <iostream>
#include <sstream>

// Define a macro to enable or disable debug logging
#define DEBUG_LOGGING_ENABLED

class DebugLog {
public:
    DebugLog() = default;
    ~DebugLog() {
        #ifdef DEBUG_LOGGING_ENABLED
			std::cout << "\033[1;34m(debug) \033[0m" << buffer.str() << std::endl;
        #endif
    }

    // Overload the << operator to capture output
    template<typename T>
    DebugLog& operator<<(const T& value) {
        #ifdef DEBUG_LOGGING_ENABLED
        buffer << value;
        #endif
        return *this;
    }

private:
    std::ostringstream buffer; // Buffer to store the log message
};

// Macro to simplify usage
#define DEBUG_LOG DebugLog()

#endif // DEBUG_LOG_HPP