# FoE-Engine<!-- omit in toc -->

[![pipeline status](https://git.stabletec.com/foe/engine/badges/main/pipeline.svg)](https://git.stabletec.com/foe/engine/commits/main)
[![license](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](https://git.stabletec.com/foe/engine/blob/main/LICENSE)

- [External Libraries](#external-libraries)
  - [macOS Environment Setup](#macos-environment-setup)

## External Libraries

- [Assimp](http://assimp.org/) - An open-source mesh loading/saving library for many formats.
- [Bullet Physics](https://pybullet.org) - Physics simulation for games, visual effects, robotics and reinforcement learning.
- [CLI11](https://github.com/CLIUtils/CLI11) - A command line parser for C++11 and beyond that provides a rich feature set with a simple and intuitive interface.
- [Dear ImGui](https://github.com/ocornut/imgui) - Easy to use immediate mode UI.
- [FreeImage](http://freeimage.sourceforge.net/) - Library for loading/saving and working on images.
- [fmt](https://github.com/fmtlib/fmt) - Library for excellent string formatting.
- [GLFW](https://www.glfw.org/) - An Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan development on the desktop. It provides a simple API for creating windows, contexts and surfaces, receiving input and events.
- [glm](https://glm.g-truc.net/) - A header only C++ mathematics library for graphics software based on the OpenGL Shading Language (GLSL) specifications.
- [GPUOpen Vulkan memory Allocator](https://gpuopen.com/gaming-product/vulkan-memory-allocator/) - A Vulkan Memory Allocation helper library. License: MIT.
- [libevent](https://libevent.org/) - The libevent API provides a mechanism to execute a callback function when a specific event occurs on a file descriptor or after a timeout has been reached. Furthermore, libevent also support callbacks due to signals or regular timeouts. 
- [libsodium](https://libsodium.org) - A modern, easy-to-use software library for encryption, decryption, signatures, password hashing, and more.
- [OpenXR](https://www.khronos.org/OpenXR/) - VR/XR API
- [Vulkan](https://www.vulkan.org/) - Graphics API
- [yaml-cpp](https://github.com/jbeder/yaml-cpp) - Library for easy read/write for Yaml data.

### macOS Environment Setup

For the Vulkan/MoltenVK, get the VulkanSDK from LunarG.

For everything else, use Homebrew and get the following:
```sh
brew install assimp bullet cmake catch2 freeimage fmt glfw glm libevent libsodium yaml-cpp 
```