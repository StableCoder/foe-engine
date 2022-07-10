#!/usr/bin/env python3

# Copyright (C) 2022 George Cave.
#
# SPDX-License-Identifier: Apache-2.0

import os
import sys
import getopt
import xml.etree.ElementTree as ET
import yaml


def printStruct(xmlRoot, yamlData, structName, count, indentation, outRoot, outSub):
    newEntropyAvailable = False
    excludedMembers = []
    if yamlData[structName] and 'exclude' in yamlData[structName]:
        excludedMembers = yamlData[structName]['exclude']

    for member in xmlRoot.findall('structs/{}/members/'.format(structName)):
        memberName = member.tag
        memberType = member.find('type').text
        memberTypeSuffix = member.find('type').get('suffix')

        if memberName in excludedMembers:
            continue
        if memberName == 'pNext' or memberName == 'sType':
            continue

        enumData = xmlRoot.findall('enums/{}/'.format(memberType))
        subStructData = xmlRoot.findall('structs/{}/'.format(memberType))
        if enumData:
            valCount = len(enumData)
            if valCount > count:
                newEntropyAvailable = True
            if valCount == 0:
                text = '{}{}: 0\n'.format(indentation, memberName)
            else:
                text = '{}{}: {}\n'.format(
                    indentation, memberName, enumData[count % valCount].tag)

            outRoot.write(text)
            outSub.write('  ' + text)

        elif subStructData and memberTypeSuffix and '*' in memberTypeSuffix:
            formattedName = memberName[1::]
            formattedName = formattedName[0].lower() + formattedName[1::]

            # Key
            text = '{}{}:\n'.format(indentation, formattedName)
            outRoot.write(text)
            outSub.write('  ' + text)
            # Array element
            text = '{}  -\n'.format(indentation, formattedName)
            outRoot.write(text)
            outSub.write('  ' + text)
            # Data
            newEntropyAvailable = newEntropyAvailable or printStruct(xmlRoot, yamlData, memberType, count,
                                                                     indentation + '    ', outRoot, outSub)

        elif subStructData:
            # Key
            text = '{}{}:\n'.format(indentation, memberName)
            outRoot.write(text)
            outSub.write('  ' + text)
            # Data
            newEntropyAvailable = newEntropyAvailable or printStruct(xmlRoot, yamlData, memberType, count,
                                                                     indentation + '  ', outRoot, outSub)

        elif 'Vk' in memberType and 'Flags' in memberType:
            # Flag Type
            flagTypeName = memberType.replace('Flags', 'FlagBits')
            typeData = xmlRoot.findall('enums/{}/'.format(flagTypeName))

            valCount = len(typeData)
            if valCount > count:
                newEntropyAvailable = True
            if valCount == 0:
                text = '{}{}: 0\n'.format(indentation, memberName)
            else:
                text = '{}{}: {}\n'.format(
                    indentation, memberName, typeData[count % valCount].tag)

            outRoot.write(text)
            outSub.write('  ' + text)

        elif memberType == 'VkBool32':
            text = '{}{}: {:d}\n'.format(indentation, memberName, count % 2)
            outRoot.write(text)
            outSub.write('  ' + text)

        elif memberType == 'float':
            text = '{}{}: {:.2f}\n'.format(indentation, memberName, count)
            outRoot.write(text)
            outSub.write('  ' + text)

        elif memberType == 'uint32_t':
            text = '{}{}: {:d}\n'.format(indentation, memberName, count)
            outRoot.write(text)
            outSub.write('  ' + text)

        else:
            print('Error: Could not handle type: {} in {}::{}'.format(
                memberType, structName, memberName))
            sys.exit(1)

    return newEntropyAvailable


def main(argv):
    xmlFile = ''
    yamlFile = ''
    outDir = ''

    try:
        opts, args = getopt.getopt(argv, 'x:y:o:', [])
    except getopt.GetoptError:
        print('Error parsing options')
        sys.exit(1)
    for opt, arg in opts:
        if opt == '-x':
            xmlFile = arg
        elif opt == '-y':
            yamlFile = arg
        elif opt == '-o':
            outDir = arg

    if xmlFile == '':
        print("Error: No Vulkan XML file specified")
        sys.exit(1)
    if yamlFile == '':
        print("Error: No input Yaml file specified")
        sys.exit(1)
    if outDir == '':
        print("Error: No output directory specified")
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

    # Main Processing
    for struct in xmlRoot.findall('structs/'):
        structName = struct.tag
        if not structName in yamlData:
            continue

        print('Generating for {}'.format(structName))
        # Make Directory
        structOutDir = '{}/{}/'.format(outDir, structName)
        os.makedirs(structOutDir, exist_ok=True)
        count = 1
        newEntropyAvailable = True
        while count <= 5 and newEntropyAvailable:
            # Open Files
            outRoot = open('{}{}.yaml'.format(structOutDir, count), 'w')
            outSub = open('{}{}-sub.yaml'.format(structOutDir, count), 'w')
            # Start the sub-node file
            outSub.write('subNode:\n')

            newEntropyAvailable = printStruct(xmlRoot, yamlData, structName,
                                              count, '', outRoot, outSub)
            count = count + 1


if __name__ == "__main__":
    main(sys.argv[1:])
