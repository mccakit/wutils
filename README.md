# wutils

A best-effort C++23 library for converting platform-dependent wide strings (`wchar_t`) to fixed-length Unicode types (`char16_t`, `char32_t`), as well as a platform-independent `wswidth` function (tested to be working and reliable on Windows and Linux systems).

-----

### What It Is

**wutils** is a C++ library that helps you convert system-defined `wchar_t` and `std::wstring` to and from Unicode, fixed-length `charX_t` and `std::uXstring`. It addresses the issue where low-level system calls or libraries use wide strings but you want to use fixed-length unicode strings.

The library provides a "best-effort" conversion by offering consistent type aliases `uchar_t`, `ustring`, and `ustring_view` for their fixed-length Unicode `charX_t` equivalents.

Conversion functions are easy to use, simply call `wutils::u8<std::wstring>(w)` to perform a checked conversion from wstring to u8string, you can convert to and from any combination of `std::string`/`std::wstring`/`std::u8string`/`std::u16string`/`std::u32string`.

-----

### How It Works

**wutils** inspects the size of `wchar_t` at compile time to determine the correct type mapping.

This means:

  * If `sizeof(wchar_t)` is 2 bytes, it assumes a UTF-16 encoding and maps the type aliases to `char16_t`.
  * If `sizeof(wchar_t)` is 4 bytes, it assumes a UTF-32 encoding and maps the type aliases to `char32_t`.

This allows your code to use a consistent `uchar_t`, `ustring`, and `ustring_view` without needing platform-specific conditional compilation.

The library also includes a platform-independent `uswidth` and `wswidth` functions. These calculate the number of columns a character occupies on a display, which is important for handling characters that take up more than one column, such as CJK ideographs.

-----

### Assumptions and Limitations

The C++ standard does not guarantee that `wchar_t` and `std::wstring` are encoded as UTF16 or UTF32. **wutils** makes a critical assumption based on the size of the type.

This can lead to incorrect behavior in certain edge cases and target systems. However, in practice, Windows uses UTF16, while Linux, MacOS, and most UNIX-like systems use UTF32.

-----

### Getting Started

To use the library, simply add `wutils.hpp` and `wutils.cpp` to your project, and add them to your compilation steps.

For code examples, see `example.cpp`.

For building examples, see the `CMakeLists.txt` file, or example Makefiles `Makefile.exmample` (GNU Make) and `Makefile.vc.example` (MSVC nmake.exe)
