# ImClass Memory Viewer

<img width="1209" height="499" alt="image" src="https://github.com/user-attachments/assets/bfa9c4f0-def8-4a83-84e4-e76b6bfab6a5" />


Inspired by [ReClass.NET](https://github.com/ReClassNET/ReClass.NET)

Using [Dear ImGui](https://github.com/ocornut/imgui) for immediate-mode rendering

## Features
- Support for x64 & x86
- Signature and string scanning
- IDA style and pattern/mask signature support
- C++ class exporting
- Runtime Type Information (RTTI) parsing
- Pointer previews
- Memory Nodes
  - Pointers to other node types
  - Hex 8 / 16 / 32 / 64
  - Int 8 / 16 / 32 / 64
  - UInt 8 / 16 / 32 / 64
  - Float
  - Double
  - Bool
  - Vector 4 / Vector 3 / Vector 2
  - Matrix 4x4 / Matrix 3x4 / Matrix 3x3
- Parsing for 0x... memory addresses
- Support for address bar math operations
- Module base address searching
- Properly named module function exports

## Tips

Math operations work inside the address bar (ex. "**ntdll.dll + 0x4000**" or "**ntdll.dll + 4100 - 0x50 - 50**" give the same resulting address)

All numbers passed into the address bar are assumed to be hexadecimal (0x1000 == 1000 when searching).

Adding a library name to the address bar will result in its base address being used in math operations involving it.

To view an export, put the library name appended by an exclamation mark and the export name into the address bar (ex. **kernel32.dll!ReadProcessMemory** or **kernel32.dll!WriteProcessMemory**)

Entering an invalid operation into the address bar will just result in a zeroed out output currently.

## Contributing
Feel free to contribute anything you'd like, and it will be accepted it as long as we consider it beneficial to the project.
This includes, but isn't limited to: new features, refactoring existing features and fixing bugs.
