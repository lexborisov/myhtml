
inc_libh = include_directories ('.')
inc_libh_dep = declare_dependency (include_directories : inc_libh)

headers = files ([
	'api.h',
	'callback.h',
	'charef.h',
	'charef_resource.h',
	'data_process.h',
	'def.h',
	'myhtml.h',
	'mynamespace.h',
	'mynamespace_resource.h',
	'myosi.h',
	'mystring.h',
	'parser.h',
	'rules.h',
	'serialization.h',
	'stream.h',
	'tag_const.h',
	'tag.h',
	'token.h',
	'tokenizer_doctype.h',
	'tokenizer_end.h',
	'tokenizer.h',
	'tokenizer_script.h',
	'tree.h'
])

install_headers(
	headers,
	subdir: join_paths (VERSIONED_PROJECT_NAME, 'myhtml')
	)

