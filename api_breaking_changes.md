3.0.0 => 4.0.0
===========
* MyHTML split to MyCORE, MyHTML, MyENCODING
* Removed all io print functions to file: ```myhtml_tree_print_by_node```, ```myhtml_tree_print_node_children```, ```myhtml_tree_print_node```; Use serializations instead of their
* If you use encoding enum, like ```MyHTML_ENCODING_UTF8```, now it ```MyENCODING_UTF_8```, i.e ```MyHTML_ENCODING_* => MyENCODING_*```
* Functions migrated to MyCORE from MyHTML: ```myhtml_incoming_buffer_*``` => ```mycore_incoming_buffer_*```, ```myhtml_string*``` => ```mycore_string*```, ```myhtml_utils*``` => ```mycore_utils*```


2.0.1 => 3.0.0
===========
Removed all depending on the Tree. Now you can get a tree of nodes ```myhtml_node_tree```

* From: ```myhtml_tree_node_t * myhtml_node_remove(myhtml_tree_t* tree, myhtml_tree_node_t *node);```
* To: ```myhtml_tree_node_t * myhtml_node_remove(myhtml_tree_node_t *node);```

* From: ```void myhtml_node_delete(myhtml_tree_t* tree, myhtml_tree_node_t *node);```
* To: ```void myhtml_node_delete(myhtml_tree_node_t *node);```

* From: ```void myhtml_node_delete_recursive(myhtml_tree_t* tree, myhtml_tree_node_t *node);```
* To: ```void myhtml_node_delete_recursive(myhtml_tree_node_t *node);```

* From: ```void myhtml_node_free(myhtml_tree_t* tree, myhtml_tree_node_t *node);```
* To: ```void myhtml_node_free(myhtml_tree_node_t *node);```

* From: ```myhtml_tree_node_t * myhtml_node_insert_before(myhtml_tree_t* tree, myhtml_tree_node_t *target, myhtml_tree_node_t *node);```
* To: ```myhtml_tree_node_t * myhtml_node_insert_before(myhtml_tree_node_t *target, myhtml_tree_node_t *node);```

* From: ```myhtml_tree_node_t * myhtml_node_insert_after(myhtml_tree_t* tree, myhtml_tree_node_t *target, myhtml_tree_node_t *node);```
* To: ```myhtml_tree_node_t * myhtml_node_insert_after(myhtml_tree_node_t *target, myhtml_tree_node_t *node);```

* From: ```myhtml_tree_node_t * myhtml_node_append_child(myhtml_tree_t* tree, myhtml_tree_node_t *target, myhtml_tree_node_t *node);```
* To: ```myhtml_tree_node_t * myhtml_node_append_child(myhtml_tree_node_t *target, myhtml_tree_node_t *node);```

* From: ```myhtml_tree_node_t * myhtml_node_insert_to_appropriate_place(myhtml_tree_t* tree, myhtml_tree_node_t *target, myhtml_tree_node_t *node);```
* To: ```myhtml_tree_node_t * myhtml_node_insert_to_appropriate_place(myhtml_tree_node_t *target, myhtml_tree_node_t *node);```

* From: ```myhtml_string_t * myhtml_node_text_set(myhtml_tree_t* tree, myhtml_tree_node_t *node, const char* text, size_t length, myhtml_encoding_t encoding);```
* To: ```myhtml_string_t * myhtml_node_text_set(myhtml_tree_node_t *node, const char* text, size_t length, myhtml_encoding_t encoding);```

* From: ```myhtml_string_t * myhtml_node_text_set_with_charef(myhtml_tree_t* tree, myhtml_tree_node_t *node, const char* text, size_t length, myhtml_encoding_t encoding);```
* To: ```myhtml_string_t * myhtml_node_text_set_with_charef(myhtml_tree_node_t *node, const char* text, size_t length, myhtml_encoding_t encoding);```

* From: ```myhtml_tree_attr_t * myhtml_attribute_add(myhtml_tree_t *tree, myhtml_tree_node_t *node, const char *key, size_t key_len, const char *value, size_t value_len, myhtml_encoding_t encoding);```
* To: ```myhtml_tree_attr_t * myhtml_attribute_add(myhtml_tree_node_t *node, const char *key, size_t key_len, const char *value, size_t value_len, myhtml_encoding_t encoding);```

* From: ```bool myhtml_serialization_tree_buffer(myhtml_tree_t* tree, myhtml_tree_node_t* scope_node, myhtml_string_raw_t* str);```
* To: ```bool myhtml_serialization_tree_buffer(myhtml_tree_node_t* scope_node, myhtml_string_raw_t* str);```

* From: ```bool myhtml_serialization_node_buffer(myhtml_tree_t* tree, myhtml_tree_node_t* node, myhtml_string_raw_t* str);```
* To: ```bool myhtml_serialization_node_buffer(myhtml_tree_node_t* node, myhtml_string_raw_t* str);```

* From: ```bool myhtml_serialization_tree_callback(myhtml_tree_t* tree, myhtml_tree_node_t* scope_node, myhtml_callback_serialize_f callback, void* ptr);```
* To: ```bool myhtml_serialization_tree_callback(myhtml_tree_node_t* scope_node, myhtml_callback_serialize_f callback, void* ptr);```

