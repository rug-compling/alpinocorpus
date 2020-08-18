project('alpinocorpus', 'cpp',
  default_options: ['cpp_std=c++11'],
  version: '3.0.0')

cpp = meson.get_compiler('cpp')

inc = include_directories('include')

boost_dep = dependency('boost', modules: ['filesystem', 'system'])
libexslt_dep = dependency('libexslt')
libxml_dep = dependency('libxml-2.0')
libxslt_dep = dependency('libxslt')
zlib_dep = dependency('zlib')

dbxml_bundle = get_option('dbxml_bundle')
if dbxml_bundle == 'system'
  xercesc_dep = dependency('xerces-c')

  xqilla_dep = cpp.find_library('xqilla',
    has_headers: 'xqilla/xqilla-simple.hpp')

  dbxml_dep = cpp.find_library('dbxml',
    has_headers: 'dbxml/DbXml.hpp')

  dbxml_rpath = ''
else
  bundle_inc = include_directories(dbxml_bundle / 'include')

  xercesc_lib = cpp.find_library('xerces-c', dirs: dbxml_bundle / 'lib')
  xercesc_dep = declare_dependency(include_directories: bundle_inc,
    dependencies: xercesc_lib)

  xqilla_lib = cpp.find_library('xqilla', dirs: dbxml_bundle / 'lib')
  xqilla_dep = declare_dependency(include_directories: bundle_inc,
    dependencies: xqilla_lib)

  dbxml_lib = cpp.find_library('dbxml', dirs: dbxml_bundle / 'lib')
  dbxml_dep = declare_dependency(include_directories: bundle_inc,
    dependencies: dbxml_lib)

  dbxml_rpath = dbxml_bundle / 'lib'
endif

subdir('include')
subdir('man')
subdir('resources')
subdir('src')
subdir('test')
subdir('util')