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

#ifndef FOE_LOG_CATEGORY_HPP
#define FOE_LOG_CATEGORY_HPP

#include <foe/export.h>
#include <foe/log/level.hpp>

#include <string_view>

class foeLogCategory {
  public:
    FOE_EXPORT std::string_view name() const noexcept;

    FOE_EXPORT foeLogLevel maxLevel() const noexcept;
    FOE_EXPORT void maxLevel(foeLogLevel level) noexcept;

  protected:
    FOE_EXPORT
    foeLogCategory(std::string_view name, foeLogLevel maxLevel);

  private:
    const std::string_view mName;
    foeLogLevel mMaxLevel;
};

#endif // FOE_LOG_CATEGORY_HPP