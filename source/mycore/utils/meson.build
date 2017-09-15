
mycoreutilssources = files ([
	'avl_tree.c',
	'mchar_async.c',
	'mcobject_async.c',
	'mcobject.c',
	'mcsimple.c',
	'mcsync.c',
	'mctree.c',
	'mhash.c'
])
mycoreutilsheaders = files ([
	'avl_tree.h',
	'mchar_async.h',
	'mcobject_async.h',
	'mcobject.h',
	'mcsimple.h',
	'mcsync.h',
	'mctree.h',
	'mhash.h',
	'resources.h'
])

inc_libmycoreutilsh = include_directories ('.')
inc_libmycoreutilsh_dep = declare_dependency (include_directories : inc_libmycoreutilsh)

# libmycoreutils = static_library('mycoreutils',
# 	mycoreutilssources,
# 	version : PROJECT_VERSION,
# 	pic : true,
# 	dependencies : [inc_headersh_dep],
# 	install : false
# 	)

install_headers(
	mycoreutilsheaders,
	subdir: join_paths (VERSIONED_PROJECT_NAME,'mycore','utils')
	)

