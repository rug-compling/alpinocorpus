util_inc = include_directories('common')

util_common_sources = [
  'common/ProgramOptions.cpp',
  'common/util.cpp'
]

rpath = join_paths(get_option ('prefix'), get_option ('libdir'))

executable('alpinocorpus-create',
  'create/main.cpp',
  util_common_sources,
  include_directories: [inc, util_inc],
  link_with: alpinocorpus,
  dependencies: boost_dep,
  install: true,
  install_rpath: rpath)

executable('alpinocorpus-extract',
  'extract/main.cpp',
  util_common_sources,
  include_directories: [inc, util_inc],
  link_with: alpinocorpus,
  dependencies: boost_dep,
  install: true,
  install_rpath: rpath)

executable('alpinocorpus-get',
  'get/main.cpp',
  util_common_sources,
  include_directories: [inc, util_inc],
  link_with: alpinocorpus,
  dependencies: boost_dep,
  install: true,
  install_rpath: rpath)

executable('alpinocorpus-stats',
  'stats/main.cpp',
  util_common_sources,
  include_directories: [inc, util_inc],
  link_with: alpinocorpus,
  dependencies: boost_dep,
  install: true,
  install_rpath: rpath)

executable('alpinocorpus-xpath',
  'xpath/main.cpp',
  util_common_sources,
  include_directories: [inc, util_inc],
  link_with: alpinocorpus,
  dependencies: boost_dep,
  install: true,
  install_rpath: rpath)

executable('alpinocorpus-xquery',
  'xquery/main.cpp',
  util_common_sources,
  include_directories: [inc, util_inc],
  link_with: alpinocorpus,
  dependencies: boost_dep,
  install: true,
  install_rpath: rpath)

executable('alpinocorpus-xslt',
  'common/Stylesheet.cpp',
  'xslt/main.cpp',
  util_common_sources,
  include_directories: [inc, util_inc],
  link_with: alpinocorpus,
  dependencies: [boost_dep, libexslt_dep, libxml_dep],
  install: true,
  install_rpath: rpath)