* From: ```bool myhtml_serialization_node_callback(myhtml_tree_t* tree, myhtml_tree_node_t* node, myhtml_callback_serialize_f callback, void* ptr);```
* To: ```bool myhtml_serialization_node_callback(myhtml_tree_node_t* node, myhtml_callback_serialize_f callback, void* ptr);```

* From: ```myhtml_status_t myhtml_get_nodes_by_attribute_key_recursion(myhtml_tree_t *tree, myhtml_tree_node_t* node, myhtml_collection_t* collection, const char* key, size_t key_len);```
* To: ```myhtml_status_t myhtml_get_nodes_by_attribute_key_recursion(myhtml_tree_node_t* node, myhtml_collection_t* collection, const char* key, size_t key_len);```

* From: ```myhtml_status_t myhtml_get_nodes_by_attribute_value_recursion(myhtml_tree_t *tree, myhtml_tree_node_t* node, myhtml_collection_t* collection, myhtml_attribute_value_find_f func_eq, const char* value, size_t value_len);```
* To: ```myhtml_status_t myhtml_get_nodes_by_attribute_value_recursion(myhtml_tree_node_t* node, myhtml_collection_t* collection, myhtml_attribute_value_find_f func_eq, const char* value, size_t value_len);```

* From: ```myhtml_status_t myhtml_get_nodes_by_attribute_value_recursion_by_key(myhtml_tree_t *tree, myhtml_tree_node_t* node, myhtml_collection_t* collection, myhtml_attribute_value_find_f func_eq, const char* key, size_t key_len, const char* value, size_t value_len);```
* To: ```myhtml_status_t myhtml_get_nodes_by_attribute_value_recursion_by_key(myhtml_tree_node_t* node, myhtml_collection_t* collection, myhtml_attribute_value_find_f func_eq, const char* key, size_t key_len, const char* value, size_t value_len);```

* From: ```void myhtml_tree_node_add_child(myhtml_tree_t* tree, myhtml_tree_node_t* root, myhtml_tree_node_t* node);```
* To: ```void myhtml_tree_node_add_child(myhtml_tree_node_t* root, myhtml_tree_node_t* node);```

* From: ```void myhtml_tree_node_insert_before(myhtml_tree_t* tree, myhtml_tree_node_t* root, myhtml_tree_node_t* node);```
* To: ```void myhtml_tree_node_insert_before(myhtml_tree_node_t* root, myhtml_tree_node_t* node);```

* From: ```void myhtml_tree_node_insert_after(myhtml_tree_t* tree, myhtml_tree_node_t* root, myhtml_tree_node_t* node);```
* To: ```void myhtml_tree_node_insert_after(myhtml_tree_node_t* root, myhtml_tree_node_t* node);```

* From: ```myhtml_tree_node_t * myhtml_tree_node_remove(myhtml_tree_t* tree, myhtml_tree_node_t* node);```
* To: ```myhtml_tree_node_t * myhtml_tree_node_remove(myhtml_tree_node_t* node);```

* From: ```void myhtml_tree_node_free(myhtml_tree_t* tree, myhtml_tree_node_t* node);```
* To: ```void myhtml_tree_node_free(myhtml_tree_node_t* node);```

* From: ```void myhtml_tree_node_delete(myhtml_tree_t* tree, myhtml_tree_node_t* node);```
* To: ```void myhtml_tree_node_delete(myhtml_tree_node_t* node);```

* From: ```void myhtml_tree_node_delete_recursive(myhtml_tree_t* tree, myhtml_tree_node_t* node);```
* To: ```void myhtml_tree_node_delete_recursive(myhtml_tree_node_t* node);```

* From: ```myhtml_tree_node_t * myhtml_tree_node_clone(myhtml_tree_t* tree, myhtml_tree_node_t* node);```
* To: ```myhtml_tree_node_t * myhtml_tree_node_clone(myhtml_tree_node_t* node);```

* From: ```void myhtml_tree_node_insert_by_mode(myhtml_tree_t* tree, myhtml_tree_node_t* adjusted_location, myhtml_tree_node_t* node, enum myhtml_tree_insertion_mode mode);```
* To: ```void myhtml_tree_node_insert_by_mode(myhtml_tree_node_t* adjusted_location, myhtml_tree_node_t* node, enum myhtml_tree_insertion_mode mode);```

* From: ```myhtml_tree_node_t * myhtml_tree_node_insert_by_token(myhtml_tree_t* tree, myhtml_token_node_t* token, enum myhtml_namespace ns);```
* To: ```myhtml_tree_node_t * myhtml_tree_node_insert_by_token(myhtml_token_node_t* token, enum myhtml_namespace ns);```

* From: ```bool myhtml_tree_element_in_scope_by_node(myhtml_tree_t* tree, myhtml_tree_node_t* node, enum myhtml_tag_categories category);```
* To: ```bool myhtml_tree_element_in_scope_by_node(myhtml_tree_node_t* node, enum myhtml_tag_categories category);```

* From: ```myhtml_tree_node_t * myhtml_tree_appropriate_place_inserting_in_tree(myhtml_tree_t* tree, myhtml_tree_node_t* target, enum myhtml_tree_insertion_mode* mode);```
* To: ```myhtml_tree_node_t * myhtml_tree_appropriate_place_inserting_in_tree(myhtml_tree_node_t* target, enum myhtml_tree_insertion_mode* mode);```
