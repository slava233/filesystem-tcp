#include "helpers.hpp"

#include <algorithm>

namespace fstcp {
    std::string trim(const std::string &s) {
        auto is_space = [](unsigned char c) { return std::isspace(c) != 0; };

        auto begin = std::find_if_not(s.begin(), s.end(), is_space);
        auto end = std::find_if_not(s.rbegin(), s.rend(), is_space).base();

        return begin >= end ? "" : std::string(begin, end);
    }

    std::string to_lower(const std::string &s) {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return result;
    }

    std::pair<std::string, std::string> split_command(const std::string &line) {
        std::size_t pos = 0;
        while (pos < line.size() && !std::isspace(static_cast<unsigned char>(line[pos]))) {
            pos++;
        }

        std::string command = line.substr(0, pos);
        std::string rest = (pos < line.size()) ? trim(line.substr(pos)) : "";

        return {command, rest};
    }
}
