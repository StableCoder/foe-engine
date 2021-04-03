/*
    Copyright (C) 2021 George Cave.

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

#ifndef FOE_ECS_EDITOR_NAME_MAP_HPP
#define FOE_ECS_EDITOR_NAME_MAP_HPP

#include <foe/ecs/id.hpp>
#include <foe/export.h>

#include <map>
#include <shared_mutex>
#include <string>

struct foeEditorNameMap {
  public:
    FOE_EXPORT foeId find(std::string_view editorName);
    FOE_EXPORT std::string find(foeId id);

    FOE_EXPORT bool add(foeId id, std::string editorName);

    FOE_EXPORT bool update(foeId id, std::string editorName);

    FOE_EXPORT bool remove(foeId id);

  private:
    std::shared_mutex mSync{};

    std::map<std::string, foeId> mEditorToId;
    std::map<foeId, std::string> mIdToEditor;
};

#endif // FOE_ECS_EDITOR_NAME_MAP_HPP