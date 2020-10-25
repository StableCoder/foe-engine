/*
    Copyright (C) 2020 George Cave.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef STDERR_SINK_HPP
#define STDERR_SINK_HPP

#include <foe/log/sink.hpp>

#include <iostream>

class StdErrSink : public foeLogSink {
    void log(foeLogCategory *pCategory, foeLogLevel level, std::string_view message) {
        std::cerr << pCategory->name() << " : " << std::to_string(level) << " : " << message
                  << "\n";
    }

    void exception() { std::cerr << std::flush; }
};

#endif // STDERR_SINK_HPP