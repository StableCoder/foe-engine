/*
    Copyright (C) 2022 George Cave.

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

#ifndef FOE_SIMULATION_TYPE_DEFS_H
#define FOE_SIMULATION_TYPE_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This macro is used to help diferentiate different binaries/plugin's functionality set.
 *
 * For the C-interface to work, some enum values, such as 'structure types' need to be unique across
 * all disparate compilations, but this cannot be easily assured normally. This handy macro, when
 * given a value between 0 and 1.000.000 will return an ID which should be the basis for enum block
 *of values which should be considered reserved for that functionality to use.
 *
 * For example, if AUD_FUN calls FOE_SIMULATION_FUNCTIONALITY_ID(1024), then AUD_FUN should should
 * consider itself to have free reign to use values 1.001.024.000 to 1.001.024.999. This AUD_FUN
 *value should then be passed when the functionality is registered, so that it can be determined if
 *this value is shared with any other set of loaded functionality, and be rejected if clashes could
 * occur.
 *
 * If functionality is rejected for this reason, then choosing a different value (there are 990.000
 * to choose from).
 *
 * @warning 0 is reserved for the main application and should not be used for non-application
 *functionality
 * @warning 1 - 9.999 is reserved for FoE-developed functionality and should not be used by others.
 **/
#define FOE_SIMULATION_FUNCTIONALITY_ID(X) 1000000000 + (X * 1000)

#ifdef __cplusplus
}
#endif

#endif // FOE_SIMULATION_TYPE_DEFS_H