# Find the Oracle Berkeley DB XML library

find_path(DBXML_INCLUDE_DIR names dbxml/DbXml.hpp)

find_library(DBXML_LIBRARY names dbxml)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DBXML DEFAULT_MSG
                                  DBXML_INCLUDE_DIR DBXML_LIBRARY)

mark_as_advanced(DBXML_INCLUDE_DIR DBXML_LIBRARY)
