inc = include_directories('.')

bmdcpp_sources = files ( [
'connecthostport.c',
'igd_desc_parse.c',
'minisoap.c',
'minissdpc.c',
'miniupnpc.c',
'miniwget.c',
'minixml.c',
'portlistingparse.c',
'receivedata.c',
'upnpcommands.c',
'upnpdev.c',
'upnperrors.c',
'upnpreplyparse.c',
])

libminiupnpc = static_library('miniupnp', sources: bmdcpp_sources, include_directories : inc,  cpp_args : '-D_GNU_SOURCE', c_args:'-D_GNU_SOURCE')


libminiupnpc_dependency = declare_dependency(
   include_directories : ['.'],
   link_with : libminiupnpc,
   sources : bmdcpp_sources)

