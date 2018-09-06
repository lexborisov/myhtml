4.0.5
===========
September 06, 2018

* Fixed parsing problem for PRE element with CDATA in thread and single mode. https://github.com/lexborisov/myhtml/issues/156
* Fixed the problem of parsing chunks when there was a script tag. https://github.com/lexborisov/myhtml/issues/154
* Fixed parsing entity. In very rare cases there were wrong parsing. https://github.com/lexborisov/myhtml/commit/541219bc5241e1b89e7d75c040c94766eff4e95c
* Fixed segfault if doctype hasn't attribute. https://github.com/lexborisov/myhtml/pull/151
* Append link to Perl 5 wrapper module.
* Minor bug fixes

Special thanks to [Kirill Zhumarin](https://github.com/Azq2) for PRs.

4.0.4
===========
January 08, 2018

* Updated ```CMakeLists.txt``` for cmake build. Added support for create Visual Studio Solution and the creation of packages for Linux systems. https://github.com/lexborisov/myhtml/issues/116
* Fixed segfault if we have </form> but not have a opening <form>. https://github.com/lexborisov/myhtml/issues/124
* Fixed cmake install path. https://github.com/lexborisov/myhtml/issues/126
* Fixed rpm changelog date
* Minor bug fixes

4.0.3
===========
January 08, 2018

* Delete

4.0.2
===========
November 07, 2017

* Grammar: change function name *_pasition* => *_position*
* Fixed infinite loop if html file is to big. Queue round not work properly - fixed. https://github.com/lexborisov/myhtml/issues/117
* Append new function ```myhtml_node_is_void_element``` for check to see if we are dealing with a void element. https://github.com/lexborisov/myhtml/pull/119
* Potential loss of the pointer on systems other than x86, x86_64 (Misaligned Integer Pointer)

4.0.1
===========
June 16, 2017

* Fix for creating a spinlock without support siplock https://github.com/lexborisov/myhtml/issues/103
* Added two functions for detect encoding with returning found position ```myencoding_prescan_stream_to_determine_encoding_with_found``` and ```myencoding_extracting_character_encoding_from_charset_with_found``` https://github.com/lexborisov/myhtml/issues/107
* Added automated package build and publicate on ```PackageCloud.io``` (```packagecloud.io/modest/myhtml```)
* Minor bug fixes

Special thanks for [Alexander Fedyashov](https://github.com/layershifter) for help with automated package build.

4.0.0
===========
March 21, 2017

* API breaking changes!!! 
* MyHTML split to MyCORE, MyHTML, MyENCODING. MyCORE is a base module which include shared functions for all others modules.
* Removed all io print functions to file: ```myhtml_tree_print_by_node```, ```myhtml_tree_print_node_children```, ```myhtml_tree_print_node```; Use serializations instead of their
* If you use encoding enum, like ```MyHTML_ENCODING_UTF8```, now it ```MyENCODING_UTF_8```, i.e ```MyHTML_ENCODING_* => MyENCODING_*```
* Functions migrated to MyCORE from MyHTML: ```myhtml_incoming_buffer_*``` => ```mycore_incoming_buffer_*```, ```myhtml_string*``` => ```mycore_string*```, ```myhtml_utils*``` => ```mycore_utils*```
* Fully refactoring build system with GNU Make (Makefile), now it expects generally accepted parameters and rules, like ```install```, ```clean```, ```library``` and [more](https://github.com/lexborisov/myhtml/blob/master/INSTALL.md)
* Tested create a DLL library for Windows OS
* Support [create ports](https://github.com/lexborisov/Modest/wiki/Add-new-port) for different OS or for simple change work with memory, io, threads (if build with threads, default)
* Support [add self modules](https://github.com/lexborisov/Modest/wiki/Add-new-module) for build library
* Now all return statuses, like a ```myhtml_status_t```, ```mycss_status_t``` changed to global `mystatus_t` (unsigned int)
* Added forgot '\0' if text node ends with '\r' [#91](https://github.com/lexborisov/myhtml/issues/91)
* Remove CMakeLists.txt
* Added PKG-CONFIG *.pc after make command


3.0.1
===========
February 17, 2017

* API breaking changes!!! See [api_breaking_changes.md](https://github.com/lexborisov/myhtml/blob/master/api_breaking_changes.md) file
* Sync with Specification (https://html.spec.whatwg.org/multipage/)
* Fix problem with close token position in title tag (the inner essence)
* Fix problem with detect SHIFT_JIS encoding
* Added function ```myhtml_encoding_prescan_stream_to_determine_encoding``` to prescan a byte stream to determine its encoding. In other words, detect encoding in ```meta``` tag before start HTML parsing. See [exapmle](https://github.com/lexborisov/myhtml/blob/master/examples/detect_encoding_in_meta_high_level.c)
* Added function ```myhtml_encoding_name_by_id``` for get encoding name by id
* Added function ```myhtml_encoding_extracting_character_encoding_from_charset```
* Added ```utils/mhash.*``` for create a hash table
* Added function ```myhtml_node_tree``` for get current Tree from a node
* Сonsumes less memory when initializing, 3MB => 1MB with no negative impact on performance. In the future, the memory will be consumed even less.
* Now ```MyHTML_INSTALL_HEADER``` in cmake options set ```ON``` by default
* Fixed broken mapping for convert encoding functions after release 3.0.0

~~3.0.0~~
~~===========~~
~~February 17, 2017~~
~~...~~

2.0.1
===========
January 08, 2017

* Fixed very serious problem with MyHTML::Collection in function ```myhtml_collection_check_size``` https://github.com/lexborisov/myhtml/issues/84

2.0.0
===========
December 22, 2016

* API Breaking Changes: Remove all functions associated with tag index: myhtml_tree_get_tag_index, myhtml_tag_index_*
* Changes for work with threads
* Removed example ```replacing_node_attributes_low_level.c```. Example is not working correctly. Let the future
* Fix for ```myhtml_string_destroy``` function. Sometimes the resources are not free.
* Fix problem with serialization in UTF-8 (0xC2 0xA0)
* Added AVL-Tree for utils

1.0.4
===========
November 15, 2016

* Added possibility to set specify user data value for tree node (https://github.com/lexborisov/myhtml/issues/67)
* Fixed gross errors in serialization (https://github.com/lexborisov/myhtml/commit/b27f9f745841fe11ba7521a3c891c342375320d7)
* Added ```myhtml_incoming_buffer_split``` function for split Incoming Buffer Node
* Added example for replace node attributes before begin token process ```replacing_node_attributes_low_level```
* Changes for function name ```myhtml_incomming_*``` => ```myhtml_incoming_*``` (my bad english)
* Minor bug fixes

1.0.3
===========
September 24, 2016

* Fixed attributes processing bug. Not cleared temp values after processing attribute key. What's in rare cases lead to segfault. (https://github.com/lexborisov/myhtml/commit/873d0fa2cbe8ec4a3b8a50b649506e791f0907c7)
* Fixes for tokenizer state problem with parse without build tree (https://github.com/lexborisov/myhtml/issues/63)
* Fixed problem with append nodes to "tag index" in formatting reconstruction algorithm (https://github.com/lexborisov/myhtml/issues/66#issuecomment-247941324)
* Fixed segfault if build without threads and used parse flag without process token. (https://github.com/lexborisov/myhtml/issues/62)
* Changed rules for parse flag MyHTML_TREE_PARSE_FLAGS_SKIP_WHITESPACE_TOKEN. Now we skip ws tokens, but not for RCDATA, RAWTEXT, CDATA and PLAINTEXT
* Added tree serialization by specification (see example https://github.com/lexborisov/myhtml/blob/master/examples/serialization_high_level.c)
* Tested by 1 billion HTML pages (by commoncrawl.org)

1.0.2
===========
July 14, 2016

* Fixed a bug that in some cases can lead to an infinite loop (https://github.com/lexborisov/myhtml/issues/49)
* Fixed bug for broken tag (like a `<div/===>`); https://github.com/lexborisov/myhtml/issues/50
* Added function myhtml_version for get current version

1.0.1
===========
July 13, 2016

* First Release
* Remove deprecated functions

1.0.0-rc
===========
June 23, 2016

* Synchronized with the specification of the 19.06.2016
* Changed many "strange" code. Improved code stability and readability
* Сonsumes less RAM
* Added interesting examples: 
	+ `tokenizer_colorize_high_level.c` — colorize input html (work with callbacks)
	+ `parse_without_whitespace.c` — parse and build tree without whitespace tokens (work with parse flags)
	+ `nodes_by_attr_key_high_level.c` — get nodes by attribute key
	+ `nodes_by_attr_value_high_level.c` — get nodes by attribute value (by key), interesting example, look to see

* Added API to work with the Incoming Buffer
* Added API for tokens
* Added original positions in html for tokens

* Added parsing flags (their names speak for themselves):
	+ `MyHTML_TREE_PARSE_FLAGS_WITHOUT_BUILD_TREE`
	+ `MyHTML_TREE_PARSE_FLAGS_WITHOUT_PROCESS_TOKEN`
	+ `MyHTML_TREE_PARSE_FLAGS_SKIP_WHITESPACE_TOKEN`
	+ `MyHTML_TREE_PARSE_FLAGS_WITHOUT_DOCTYPE_IN_TREE`

* Added functions to find nodes by attibutes
	+ `myhtml_get_nodes_by_attribute_key` (like a css [foo])
	+ `myhtml_get_nodes_by_attribute_value` (like a css [foo="bar"])
	+ `myhtml_get_nodes_by_attribute_value_whitespace_separated` (like a css [class~="footer"])
	+ `myhtml_get_nodes_by_attribute_value_begin` (like a css [foo^="bar"])
	+ `myhtml_get_nodes_by_attribute_value_end` (like a css [foo$="bar"])
	+ `myhtml_get_nodes_by_attribute_value_contain` (like a css [foo*="bar"])
	+ `myhtml_get_nodes_by_attribute_value_hyphen_separated` (like a css [foo|="bar"])

* Added callbacks for tokens
	+ `myhtml_callback_before_token_done_set`
	+ `myhtml_callback_after_token_done_set`

* Added conditions in source code to build MinGW
* All functions `html_parser*` now return real status
* Redesigned chunks handler and chunk global position, an Incoming Buffer for utf-16
* `myhtml attribute_name` is now deprecated, use `myhtml_attribute_key`
* Tested `myhtml_t` object for thread safe
* Changed `myhtml_string_realloc` args count
* Changed name for function `myhtml_tree_print_node_childs` to `myhtml_tree_print_node_children`
* Changed name for function `myhtml_node_insert_append_child` to `myhtml_node_append_child`
* Changed `tag_ctx_idx` on tokens to `tag_id`
* Changed `tag_idx` on tree nodes to `tag_id`
* Changed `my_str_tm` to `str` on tokens and `my_namespace` to `ns`
* Changed attributes. Now for the key and value using different strings, not united as before.
* Changed input to tokenizer stages
* Fixed handling of strings in the encoding is not UTF-8
* Fixed a hypothetical possibility of going beyond the limits of the buffer in strings
* Reworked parsers tokens. Now they have become clear and obvious
* Fixed significant bug with memory allocated for strings
* Fixed cache for strings. Previously, due to an error it did not work
* Fixed a bug which caused incorrect handle documents in UTF-16
* Deleted function `myhtml_token_is_whithspace`
* Deleted function `myhtml_tree_incomming_buffer_get_last`
* Deleted `myhtml_tree_temp_stream_t` structure and everything connected with it
