version_array = meson.project_version().split('.')

conf_data = configuration_data()
conf_data.set('version', meson.project_version())
conf_data.set('major_version', version_array[0].to_int())
conf_data.set('minor_version', version_array[1].to_int())
conf_data.set('patch_version', version_array[2].to_int())

configure_file(
  input: 'AlpinoCorpus/config.h.in',
  output: 'config.h',
  configuration: conf_data,
  install: true,
  install_dir: get_option('includedir') / 'AlpinoCorpus'
)

install_headers(
  'AlpinoCorpus/capi.h',
  'AlpinoCorpus/CompactCorpusReader.hh',
  'AlpinoCorpus/CompactCorpusWriter.hh',
  'AlpinoCorpus/CorpusInfo.hh',
  'AlpinoCorpus/CorpusReader.hh',
  'AlpinoCorpus/CorpusReaderFactory.hh',
  'AlpinoCorpus/CorpusWriter.hh',
  'AlpinoCorpus/DLLDefines.hh',
  'AlpinoCorpus/DbCorpusReader.hh',
  'AlpinoCorpus/DbCorpusWriter.hh',
  'AlpinoCorpus/DirectoryCorpusReader.hh',
  'AlpinoCorpus/Entry.hh',
  'AlpinoCorpus/Error.hh',
  'AlpinoCorpus/IterImpl.hh',
  'AlpinoCorpus/LexItem.hh',
  'AlpinoCorpus/MultiCorpusReader.hh',
  'AlpinoCorpus/RecursiveCorpusReader.hh',
  'AlpinoCorpus/macros.hh',
  subdir: 'AlpinoCorpus')

install_headers(
  'AlpinoCorpus/util/Either.hh',
  'AlpinoCorpus/util/NonCopyable.hh',
  subdir: 'AlpinoCorpus/util')