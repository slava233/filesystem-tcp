/* Helper functions implementation.
 * FILE NAME: helpers.cpp
 * PROGRAMMER: Sviatoslav Kononov.
 * LAST UPDATE: 05.08.2026
 */

#include "helpers.hpp"

#include <algorithm>

namespace fstcp {
    /* Remove leading and tailing spaces.
     * ARGUMENTS:
     *     - line to trim:
     *         const std::string &s;
     * RETURNS:
     *     (std::string) trimmed line.
     */
    std::string trim(const std::string &s) {
        auto is_space = [](unsigned char c) { return std::isspace(c) != 0; };

        auto begin = std::find_if_not(s.begin(), s.end(), is_space);
        auto end = std::find_if_not(s.rbegin(), s.rend(), is_space).base();

        return begin >= end ? "" : std::string(begin, end);
    }

    /* Change line to lower case.
     * ARGUMENTS:
     *     - line to apply lower case:
     *         const std::string &s;
     * RETURNS:
     *     (std::string) line in lower case.
     */
    std::string to_lower(const std::string &s) {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return result;
    }

    /* Split the line into command and arguments.
     * ARGUMENTS:
     *     - line containing command and arguments:
     *         const std::string &line;
     * RETURNS:
     *     (std::pair<std::string, std::string>) command and arguments.
     */
    std::pair<std::string, std::string> split_command(const std::string &line) {
        // Find the position of first space
        std::size_t pos = 0;
        while (pos < line.size() && !std::isspace(static_cast<unsigned char>(line[pos]))) {
            pos++;
        }

        // Split the line using first space position
        std::string command = line.substr(0, pos);
        std::string rest = (pos < line.size()) ? trim(line.substr(pos)) : "";

        return {command, rest};
    }
}
