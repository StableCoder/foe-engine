// Copyright (C) 2020-2022 George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#ifndef FOE_COMMAND_STRING_RUNNER_HPP
#define FOE_COMMAND_STRING_RUNNER_HPP

#include <foe/export.h>

#include <functional>
#include <mutex>
#include <string_view>
#include <unordered_map>

/** @brief Object that stores a set of string-based runnable commands
 *
 * Any good application will have a mixture of appropriate ways to interact with it. Sometimes some
 * just in UI, sometimes in text, sometimes both.
 *
 * This class is meant to be where such string-based command input can be entered along with the
 * function calls to process them. The 'commandName' is meant to be a unique string that maps to the
 * function, and be the first entered on incoming console input.
 *
 * For example in `updateTime 10 9` would be the full passed in string to 'runCommand' function and
 * 'updateTime' would be the unique commandName that would be searched for and if found, the
 * function associated with it would be called.
 */
class FOE_EXPORT foeCommandStringRunner {
  public:
    // Function call type for when a command finds a match
    using CommandFn = std::function<void(std::string_view)>;

    /// Default constructor
    foeCommandStringRunner();

    /// Default destructor
    virtual ~foeCommandStringRunner();

    /** @brief Register a unique command name and the associated function to call
     * @param commandName Unique command name that would trigger the function call
     * @param commandFn Function called when the command string is used
     * @return True if the function was registered for use. False if the commandName is already
     * registered
     */
    bool registerCommand(std::string_view commandName, CommandFn &&commandFn);

    /** @brief Deregister a unique command name and the associated function to call
     * @param commandName Unique command name that would trigger the function call
     * @return True if the function was deregistered for use. False if the command name wasn't found
     * and thus not deregistered
     */
    bool deregisterCommand(std::string_view commandName);

    /** @brief Given the string, attempts to find and run an associated function
     * @param commandCall Full string to try to find a registered command for
     * @return True if a command was found and run, false otherwise.
     */
    virtual bool runCommand(std::string_view commandCall);

  private:
    /// Synchronizes access to the command map
    FOE_NO_EXPORT std::mutex mSync;
    /// Map of command names and the associated functions to call
    FOE_NO_EXPORT std::unordered_map<std::string, CommandFn> mCommandMap;
};

#endif // FOE_COMMAND_STRING_RUNNER_HPP