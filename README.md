# Vulkan SDK Demos

This is a copy of Demos directory of [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) version 1.1.108, Windows version.

Why? In order to track incremental changes and share Vulkan code showing specific behavior.

## Build instructions

Master branch of the repository is updated to make the code buildable as a detached repository:

- Visual Studio solution and projects are upgraded to Visual Studio 2017: v141 toolset and Windows 10 SDK 10.0.16299 (more recent will do fine as well, these are taken for better backward compatibility)
- vcubecpp project is the only one which is updated further
- to make the project buildable, compiler and linker input directories were reset, and Vulkan.props was attached instead to resolve the SDK reference
