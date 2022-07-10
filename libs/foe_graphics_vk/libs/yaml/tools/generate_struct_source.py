#!/usr/bin/env python3

# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

import os
import sys
import getopt
import xml.etree.ElementTree as ET
import yaml
import datetime


def main(argv):
    xmlFile = ''
    yamlFile = ''

    try:
        opts, args = getopt.getopt(argv, 'x:y:', [])
    except getopt.GetoptError:
        print('Error parsing options')
        sys.exit(1)
    for opt, arg in opts:
        if opt == '-x':
            xmlFile = arg
        elif opt == '-y':
            yamlFile = arg

    if xmlFile == '':
        print("Error: No Vulkan XML file specified")
        sys.exit(1)
    if yamlFile == '':
        print("Error: No input Yaml file specified")
        sys.exit(1)

    try:
        xmlData = ET.parse(xmlFile)
        xmlRoot = xmlData.getroot()
    except:
        print("Error: Could not open XML file: ", xmlFile)
        sys.exit(1)

    try:
        with open(yamlFile, 'r') as file:
            yamlData = yaml.safe_load(file)
    except:
        print("Error: Could not open Yaml file: ", yamlFile)
        sys.exit(1)

    # Get first/last versions
    firstVersion = xmlRoot.get('first')
    lastVersion = xmlRoot.get('last')

    # Copyright / Headers
    year = datetime.date.today().year
    if year != 2022:
        year = '2022-{}'.format(year)

    print("""// Copyright (C) {0} George Cave.
//
// SPDX-License-Identifier: Apache-2.0

#include <foe/graphics/vk/yaml/vk_enum.hpp>
#include <foe/yaml/exception.hpp>
#include <foe/yaml/parsing.hpp>
#include <vk_struct_cleanup.h>
#include <vulkan/vulkan.h>

#include <cstring>
#include <string>
""".format(year))

    # Main Processing
    for struct in xmlRoot.findall('structs/'):
        structName = struct.tag
        if not structName in yamlData:
            continue

        readOptional = """template <>
FOE_GFX_VK_YAML_EXPORT bool yaml_read_optional<{0}>(std::string const &nodeName, YAML::Node const &node, {0} &data) {{
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if(!subNode) {{
        return false;
    }}

    {0} newData{{ }};
    bool read = false;
    try {{""".format(structName)

        readRequired = """template <>
FOE_GFX_VK_YAML_EXPORT void yaml_read_required<{0}>(std::string const &nodeName, YAML::Node const &node, {0} &data) {{
    YAML::Node const &subNode = (nodeName.empty()) ? node : node[nodeName];
    if(!subNode) {{
        throw foeYamlException(nodeName + \" - Required node not found to parse as '{0}'\");
    }}

    {0} newData{{ }};
    bool read = false;
    try {{""".format(structName)

        writeOptional = """template <>
FOE_GFX_VK_YAML_EXPORT bool yaml_write_optional<{0}>(std::string const& nodeName, {0} const& defaultData, {0} const &data, YAML::Node &node) {{
    YAML::Node writeNode;
    bool addedNode = false;

    try {{""".format(structName)

        writeRequired = """template <>
FOE_GFX_VK_YAML_EXPORT void yaml_write_required<{0}>(std::string const& nodeName, {0} const &data, YAML::Node &node) {{
    YAML::Node writeNode;

    try {{""".format(structName)

        excludedMembers = []
        if yamlData[structName] and 'exclude' in yamlData[structName]:
            excludedMembers = yamlData[structName]['exclude']

        for member in struct.findall('members/'):
            memberName = member.tag
            memberType = member.find('type').text
            memberTypeSuffix = member.find('type').get('suffix')

            if memberName in excludedMembers:
                continue
            if memberName == 'pNext':
                continue

            if memberName == 'sType' and memberType == 'VkStructureType':
                text = """
        // sType
        newData.sType = {};
""".format(member.find('value').text)
                readOptional += text
                readRequired += text

            elif memberTypeSuffix and '*' in memberTypeSuffix and member.get('len'):
                # Some variable array type
                formattedName = memberName[1::]
                formattedName = formattedName[0].lower() + formattedName[1::]
                countMember = member.get('len')

                readOptional += """
        // {0} - {1} / {2}
        std::unique_ptr<{0}[]> {1};
        if (auto {3}Node = subNode["{3}"]; {3}Node) {{
            newData.{1} = static_cast<{0} *>(calloc({3}Node.size(), sizeof({0})));
            size_t count = 0;
            for (auto it = {3}Node.begin(); it != {3}Node.end(); ++it) {{
                yaml_read_required("", *it, *const_cast<{0} *>(&newData.{1}[count]));
                ++count;
            }}
            newData.{2} = static_cast<uint32_t>({3}Node.size());
            read = true;
        }}
""".format(memberType, memberName, countMember, formattedName)
                readRequired += """
        // {0} - {1} / {2}
        std::unique_ptr<{0}[]> {1};
        if (auto {3}Node = subNode["{3}"]; {3}Node) {{
            newData.{1} = static_cast<{0} *>(calloc({3}Node.size(), sizeof({0})));
            size_t count = 0;
            for (auto it = {3}Node.begin(); it != {3}Node.end(); ++it) {{
                yaml_read_required("", *it, *const_cast<{0} *>(&newData.{1}[count]));
                ++count;
            }}
            newData.{2} = static_cast<uint32_t>({3}Node.size());
            read = true;
        }} else {{
            throw foeYamlException{{"{3} - Required node not found"}};
        }}
""".format(memberType, memberName, countMember, formattedName)
                writeOptional += """
        // {0} - {1} / {2}
        if(data.{2} > 0) {{
            YAML::Node {3}Node;
            for(uint32_t i = 0; i < data.{2}; ++i) {{
                YAML::Node newNode;
                yaml_write_optional<{0}>("", {0}{{ }}, data.{1}[i], newNode);
                {3}Node.push_back(newNode);
            }}
            writeNode["{3}"] = {3}Node;
            addedNode = true;
        }}
""".format(memberType, memberName, countMember, formattedName)
                writeRequired += """
        // {0} - {1} / {2}
        YAML::Node {3}Node;
        for(uint32_t i = 0; i < data.{2}; ++i) {{
            YAML::Node newNode;
            yaml_write_required<{0}>("", data.{1}[i], newNode);
            {3}Node.push_back(newNode);
        }}
        writeNode["{3}"] = {3}Node;
""".format(memberType, memberName, countMember, formattedName)

            elif xmlRoot.find('enums/{}'.format(memberType)):
                readOptional += """
        // {0} - {1}
        read |= yaml_read_optional_VkEnum("{0}", "{1}", subNode, newData.{1});
""".format(memberType, memberName)
                readRequired += """
        // {0} - {1}
        read |= yaml_read_optional_VkEnum("{0}", "{1}", subNode, newData.{1}); 
""".format(memberType, memberName)
                writeOptional += """
        // {0} - {1}
        addedNode |= yaml_write_optional_VkEnum("{0}", "{1}", defaultData.{1}, data.{1}, writeNode);
""".format(memberType, memberName)
                writeRequired += """
        // {0} - {1}
        yaml_write_required_VkEnum("{0}", "{1}", data.{1}, writeNode);
""".format(memberType, memberName)

            else:
                readOptional += """
        // {0} - {1}
        read |= yaml_read_optional<{0}>("{1}", subNode, newData.{1});
""".format(memberType, memberName)
                readRequired += """
        // {0} - {1}
        read |= yaml_read_optional<{0}>("{1}", subNode, newData.{1});
""".format(memberType, memberName)
                writeOptional += """
        // {0} - {1}
        addedNode |= yaml_write_optional<{0}>("{1}", defaultData.{1}, data.{1}, writeNode);
""".format(memberType, memberName)
                writeRequired += """
        // {0} - {1}
        yaml_write_required<{0}>("{1}", data.{1}, writeNode);
""".format(memberType, memberName)

        # Finish up the function definitions
        readOptional += """
    }} catch (foeYamlException const& e) {{
        cleanup_{}(&newData);
        throw foeYamlException(nodeName + "::" + e.what());
    }}

    if(read)
        data = newData;
    return read;
}}
""".format(structName)

        readRequired += """
    }} catch (foeYamlException const& e) {{
        cleanup_{}(&newData);
        throw foeYamlException(nodeName + "::" + e.what());
    }}

    data = newData;
}}
""".format(structName)

        writeOptional += """
    } catch (foeYamlException const& e) {
        throw foeYamlException(nodeName + \"::\" + e.what());
    }

    if(addedNode) {
        if(nodeName.empty()) {
            node = writeNode;
        } else {
            node[nodeName] = writeNode;
        }
    }

    return addedNode;
}
"""

        writeRequired += """
    } catch (foeYamlException const& e) {
        throw foeYamlException(nodeName + \"::\" + e.what());
    }

    if(nodeName.empty()) {
        node = writeNode;
    } else {
        node[nodeName] = writeNode;
    }
}"""

        print(readRequired)
        print('\n')
        print(readOptional)
        print('\n')
        print(writeRequired)
        print('\n')
        print(writeOptional)
        print('\n')


if __name__ == "__main__":
    main(sys.argv[1:])
