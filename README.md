# wutils

A best-effort C++ library for converting platform-dependent wide strings (`wchar_t`) to fixed-length Unicode types (`char16_t`, `char32_t`).

-----

### What It Is

**wutils** is a C++ library that helps you convert system-defined `wchar_t` and `std::wstring` to Unicode, fixed-length `char16_t`/`char32_t` and `std::u16string`/`std::u32string`. It addresses the issue where low-level system calls or libraries use wide strings but you want to use fixed-length unicode strings.

The library provides a "best-effort" conversion by offering consistent type aliases `uchar_t`, `ustring`, and `ustring_view` for fixed-length Unicode types like `char16_t` (UTF-16) and `char32_t` (UTF-32).

-----

### How It Works

**wutils** inspects the size of `wchar_t` at compile time to determine the correct type mapping.

  * If `sizeof(wchar_t)` is 2 bytes, it assumes a UTF-16 encoding and maps the type aliases to `char16_t`.
  * If `sizeof(wchar_t)` is 4 bytes, it assumes a UTF-32 encoding and maps the type aliases to `char32_t`.

This allows your code to use a consistent `uchar_t`, `ustring`, and `ustring_view` without needing platform-specific conditional compilation.

The library also includes a platform-independent `uswidth` and `wswidth` functions. These calculate the number of columns a character occupies on a display, which is important for handling characters that take up more than one column, such as CJK ideographs.

-----

### Assumptions and Limitations

The C++ standard does not guarantee that `wchar_t` and `std::wstring` are encoded as UTF-16 or UTF-32. **wutils** makes a critical assumption based on the size of the type.

This can lead to incorrect behavior in certain edge cases. For example, some Windows APIs use the legacy UCS-2 encoding for file paths, which is not a complete UTF-16 encoding. In these rare scenarios, **wutils** may produce incorrect conversions or width calculations.

-----

### Getting Started

To use the library, simply add `wutils.hpp` and `wutils.cpp` to your project, and add them to your compilation steps.

For building examples, see the testing `Makefile` (GNU Make) and `Makefile.vc` (MSVC nmake.exe)