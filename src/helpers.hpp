/* Helper functions declaration.
 * FILE NAME: helpers.cpp
 * PROGRAMMER: Sviatoslav Kononov.
 * LAST UPDATE: 05.08.2026
 */

#pragma once

#include <string>

namespace fstcp {
    /* Remove leading and tailing spaces.
     * ARGUMENTS:
     *     - line to trim:
     *         const std::string &s;
     * RETURNS:
     *     (std::string) trimmed line.
     */
    std::string trim(const std::string &s);

    /* Change line to lower case.
     * ARGUMENTS:
     *     - line to apply lower case:
     *         const std::string &s;
     * RETURNS:
     *     (std::string) line in lower case.
     */
    std::string to_lower(const std::string &s);

    /* Split the line into command and arguments.
     * ARGUMENTS:
     *     - line containing command and arguments:
     *         const std::string &line;
     * RETURNS:
     *     (std::pair<std::string, std::string>) command and arguments.
     */
    std::pair<std::string, std::string> split_command(const std::string &line);
}