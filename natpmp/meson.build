inc = include_directories('.')

bmdcpp_sources = files ( [
'getgateway.c',
'natpmp.c',
'wingettimeofday.c'
])

bmdcpp_headers = files ( [
'declspec.h',
'getgateway.h',
'natpmp.h',
'wingettimeofday.h',
])

libnatpmp = static_library('natpmp', sources: bmdcpp_sources,include_directories : inc)

libnatpmp_dependency = declare_dependency(
   include_directories : inc,
   link_with : libnatpmp,
   sources : bmdcpp_sources)

