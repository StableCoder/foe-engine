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

#ifndef FOE_LOG_SINK_HPP
#define FOE_LOG_SINK_HPP

#include <foe/export.h>
#include <foe/log/category.hpp>
#include <foe/log/level.hpp>

#include <string_view>

class foeLogSink {
  public:
    virtual ~foeLogSink() = default;

    virtual void log(foeLogCategory *pCategory, foeLogLevel level, std::string_view message) = 0;

    virtual void exception() = 0;
};

#endif // FOE_LOG_SINK_HPP