project('Wutils', 'cpp',
  version: '1.0.0',
  meson_version: '>= 1.8.0',
  default_options: ['cpp_std=c++26']
)
inc = include_directories('include')
lib = static_library('wutils', files('src/wutils.cpp'), include_directories: inc)
wutils_dep = declare_dependency(link_with: lib, include_directories: inc)

if not meson.is_cross_build()
  gtest_dep = dependency('gtest', method: 'pkg-config', required: true)
  test_header = executable('test_header', 'tests/test_header.cpp', dependencies: [wutils_dep, gtest_dep])
  test('test-header', test_header)
endif
