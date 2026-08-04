#pragma once

#include <string>

namespace fstcp {
    std::string trim(const std::string &s);
    std::string to_lower(const std::string &s);
    std::pair<std::string, std::string> split_command(const std::string &line);
}