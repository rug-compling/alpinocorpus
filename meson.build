project('alpinocorpus', 'cpp',
  default_options: ['cpp_std=c++11'],
  version: '3.0.0')

cpp = meson.get_compiler('cpp')

inc = include_directories('include')

boost_dep = dependency('boost', modules: ['filesystem', 'system'])
libexslt_dep = dependency('libexslt')
libxml_dep = dependency('libxml-2.0')
libxslt_dep = dependency('libxslt')
dbxml_dep = cpp.find_library('dbxml', has_headers: 'dbxml/DbXml.hpp')
xercesc_dep = dependency('xerces-c')
xqilla_dep = cpp.find_library('xqilla',
  has_headers: 'xqilla/xqilla-simple.hpp')
zlib_dep = dependency('zlib')

subdir('include')
subdir('man')
subdir('resources')
subdir('src')
subdir('test')
subdir('util')