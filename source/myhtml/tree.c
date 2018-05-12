/*
 Copyright (C) 2015-2017 Alexander Borisov
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 
 Author: lex.borisov@gmail.com (Alexander Borisov)
*/

#include "myhtml/tree.h"

myhtml_tree_t * myhtml_tree_create(void)
{
    return (myhtml_tree_t*)mycore_calloc(1, sizeof(myhtml_tree_t));
}

mystatus_t myhtml_tree_init(myhtml_tree_t* tree, myhtml_t* myhtml)
{
    mystatus_t status = MyHTML_STATUS_OK;
    
    tree->myhtml             = myhtml;
    tree->token              = myhtml_token_create(tree, 512);
    
    if(tree->token == NULL)
      return MyHTML_STATUS_TOKENIZER_ERROR_MEMORY_ALLOCATION;
    
    tree->temp_tag_name.data = NULL;
    tree->stream_buffer      = NULL;
    tree->parse_flags        = MyHTML_TREE_PARSE_FLAGS_CLEAN;
    tree->context            = NULL;
    
    tree->callback_before_token     = NULL;
    tree->callback_after_token      = NULL;
    tree->callback_before_token_ctx = NULL;
    tree->callback_after_token_ctx  = NULL;
    
    tree->callback_tree_node_insert     = NULL;
    tree->callback_tree_node_remove     = NULL;
    tree->callback_tree_node_insert_ctx = NULL;
    tree->callback_tree_node_remove_ctx = NULL;
    
    if(status)
        return status;
    
    /* Thread Queue */
    tree->queue = mythread_queue_create();
    if(tree->queue == NULL)
        return MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
    
    status = mythread_queue_init(tree->queue, 9182);
    if(status)
        return status;
    
    /* Init Incoming Buffer objects */
    tree->mcobject_incoming_buf = mcobject_create();
    if(tree->mcobject_incoming_buf == NULL)
        return MyHTML_STATUS_TREE_ERROR_INCOMING_BUFFER_CREATE;
    
    status = mcobject_init(tree->mcobject_incoming_buf, 256, sizeof(mycore_incoming_buffer_t));
    if(status)
        return status;
    
    /* init Tree Node objects */
    tree->tree_obj = mcobject_async_create();
    if(tree->tree_obj == NULL)
        return MyHTML_STATUS_TREE_ERROR_MCOBJECT_CREATE;
    
    mcobject_async_status_t mcstatus = mcobject_async_init(tree->tree_obj, 128, 1024, sizeof(myhtml_tree_node_t));
    if(mcstatus)
        return MyHTML_STATUS_TREE_ERROR_MCOBJECT_INIT;
    
    tree->mchar              = mchar_async_create();
    tree->active_formatting  = myhtml_tree_active_formatting_init(tree);
    tree->open_elements      = myhtml_tree_open_elements_init(tree);
    tree->other_elements     = myhtml_tree_list_init();
    tree->token_list         = myhtml_tree_token_list_init();
    tree->template_insertion = myhtml_tree_template_insertion_init(tree);
    
    if(tree->mchar == NULL)
        return MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
    
    if((status = mchar_async_init(tree->mchar, 128, (4096 * 5))))
        return status;
    
    tree->mcasync_tree_id = mcobject_async_node_add(tree->tree_obj, &mcstatus);
    if(mcstatus)
        return MyHTML_STATUS_TREE_ERROR_MCOBJECT_CREATE_NODE;
    
    tree->mcasync_rules_token_id = mcobject_async_node_add(tree->token->nodes_obj, &mcstatus);
    if(mcstatus)
        return MyHTML_STATUS_TREE_ERROR_MCOBJECT_CREATE_NODE;
    
    tree->mcasync_rules_attr_id = mcobject_async_node_add(tree->token->attr_obj, &mcstatus);
    if(mcstatus)
        return MyHTML_STATUS_TREE_ERROR_MCOBJECT_CREATE_NODE;
    
#ifndef MyCORE_BUILD_WITHOUT_THREADS
    tree->async_args = (myhtml_async_args_t*)mycore_calloc(myhtml->thread_total, sizeof(myhtml_async_args_t));
    if(tree->async_args == NULL)
        return MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
    
    // for batch thread
    for(size_t i = 0; i < myhtml->thread_total; i++) {
        tree->async_args[i].mchar_node_id = mchar_async_node_add(tree->mchar, &status);
        
        if(status)
            return status;
    }
#else /* MyCORE_BUILD_WITHOUT_THREADS */
    tree->async_args = (myhtml_async_args_t*)mycore_calloc(1, sizeof(myhtml_async_args_t));
    
    if(tree->async_args == NULL)
        return MyHTML_STATUS_TREE_ERROR_MEMORY_ALLOCATION;
    
    tree->async_args->mchar_node_id = mchar_async_node_add(tree->mchar, &status);
    
    if(status)
        return status;
    
#endif /* MyCORE_BUILD_WITHOUT_THREADS */
    
    /* for main thread only after parsing */
    tree->mchar_node_id = tree->async_args->mchar_node_id;
    
    tree->sync = mcsync_create();
    if(tree->sync == NULL)
        return MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
    
    if(mcsync_init(tree->sync))
        return MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
    
    /* init Tags after create and init mchar */
    tree->tags = myhtml_tag_create();
    status = myhtml_tag_init(tree, tree->tags);
    
    myhtml_tree_clean(tree);
    
    return status;
}

void myhtml_tree_clean(myhtml_tree_t* tree)
{
#ifndef MyCORE_BUILD_WITHOUT_THREADS
    myhtml_t* myhtml = tree->myhtml;
    
    for(size_t i = 0; i < myhtml->thread_total; i++) {
        mchar_async_node_clean(tree->mchar, tree->async_args[i].mchar_node_id);
    }
#else
    mchar_async_node_clean(tree->mchar, tree->mchar_node_id);
#endif
    
    mcobject_async_node_clean(tree->tree_obj, tree->mcasync_tree_id);
    mcobject_async_node_clean(tree->token->nodes_obj, tree->mcasync_rules_token_id);
    mcobject_async_node_clean(tree->token->attr_obj, tree->mcasync_rules_attr_id);
    
#ifndef MyCORE_BUILD_WITHOUT_THREADS
    mythread_queue_list_entry_clean(tree->queue_entry);
    mythread_queue_list_entry_make_batch(tree->myhtml->thread_batch, tree->queue_entry);
    mythread_queue_list_entry_make_stream(tree->myhtml->thread_stream, tree->queue_entry);
#endif /* MyCORE_BUILD_WITHOUT_THREADS */
    
    myhtml_token_clean(tree->token);
    
    // null root
    myhtml_tree_node_create(tree);
    
    tree->document  = myhtml_tree_node_create(tree);
    tree->fragment  = NULL;
    
    tree->doctype.is_html = false;
    tree->doctype.attr_name    = NULL;
    tree->doctype.attr_public  = NULL;
    tree->doctype.attr_system  = NULL;
    
    tree->node_html = 0;
    tree->node_body = 0;
    tree->node_head = 0;
    tree->node_form = 0;
    
    tree->state               = MyHTML_TOKENIZER_STATE_DATA;
    tree->state_of_builder    = MyHTML_TOKENIZER_STATE_DATA;
    tree->insert_mode         = MyHTML_INSERTION_MODE_INITIAL;
    tree->orig_insert_mode    = MyHTML_INSERTION_MODE_INITIAL;
    tree->compat_mode         = MyHTML_TREE_COMPAT_MODE_NO_QUIRKS;
    tree->tmp_tag_id          = MyHTML_TAG__UNDEF;
    tree->flags               = MyHTML_TREE_FLAGS_CLEAN|MyHTML_TREE_FLAGS_FRAMESET_OK;
    tree->foster_parenting    = false;
    tree->token_namespace     = NULL;
    tree->incoming_buf        = NULL;
    tree->incoming_buf_first  = NULL;
    tree->global_offset       = 0;
    tree->current_qnode       = NULL;
    tree->token_last_done     = NULL;
    tree->tokenizer_status    = MyHTML_STATUS_OK;
    
    tree->encoding            = MyENCODING_UTF_8;
    tree->encoding_usereq     = MyENCODING_DEFAULT;
    
    myhtml_stream_buffer_clean(tree->stream_buffer);
    
    myhtml_tree_active_formatting_clean(tree);
    myhtml_tree_open_elements_clean(tree);
    myhtml_tree_list_clean(tree->other_elements);
    myhtml_tree_token_list_clean(tree->token_list);
    myhtml_tree_template_insertion_clean(tree);
    mcobject_clean(tree->mcobject_incoming_buf);
    myhtml_tag_clean(tree->tags);
    mythread_queue_clean(tree->queue);
    
    tree->attr_current = myhtml_token_attr_create(tree->token, tree->token->mcasync_attr_id);
}

void myhtml_tree_clean_all(myhtml_tree_t* tree)
{
    mcobject_async_clean(tree->tree_obj);
    myhtml_token_clean(tree->token);
    mchar_async_clean(tree->mchar);
    
    // null root
    myhtml_tree_node_create(tree);
    
    tree->document  = myhtml_tree_node_create(tree);
    tree->fragment  = NULL;
    
    tree->doctype.is_html = false;
    tree->doctype.attr_name    = NULL;
    tree->doctype.attr_public  = NULL;
    tree->doctype.attr_system  = NULL;
    
    tree->node_html = 0;
    tree->node_body = 0;
    tree->node_head = 0;
    tree->node_form = 0;
    
    tree->state               = MyHTML_TOKENIZER_STATE_DATA;
    tree->state_of_builder    = MyHTML_TOKENIZER_STATE_DATA;
    tree->insert_mode         = MyHTML_INSERTION_MODE_INITIAL;
    tree->orig_insert_mode    = MyHTML_INSERTION_MODE_INITIAL;
    tree->compat_mode         = MyHTML_TREE_COMPAT_MODE_NO_QUIRKS;
    tree->tmp_tag_id          = MyHTML_TAG__UNDEF;
    tree->flags               = MyHTML_TREE_FLAGS_CLEAN|MyHTML_TREE_FLAGS_FRAMESET_OK;
    tree->foster_parenting    = false;
    tree->token_namespace     = NULL;
    tree->incoming_buf        = NULL;
    tree->incoming_buf_first  = NULL;
    tree->global_offset       = 0;
    tree->current_qnode       = NULL;
    tree->token_last_done     = NULL;
    tree->tokenizer_status    = MyHTML_STATUS_OK;
    
    tree->encoding            = MyENCODING_UTF_8;
    tree->encoding_usereq     = MyENCODING_DEFAULT;
    
    myhtml_stream_buffer_clean(tree->stream_buffer);
    
    myhtml_tree_active_formatting_clean(tree);
    myhtml_tree_open_elements_clean(tree);
    myhtml_tree_list_clean(tree->other_elements);
    myhtml_tree_token_list_clean(tree->token_list);
    myhtml_tree_template_insertion_clean(tree);
    mcobject_clean(tree->mcobject_incoming_buf);
    myhtml_tag_clean(tree->tags);
    
#ifndef MyCORE_BUILD_WITHOUT_THREADS
    mythread_queue_list_entry_clean(tree->queue_entry);
    mythread_queue_list_entry_make_batch(tree->myhtml->thread_batch, tree->queue_entry);
    mythread_queue_list_entry_make_stream(tree->myhtml->thread_stream, tree->queue_entry);
#endif
    
    tree->attr_current = myhtml_token_attr_create(tree->token, tree->token->mcasync_attr_id);
}

myhtml_tree_t * myhtml_tree_destroy(myhtml_tree_t* tree)
{
    if(tree == NULL)
        return NULL;
    
    /* destroy tags before other objects */
    tree->tags                  = myhtml_tag_destroy(tree->tags);
    tree->active_formatting     = myhtml_tree_active_formatting_destroy(tree);
    tree->open_elements         = myhtml_tree_open_elements_destroy(tree);
    tree->other_elements        = myhtml_tree_list_destroy(tree->other_elements, true);
    tree->token_list            = myhtml_tree_token_list_destroy(tree->token_list, true);
    tree->template_insertion    = myhtml_tree_template_insertion_destroy(tree);
    tree->sync                  = mcsync_destroy(tree->sync, true);
    tree->tree_obj              = mcobject_async_destroy(tree->tree_obj, true);
    tree->token                 = myhtml_token_destroy(tree->token);
    tree->mchar                 = mchar_async_destroy(tree->mchar, 1);
    tree->stream_buffer         = myhtml_stream_buffer_destroy(tree->stream_buffer, true);
    tree->queue                 = mythread_queue_destroy(tree->queue);
    tree->mcobject_incoming_buf = mcobject_destroy(tree->mcobject_incoming_buf, true);
    
    myhtml_tree_temp_tag_name_destroy(&tree->temp_tag_name, false);
    
    mycore_free(tree->async_args);
    mycore_free(tree);
    
    return NULL;
}

void myhtml_tree_node_clean(myhtml_tree_node_t* tree_node)
{
    memset(tree_node, 0, sizeof(myhtml_tree_node_t));
    tree_node->ns = MyHTML_NAMESPACE_HTML;
}

/* parse flags */
myhtml_tree_parse_flags_t myhtml_tree_parse_flags(myhtml_tree_t* tree)
{
    return tree->parse_flags;
}

void myhtml_tree_parse_flags_set(myhtml_tree_t* tree, myhtml_tree_parse_flags_t flags)
{
    tree->parse_flags = flags;
}

myhtml_t * myhtml_tree_get_myhtml(myhtml_tree_t* tree)
{
    if(tree)
        return tree->myhtml;
    
    return NULL;
}

myhtml_tag_t * myhtml_tree_get_tag(myhtml_tree_t* tree)
{
    if(tree)
        return tree->tags;
    
    return NULL;
}

myhtml_tree_node_t * myhtml_tree_get_document(myhtml_tree_t* tree)
{
    return tree->document;
}

myhtml_tree_node_t * myhtml_tree_get_node_html(myhtml_tree_t* tree)
{
    return tree->node_html;
}

myhtml_tree_node_t * myhtml_tree_get_node_body(myhtml_tree_t* tree)
{
    return tree->node_body;
}

myhtml_tree_node_t * myhtml_tree_get_node_head(myhtml_tree_t* tree)
{
    return tree->node_head;
}

mchar_async_t * myhtml_tree_get_mchar(myhtml_tree_t* tree)
{
    return tree->mchar;
}

size_t myhtml_tree_get_mchar_node_id(myhtml_tree_t* tree)
{
    return tree->mchar_node_id;
}

myhtml_tree_node_t * myhtml_tree_node_create(myhtml_tree_t* tree)
{
    myhtml_tree_node_t* node = (myhtml_tree_node_t*)mcobject_async_malloc(tree->tree_obj, tree->mcasync_tree_id, NULL);
    myhtml_tree_node_clean(node);
    
    node->tree = tree;
    
    return node;
}

void myhtml_tree_node_add_child(myhtml_tree_node_t* root, myhtml_tree_node_t* node)
{
    if(root->last_child) {
        root->last_child->next = node;
        node->prev = root->last_child;
    }
    else {
        root->child = node;
    }
    
    node->parent     = root;
    root->last_child = node;
    
    myhtml_tree_node_callback_insert(node->tree, node);
}

void myhtml_tree_node_insert_before(myhtml_tree_node_t* root, myhtml_tree_node_t* node)
{
    if(root->prev) {
        root->prev->next = node;
        node->prev = root->prev;
    }
    else {
        root->parent->child = node;
    }
    
    node->parent = root->parent;
    node->next   = root;
    root->prev   = node;
    
    myhtml_tree_node_callback_insert(node->tree, node);
}

void myhtml_tree_node_insert_after(myhtml_tree_node_t* root, myhtml_tree_node_t* node)
{
    if(root->next) {
        root->next->prev = node;
        node->next = root->next;
    }
    else {
        root->parent->last_child = node;
    }
    
    node->parent = root->parent;
    node->prev   = root;
    root->next   = node;
    
    myhtml_tree_node_callback_insert(node->tree, node);
}

myhtml_tree_node_t * myhtml_tree_node_find_parent_by_tag_id(myhtml_tree_node_t* node, myhtml_tag_id_t tag_id)
{
    node = node->parent;
    
    while (node && node->tag_id != tag_id) {
        node = node->parent;
    }
    
    return node;
}

myhtml_tree_node_t * myhtml_tree_node_remove(myhtml_tree_node_t* node)
{
    if(node->next)
        node->next->prev = node->prev;
    else if(node->parent)
        node->parent->last_child = node->prev;
    
    if(node->prev) {
        node->prev->next = node->next;
        node->prev = NULL;
    } else if(node->parent)
        node->parent->child = node->next;
    
    node->parent = NULL;
    
    if(node->next)
        node->next = NULL;
    
    myhtml_tree_node_callback_remove(node->tree, node);
    
    return node;
}

void myhtml_tree_node_free(myhtml_tree_node_t* node)
{
    if(node == NULL)
        return;
    
    if(node->token) {
        myhtml_token_attr_delete_all(node->tree->token, node->token);
        myhtml_token_delete(node->tree->token, node->token);
    }
    
    mcobject_async_free(node->tree->tree_obj, node);
}

void myhtml_tree_node_delete(myhtml_tree_node_t* node)
{
    if(node == NULL)
        return;
    
    myhtml_tree_node_remove(node);
    myhtml_tree_node_free(node);
}

static void _myhtml_tree_node_delete_recursive(myhtml_tree_node_t* node)
{
    while(node)
    {
        if(node->child)
            _myhtml_tree_node_delete_recursive(node->child);
        
        node = node->next;
        myhtml_tree_node_delete(node);
    }
}

void myhtml_tree_node_delete_recursive(myhtml_tree_node_t* node)
{
    if(node == NULL)
        return;
    
    if(node->child)
        _myhtml_tree_node_delete_recursive(node->child);
    
    myhtml_tree_node_delete(node);
}

myhtml_tree_node_t * myhtml_tree_node_clone(myhtml_tree_node_t* node)
{
    myhtml_tree_node_t* new_node = myhtml_tree_node_create(node->tree);
    
    if(node->token)
        myhtml_token_node_wait_for_done(node->tree->token, node->token);
    
    new_node->token        = myhtml_token_node_clone(node->tree->token, node->token,
                                                     node->tree->mcasync_rules_token_id,
                                                     node->tree->mcasync_rules_attr_id);
    new_node->tag_id       = node->tag_id;
    new_node->ns           = node->ns;
    
    if(new_node->token)
        new_node->token->type |= MyHTML_TOKEN_TYPE_DONE;
    
    return new_node;
}

void myhtml_tree_node_insert_by_mode(myhtml_tree_node_t* adjusted_location,
                                     myhtml_tree_node_t* node, enum myhtml_tree_insertion_mode mode)
{
    if(mode == MyHTML_TREE_INSERTION_MODE_DEFAULT)
        myhtml_tree_node_add_child(adjusted_location, node);
    else if(mode == MyHTML_TREE_INSERTION_MODE_BEFORE)
        myhtml_tree_node_insert_before(adjusted_location, node);
    else
        myhtml_tree_node_insert_after(adjusted_location, node);
}

myhtml_tree_node_t * myhtml_tree_node_insert_by_token(myhtml_tree_t* tree, myhtml_token_node_t* token, myhtml_namespace_t ns)
{
    myhtml_tree_node_t* node = myhtml_tree_node_create(tree);
    
    node->tag_id = token->tag_id;
    node->token  = token;
    node->ns     = ns;
    
    enum myhtml_tree_insertion_mode mode;
    myhtml_tree_node_t* adjusted_location = myhtml_tree_appropriate_place_inserting(tree, NULL, &mode);
    myhtml_tree_node_insert_by_mode(adjusted_location, node, mode);
    
    myhtml_tree_open_elements_append(tree, node);
    return node;
}

myhtml_tree_node_t * myhtml_tree_node_insert(myhtml_tree_t* tree, myhtml_tag_id_t tag_idx, myhtml_namespace_t ns)
{
    myhtml_tree_node_t* node = myhtml_tree_node_create(tree);
    
    node->token  = NULL;
    node->tag_id = tag_idx;
    node->ns     = ns;
    
    enum myhtml_tree_insertion_mode mode;
    myhtml_tree_node_t* adjusted_location = myhtml_tree_appropriate_place_inserting(tree, NULL, &mode);
    myhtml_tree_node_insert_by_mode(adjusted_location, node, mode);
    
    myhtml_tree_open_elements_append(tree, node);
    return node;
}

myhtml_tree_node_t * myhtml_tree_node_insert_comment(myhtml_tree_t* tree, myhtml_token_node_t* token, myhtml_tree_node_t* parent)
{
    myhtml_tree_node_t* node = myhtml_tree_node_create(tree);
    
    node->token  = token;
    node->tag_id = MyHTML_TAG__COMMENT;
    
    enum myhtml_tree_insertion_mode mode = 0;
    if(parent == NULL) {
        parent = myhtml_tree_appropriate_place_inserting(tree, NULL, &mode);
    }
    
    myhtml_tree_node_insert_by_mode(parent, node, mode);
    node->ns = parent->ns;
    
    return node;
}

myhtml_tree_node_t * myhtml_tree_node_insert_doctype(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    myhtml_tree_node_t* node = myhtml_tree_node_create(tree);
    
    node->token        = token;
    node->ns           = MyHTML_NAMESPACE_HTML;
    node->tag_id       = MyHTML_TAG__DOCTYPE;
    
    myhtml_tree_node_add_child(tree->document, node);
    return node;
}

myhtml_tree_node_t * myhtml_tree_node_insert_root(myhtml_tree_t* tree, myhtml_token_node_t* token, enum myhtml_namespace ns)
{
    myhtml_tree_node_t* node = myhtml_tree_node_create(tree);
    
    if(token)
        node->tag_id = token->tag_id;
    else
        node->tag_id = MyHTML_TAG_HTML;
    
    node->token = token;
    node->ns    = ns;
    
    myhtml_tree_node_add_child(tree->document, node);
    myhtml_tree_open_elements_append(tree, node);
    
    tree->node_html = node;
    return node;
}

myhtml_tree_node_t * myhtml_tree_node_insert_text(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    enum myhtml_tree_insertion_mode mode;
    myhtml_tree_node_t* adjusted_location = myhtml_tree_appropriate_place_inserting(tree, NULL, &mode);
    
    if(adjusted_location == tree->document)
        return NULL;
    
    if(mode == MyHTML_TREE_INSERTION_MODE_AFTER) {
        if(adjusted_location->tag_id == MyHTML_TAG__TEXT && adjusted_location->token)
        {
            myhtml_token_merged_two_token_string(tree, adjusted_location->token, token, false);
            return adjusted_location;
        }
    }
    else if(mode == MyHTML_TREE_INSERTION_MODE_BEFORE) {
        if(adjusted_location->tag_id == MyHTML_TAG__TEXT && adjusted_location->token) {
            myhtml_token_merged_two_token_string(tree, token, adjusted_location->token, true);
            return adjusted_location;
        }
    }
    else {
        if(adjusted_location->last_child && adjusted_location->last_child->tag_id == MyHTML_TAG__TEXT &&
           adjusted_location->last_child->token)
        {
            myhtml_token_merged_two_token_string(tree, adjusted_location->last_child->token, token, false);
            return adjusted_location->last_child;
        }
    }
    
    myhtml_tree_node_t* node = myhtml_tree_node_create(tree);
    
    node->tag_id = MyHTML_TAG__TEXT;
    node->token  = token;
    node->ns     = adjusted_location->ns;
    
    myhtml_tree_node_insert_by_mode(adjusted_location, node, mode);
    return node;
}

myhtml_tree_node_t * myhtml_tree_node_insert_by_node(myhtml_tree_t* tree, myhtml_tree_node_t* node)
{
    enum myhtml_tree_insertion_mode mode;
    myhtml_tree_node_t* adjusted_location = myhtml_tree_appropriate_place_inserting(tree, NULL, &mode);
    myhtml_tree_node_insert_by_mode(adjusted_location, node, mode);
    
    myhtml_tree_open_elements_append(tree, node);
    return node;
}

myhtml_tree_node_t * myhtml_tree_node_insert_foreign_element(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    enum myhtml_tree_insertion_mode mode;
    myhtml_tree_node_t* adjusted_location = myhtml_tree_appropriate_place_inserting(tree, NULL, &mode);
    
    myhtml_tree_node_t* node = myhtml_tree_node_create(tree);
    
    node->tag_id = token->tag_id;
    node->token  = token;
    node->ns     = adjusted_location->ns;
    
    myhtml_tree_node_insert_by_mode(adjusted_location, node, mode);
    myhtml_tree_open_elements_append(tree, node);
    return node;
}

myhtml_tree_node_t * myhtml_tree_node_insert_html_element(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    enum myhtml_tree_insertion_mode mode;
    myhtml_tree_node_t* adjusted_location = myhtml_tree_appropriate_place_inserting(tree, NULL, &mode);
    
    myhtml_tree_node_t* node = myhtml_tree_node_create(tree);
    
    node->tag_id = token->tag_id;
    node->token  = token;
    node->ns     = MyHTML_NAMESPACE_HTML;
    
    myhtml_tree_node_insert_by_mode(adjusted_location, node, mode);
    myhtml_tree_open_elements_append(tree, node);
    return node;
}

myhtml_tree_node_t * myhtml_tree_element_in_scope(myhtml_tree_t* tree, myhtml_tag_id_t tag_idx,
                                                  myhtml_namespace_t mynamespace, enum myhtml_tag_categories category)
{
    myhtml_tree_node_t** list = tree->open_elements->list;
    
    const myhtml_tag_context_t *tag_ctx;
    size_t i = tree->open_elements->length;
    
    while(i) {
        i--;
        
        tag_ctx = myhtml_tag_get_by_id(tree->tags, list[i]->tag_id);
        
        if(list[i]->tag_id == tag_idx &&
           (mynamespace == MyHTML_NAMESPACE_UNDEF || list[i]->ns == mynamespace))
            return list[i];
        else if(category == MyHTML_TAG_CATEGORIES_SCOPE_SELECT) {
            if((tag_ctx->cats[list[i]->ns] & category) == 0)
                break;
        }
        else if(tag_ctx->cats[list[i]->ns] & category)
            break;
    }
    
    return NULL;
}

bool myhtml_tree_element_in_scope_by_node(myhtml_tree_node_t* node, enum myhtml_tag_categories category)
{
    myhtml_tree_t* tree = node->tree;
    myhtml_tree_node_t** list = tree->open_elements->list;
    
    const myhtml_tag_context_t *tag_ctx;
    size_t i = tree->open_elements->length;
    
    while(i) {
        i--;
        
        tag_ctx = myhtml_tag_get_by_id(tree->tags, list[i]->tag_id);
        
        if(list[i] == node)
            return true;
        else if(category == MyHTML_TAG_CATEGORIES_SCOPE_SELECT) {
            if((tag_ctx->cats[list[i]->ns] & category) == 0)
                break;
        }
        else if(tag_ctx->cats[list[i]->ns] & category)
            break;
    }
    
    return false;
}

// list
myhtml_tree_list_t * myhtml_tree_list_init(void)
{
    myhtml_tree_list_t* list = mycore_malloc(sizeof(myhtml_tree_list_t));
    
    list->length = 0;
    list->size = 4096;
    list->list = (myhtml_tree_node_t**)mycore_malloc(sizeof(myhtml_tree_node_t*) * list->size);
    
    return list;
}

void myhtml_tree_list_clean(myhtml_tree_list_t* list)
{
    list->length = 0;
}

myhtml_tree_list_t * myhtml_tree_list_destroy(myhtml_tree_list_t* list, bool destroy_self)
{
    if(list == NULL)
        return NULL;
    
    if(list->list)
        mycore_free(list->list);
    
    if(destroy_self && list) {
        mycore_free(list);
        return NULL;
    }
    
    return list;
}

void myhtml_tree_list_append(myhtml_tree_list_t* list, myhtml_tree_node_t* node)
{
    if(list->length >= list->size) {
        list->size <<= 1;
        
        myhtml_tree_node_t** tmp = (myhtml_tree_node_t**)mycore_realloc(list->list, sizeof(myhtml_tree_node_t*) * list->size);
        
        if(tmp)
            list->list = tmp;
    }
    
    list->list[list->length] = node;
    list->length++;
}

void myhtml_tree_list_append_after_index(myhtml_tree_list_t* list, myhtml_tree_node_t* node, size_t index)
{
    myhtml_tree_list_insert_by_index(list, node, (index + 1));
}

void myhtml_tree_list_insert_by_index(myhtml_tree_list_t* list, myhtml_tree_node_t* node, size_t index)
{
    if(list->length >= list->size) {
        list->size <<= 1;
        
        myhtml_tree_node_t** tmp = (myhtml_tree_node_t**)mycore_realloc(list->list, sizeof(myhtml_tree_node_t*) * list->size);
        
        if(tmp)
            list->list = tmp;
    }
    
    myhtml_tree_node_t** node_list = list->list;
    
    memmove(&node_list[(index + 1)], &node_list[index], sizeof(myhtml_tree_node_t*) * (list->length - index));
    
    list->list[index] = node;
    list->length++;
}

myhtml_tree_node_t * myhtml_tree_list_current_node(myhtml_tree_list_t* list)
{
    if(list->length == 0)
        return 0;
    
    return list->list[ list->length - 1 ];
}

// stack of open elements
myhtml_tree_list_t * myhtml_tree_open_elements_init(myhtml_tree_t* tree)
{
    return myhtml_tree_list_init();
}

void myhtml_tree_open_elements_clean(myhtml_tree_t* tree)
{
    tree->open_elements->length = 0;
}

myhtml_tree_list_t * myhtml_tree_open_elements_destroy(myhtml_tree_t* tree)
{
    return myhtml_tree_list_destroy(tree->open_elements, true);
}

myhtml_tree_node_t * myhtml_tree_current_node(myhtml_tree_t* tree)
{
    if(tree->open_elements->length == 0) {
        MyCORE_DEBUG("Current node; Open elements is 0");
        return 0;
    }
    
    return tree->open_elements->list[ tree->open_elements->length - 1 ];
}

myhtml_tree_node_t * myhtml_tree_adjusted_current_node(myhtml_tree_t* tree)
{
    if(tree->open_elements->length == 1 && tree->fragment)
        return tree->fragment;
    
    return myhtml_tree_current_node(tree);
}

void myhtml_tree_open_elements_append(myhtml_tree_t* tree, myhtml_tree_node_t* node)
{
    myhtml_tree_list_append(tree->open_elements, node);
}

void myhtml_tree_open_elements_append_after_index(myhtml_tree_t* tree, myhtml_tree_node_t* node, size_t index)
{
    myhtml_tree_list_append_after_index(tree->open_elements, node, index);
}

void myhtml_tree_open_elements_pop(myhtml_tree_t* tree)
{
    if(tree->open_elements->length)
        tree->open_elements->length--;
    
#ifdef MyCORE_BUILD_DEBUG
    if(tree->open_elements->length == 0) {
        MyCORE_DEBUG("Pop open elements; Now, Open Elements set 0; Good, if the end of parsing, otherwise is very bad");
    }
#endif
}

void myhtml_tree_open_elements_remove(myhtml_tree_t* tree, myhtml_tree_node_t* node)
{
    myhtml_tree_node_t** list = tree->open_elements->list;
    
    size_t el_idx = tree->open_elements->length;
    while(el_idx)
    {
        el_idx--;
        
        if(list[el_idx] == node)
        {
            memmove(&list[el_idx], &list[el_idx + 1], sizeof(myhtml_tree_node_t*) * (tree->open_elements->length - el_idx));
            tree->open_elements->length--;
            
            break;
        }
    }
    
#ifdef MyCORE_BUILD_DEBUG
    if(tree->open_elements->length == 0) {
        MyCORE_DEBUG("Remove open elements; Now, Open Elements set 0; Good, if the end of parsing, otherwise is very bad");
    }
#endif
}

void myhtml_tree_open_elements_pop_until(myhtml_tree_t* tree, myhtml_tag_id_t tag_idx, myhtml_namespace_t mynamespace, bool is_exclude)
{
    myhtml_tree_node_t** list = tree->open_elements->list;
    
    while(tree->open_elements->length)
    {
        tree->open_elements->length--;
        
        if(list[ tree->open_elements->length ]->tag_id == tag_idx &&
           // check namespace if set
           (mynamespace == MyHTML_NAMESPACE_UNDEF ||
            list[ tree->open_elements->length ]->ns == mynamespace))
        {
            if(is_exclude)
                tree->open_elements->length++;
            
            break;
        }
    }
    
#ifdef MyCORE_BUILD_DEBUG
    if(tree->open_elements->length == 0) {
        MyCORE_DEBUG("Until open elements; Now, Open Elements set 0; Good, if the end of parsing, otherwise is very bad");
    }
#endif
}

void myhtml_tree_open_elements_pop_until_by_node(myhtml_tree_t* tree, myhtml_tree_node_t* node, bool is_exclude)
{
    myhtml_tree_node_t** list = tree->open_elements->list;
    
    while(tree->open_elements->length)
    {
        tree->open_elements->length--;
        
        if(list[ tree->open_elements->length ] == node) {
            if(is_exclude)
                tree->open_elements->length++;
            
            break;
        }
    }
    
#ifdef MyCORE_BUILD_DEBUG
    if(tree->open_elements->length == 0) {
        MyCORE_DEBUG("Until by node open elements; Now, Open Elements set 0; Good, if the end of parsing, otherwise is very bad");
    }
#endif
}

void myhtml_tree_open_elements_pop_until_by_index(myhtml_tree_t* tree, size_t idx, bool is_exclude)
{
    while(tree->open_elements->length)
    {
        tree->open_elements->length--;
        
        if(tree->open_elements->length == idx) {
            if(is_exclude)
                tree->open_elements->length++;
            
            break;
        }
    }
    
#ifdef MyCORE_BUILD_DEBUG
    if(tree->open_elements->length == 0) {
        MyCORE_DEBUG("Until by index open elements; Now, Open Elements set 0; Good, if the end of parsing, otherwise is very bad");
    }
#endif
}

bool myhtml_tree_open_elements_find_reverse(myhtml_tree_t* tree, myhtml_tree_node_t* idx, size_t* pos)
{
    myhtml_tree_node_t** list = tree->open_elements->list;
    
    size_t i = tree->open_elements->length;
    while(i)
    {
        i--;
        
        if(list[i] == idx) {
            if(pos)
                *pos = i;
            
            return true;
        }
    }
    
    return false;
}

bool myhtml_tree_open_elements_find(myhtml_tree_t* tree, myhtml_tree_node_t* node, size_t* pos)
{
    myhtml_tree_node_t** list = tree->open_elements->list;
    
    for (size_t i = 0; i < tree->open_elements->length; i++)
    {
        if(list[i] == node) {
            if(pos)
                *pos = i;
            
            return true;
        }
    }
    
    return false;
}

myhtml_tree_node_t * myhtml_tree_open_elements_find_by_tag_idx_reverse(myhtml_tree_t* tree, myhtml_tag_id_t tag_idx, myhtml_namespace_t mynamespace, size_t* return_index)
{
    myhtml_tree_node_t** list = tree->open_elements->list;
    
    size_t i = tree->open_elements->length;
    while(i)
    {
        i--;
        
        if(list[i]->tag_id == tag_idx &&
           (mynamespace == MyHTML_NAMESPACE_UNDEF || list[i]->ns == mynamespace))
        {
            if(return_index)
                *return_index = i;
            
            return list[i];
        }
    }
    
    return NULL;
}

myhtml_tree_node_t * myhtml_tree_open_elements_find_by_tag_idx(myhtml_tree_t* tree, myhtml_tag_id_t tag_idx, myhtml_namespace_t mynamespace, size_t* return_index)
{
    myhtml_tree_node_t** list = tree->open_elements->list;
    
    for (size_t i = 0; i < tree->open_elements->length; i++)
    {
        if(list[i]->tag_id == tag_idx &&
           (mynamespace == MyHTML_NAMESPACE_UNDEF || list[i]->ns == mynamespace))
        {
            if(return_index)
                *return_index = i;
            
            return list[i];
        }
    }
    
    return NULL;
}

void myhtml_tree_generate_implied_end_tags(myhtml_tree_t* tree, myhtml_tag_id_t exclude_tag_idx, myhtml_namespace_t mynamespace)
{
    if(tree->open_elements->length == 0) {
        MyCORE_DEBUG("Generate implied end tags; Open elements is 0");
        return;
    }
    
    while(tree->open_elements->length > 0)
    {
        myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
        
#ifdef MyCORE_BUILD_DEBUG
        if(current_node == NULL) {
            MyCORE_DEBUG_ERROR("Generate implied end tags; Current node is NULL! This is very bad");
        }
#endif
        
        switch (current_node->tag_id) {
            case MyHTML_TAG_DD:
            case MyHTML_TAG_DT:
            case MyHTML_TAG_LI:
            case MyHTML_TAG_MENUITEM:
            case MyHTML_TAG_OPTGROUP:
            case MyHTML_TAG_OPTION:
            case MyHTML_TAG_P:
            case MyHTML_TAG_RB:
            case MyHTML_TAG_RP:
            case MyHTML_TAG_RT:
            case MyHTML_TAG_RTC:
                if(exclude_tag_idx == current_node->tag_id &&
                   (mynamespace == MyHTML_NAMESPACE_UNDEF || current_node->ns == mynamespace))
                    return;
                
                myhtml_tree_open_elements_pop(tree);
                continue;
                
            default:
                return;
        }
    }
}

void myhtml_tree_generate_all_implied_end_tags(myhtml_tree_t* tree, myhtml_tag_id_t exclude_tag_idx, myhtml_namespace_t mynamespace)
{
    if(tree->open_elements->length == 0) {
        MyCORE_DEBUG("Generate all implied end tags; Open elements is 0");
        return;
    }
    
    while(tree->open_elements->length > 0)
    {
        myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
        
#ifdef MyCORE_BUILD_DEBUG
        if(current_node == NULL) {
            MyCORE_DEBUG_ERROR("Generate all implied end tags; Current node is NULL! This is very bad");
        }
#endif
        
        switch (current_node->tag_id) {
            case MyHTML_TAG_CAPTION:
            case MyHTML_TAG_COLGROUP:
            case MyHTML_TAG_DD:
            case MyHTML_TAG_DT:
            case MyHTML_TAG_LI:
            case MyHTML_TAG_OPTGROUP:
            case MyHTML_TAG_OPTION:
            case MyHTML_TAG_P:
            case MyHTML_TAG_RB:
            case MyHTML_TAG_RP:
            case MyHTML_TAG_RT:
            case MyHTML_TAG_RTC:
            case MyHTML_TAG_TBODY:
            case MyHTML_TAG_TD:
            case MyHTML_TAG_TFOOT:
            case MyHTML_TAG_TH:
            case MyHTML_TAG_THEAD:
            case MyHTML_TAG_TR:
                if(exclude_tag_idx == current_node->tag_id &&
                   (mynamespace == MyHTML_NAMESPACE_UNDEF || current_node->ns == mynamespace))
                    return;
                
                myhtml_tree_open_elements_pop(tree);
                continue;
                
            default:
                return;
        }
    }
}

void myhtml_tree_reset_insertion_mode_appropriately(myhtml_tree_t* tree)
{
    if(tree->open_elements->length == 0) {
        MyCORE_DEBUG("Reset insertion mode appropriately; Open elements is 0");
        return;
    }
    
    size_t i = tree->open_elements->length;
    
    // step 1
    bool last = false;
    myhtml_tree_node_t** list = tree->open_elements->list;
    
    // step 3
    while(i)
    {
        i--;
        
        // step 2
        myhtml_tree_node_t* node = list[i];
        
#ifdef MyCORE_BUILD_DEBUG
        if(node == NULL) {
            MyCORE_DEBUG_ERROR("Reset insertion mode appropriately; node is NULL! This is very bad");
        }
#endif
        
        if(i == 0) {
            last = true;
            
            if(tree->fragment) {
                node = tree->fragment;
            }
        }
        
        if(node->ns != MyHTML_NAMESPACE_HTML) {
            if(last) {
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                return;
            }
            
            continue;
        }
        
        // step 4
        if(node->tag_id == MyHTML_TAG_SELECT)
        {
            // step 4.1
            if(last) {
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_SELECT;
                return;
            }
            
            // step 4.2
            size_t ancestor = i;
            
            while(1)
            {
                // step 4.3
                if(ancestor == 0) {
                    tree->insert_mode = MyHTML_INSERTION_MODE_IN_SELECT;
                    return;
                }
                
#ifdef MyCORE_BUILD_DEBUG
                if(ancestor == 0) {
                    MyCORE_DEBUG_ERROR("Reset insertion mode appropriately; Ancestor is 0! This is very, very bad");
                }
#endif
                
                // step 4.4
                ancestor--;
                
                // step 4.5
                if(list[ancestor]->tag_id == MyHTML_TAG_TEMPLATE) {
                    tree->insert_mode = MyHTML_INSERTION_MODE_IN_SELECT;
                    return;
                }
                // step 4.6
                else if(list[ancestor]->tag_id == MyHTML_TAG_TABLE) {
                    tree->insert_mode = MyHTML_INSERTION_MODE_IN_SELECT_IN_TABLE;
                    return;
                }
            }
        }
        
        // step 5-15
        switch (node->tag_id) {
            case MyHTML_TAG_TD:
            case MyHTML_TAG_TH:
                if(last == false) {
                    tree->insert_mode = MyHTML_INSERTION_MODE_IN_CELL;
                    return;
                }
                break;
                
            case MyHTML_TAG_TR:
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_ROW;
                return;
                
            case MyHTML_TAG_TBODY:
            case MyHTML_TAG_TFOOT:
            case MyHTML_TAG_THEAD:
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE_BODY;
                return;
                
            case MyHTML_TAG_CAPTION:
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_CAPTION;
                return;
                
            case MyHTML_TAG_COLGROUP:
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_COLUMN_GROUP;
                return;
                
            case MyHTML_TAG_TABLE:
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_TABLE;
                return;
                
            case MyHTML_TAG_TEMPLATE:
                tree->insert_mode = tree->template_insertion->list[(tree->template_insertion->length - 1)];
                return;
                
            case MyHTML_TAG_HEAD:
                if(last == false) {
                    tree->insert_mode = MyHTML_INSERTION_MODE_IN_HEAD;
                    return;
                }
                break;
                
            case MyHTML_TAG_BODY:
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
                return;
                
            case MyHTML_TAG_FRAMESET:
                tree->insert_mode = MyHTML_INSERTION_MODE_IN_FRAMESET;
                return;
                
            case MyHTML_TAG_HTML:
            {
                if(tree->node_head) {
                    tree->insert_mode = MyHTML_INSERTION_MODE_AFTER_HEAD;
                    return;
                }
                
                tree->insert_mode = MyHTML_INSERTION_MODE_BEFORE_HEAD;
                return;
            }
                
            default:
                break;
        }
        
        // step 16
        if(last) {
            tree->insert_mode = MyHTML_INSERTION_MODE_IN_BODY;
            return;
        }
        
        // step 17
    }
    
    tree->insert_mode = MyHTML_INSERTION_MODE_INITIAL;
}

// stack of active formatting elements
myhtml_tree_list_t * myhtml_tree_active_formatting_init(myhtml_tree_t* tree)
{
    return myhtml_tree_list_init();
}

void myhtml_tree_active_formatting_clean(myhtml_tree_t* tree)
{
    tree->active_formatting->length = 0;
}

myhtml_tree_list_t * myhtml_tree_active_formatting_destroy(myhtml_tree_t* tree)
{
    return myhtml_tree_list_destroy(tree->active_formatting, true);
}

bool myhtml_tree_active_formatting_is_marker(myhtml_tree_t* tree, myhtml_tree_node_t* node)
{
#ifdef MyCORE_BUILD_DEBUG
    if(node == NULL) {
        MyCORE_DEBUG_ERROR("Active formatting is marker; node is NULL!");
    }
#endif
    
    if(tree->myhtml->marker == node)
        return true;
    
    switch (node->tag_id) {
        case MyHTML_TAG_APPLET:
        case MyHTML_TAG_BUTTON:
        case MyHTML_TAG_OBJECT:
        case MyHTML_TAG_MARQUEE:
        case MyHTML_TAG_TD:
        case MyHTML_TAG_TH:
        case MyHTML_TAG_CAPTION:
            return true;
            
        default:
            break;
    }
    
    return false;
}

void myhtml_tree_active_formatting_append(myhtml_tree_t* tree, myhtml_tree_node_t* node)
{
    myhtml_tree_list_append( tree->active_formatting, node);
}

void myhtml_tree_active_formatting_pop(myhtml_tree_t* tree)
{
    if(tree->active_formatting->length)
        tree->active_formatting->length--;
    
#ifdef MyCORE_BUILD_DEBUG
    if(tree->active_formatting->length == 0) {
        MyCORE_DEBUG("Pop active formatting; length is 0");
    }
#endif
}

void myhtml_tree_active_formatting_remove(myhtml_tree_t* tree, myhtml_tree_node_t* node)
{
    myhtml_tree_node_t** list = tree->active_formatting->list;
    
    size_t el_idx = tree->active_formatting->length;
    while(el_idx)
    {
        el_idx--;
        
        if(list[el_idx] == node)
        {
            memmove(&list[el_idx], &list[el_idx + 1], sizeof(myhtml_tree_node_t*) * (tree->active_formatting->length - el_idx));
            tree->active_formatting->length--;
            
            break;
        }
    }
    
#ifdef MyCORE_BUILD_DEBUG
    if(tree->active_formatting->length == 0) {
        // MyCORE_DEBUG("Remove active formatting; length is 0");
    }
#endif
}

void myhtml_tree_active_formatting_remove_by_index(myhtml_tree_t* tree, size_t idx)
{
    myhtml_tree_node_t** list = tree->active_formatting->list;
    
    memmove(&list[idx], &list[idx + 1], sizeof(myhtml_tree_node_t*) * (tree->active_formatting->length - idx));
    tree->active_formatting->length--;
    
#ifdef MyCORE_BUILD_DEBUG
    if(tree->active_formatting->length == 0) {
        MyCORE_DEBUG("Remove active formatting by index; length is 0");
    }
#endif
}

void myhtml_tree_active_formatting_append_with_check(myhtml_tree_t* tree, myhtml_tree_node_t* node)
{
//    if(myhtml_tree_active_formatting_is_marker(tree, node)) {
//        myhtml_tree_active_formatting_append(tree, node);
//        return;
//    }
    
    myhtml_tree_node_t** list = tree->active_formatting->list;
    size_t i = tree->active_formatting->length;
    size_t earliest_idx = (i ? (i - 1) : 0);
    size_t count = 0;
    
    while(i)
    {
        i--;
        
#ifdef MyCORE_BUILD_DEBUG
        if(list[i] == NULL) {
            MyCORE_DEBUG("Appen active formatting with check; list[" MyCORE_FORMAT_Z "] is NULL", i);
        }
#endif
        
        if(myhtml_tree_active_formatting_is_marker(tree, list[i]))
            break;
        
        if(list[i]->token && node->token)
        {
            myhtml_token_node_wait_for_done(tree->token, list[i]->token);
            myhtml_token_node_wait_for_done(tree->token, node->token);
            
            if(list[i]->ns == node->ns &&
               list[i]->tag_id == node->tag_id &&
               myhtml_token_attr_compare(list[i]->token, node->token))
            {
                count++;
                earliest_idx = i;
            }
        }
    }
    
    if(count >= 3)
        myhtml_tree_active_formatting_remove_by_index(tree, earliest_idx);
    
    myhtml_tree_active_formatting_append(tree, node);
}

myhtml_tree_node_t * myhtml_tree_active_formatting_current_node(myhtml_tree_t* tree)
{
    if(tree->active_formatting->length == 0) {
        MyCORE_DEBUG("Current node active formatting; length is 0");
        return 0;
    }
    
    return tree->active_formatting->list[ tree->active_formatting->length - 1 ];
}

bool myhtml_tree_active_formatting_find(myhtml_tree_t* tree, myhtml_tree_node_t* node, size_t* return_idx)
{
    myhtml_tree_node_t** list = tree->active_formatting->list;
    
    size_t i = tree->active_formatting->length;
    while(i)
    {
        i--;
        
        if(list[i] == node) {
            if(return_idx)
                *return_idx = i;
            
            return true;
        }
    }
    
    return false;
}

void myhtml_tree_active_formatting_up_to_last_marker(myhtml_tree_t* tree)
{
    // Step 1: Let entry be the last (most recently added) entry in the list of active formatting elements.
    myhtml_tree_node_t** list = tree->active_formatting->list;
    
    // Step 2: Remove entry from the list of active formatting elements.
    if(tree->active_formatting->length == 0)
        return;
    
#ifdef MyCORE_BUILD_DEBUG
    if(list[ tree->active_formatting->length ] == NULL) {
        MyCORE_DEBUG("Up to last marker active formatting; list[" MyCORE_FORMAT_Z "] is NULL", tree->active_formatting->length);
    }
#endif
    
    while(tree->active_formatting->length)
    {
        tree->active_formatting->length--;
        
#ifdef MyCORE_BUILD_DEBUG
        if(list[ tree->active_formatting->length ] == NULL) {
            MyCORE_DEBUG("Up to last marker active formatting; list[" MyCORE_FORMAT_Z "] is NULL", tree->active_formatting->length);
        }
#endif
        
        if(myhtml_tree_active_formatting_is_marker(tree, list[ tree->active_formatting->length ])) {
            // include marker
            //tree->active_formatting->length++;
            break;
        }
    }
}

myhtml_tree_node_t * myhtml_tree_active_formatting_between_last_marker(myhtml_tree_t* tree, myhtml_tag_id_t tag_idx, size_t* return_idx)
{
    myhtml_tree_node_t** list = tree->active_formatting->list;
    
    size_t i = tree->active_formatting->length;
    while(i)
    {
        i--;
        
#ifdef MyCORE_BUILD_DEBUG
        if(list[i] == NULL) {
            MyCORE_DEBUG("Between last marker active formatting; list[" MyCORE_FORMAT_Z "] is NULL", i);
        }
#endif
        
        if(myhtml_tree_active_formatting_is_marker(tree, list[i]))
            break;
        else if(list[i]->tag_id == tag_idx && list[i]->ns == MyHTML_NAMESPACE_HTML) {
            if(return_idx)
                *return_idx = i;
            return list[i];
        }
    }
    
    return NULL;
}

void myhtml_tree_active_formatting_reconstruction(myhtml_tree_t* tree)
{
    myhtml_tree_list_t* af = tree->active_formatting;
    myhtml_tree_node_t** list = af->list;
    
    // step 1
    if(af->length == 0)
        return;
    
    // step 2--3
    size_t af_idx = af->length - 1;
    
    if(myhtml_tree_active_formatting_is_marker(tree, list[af_idx]) ||
       myhtml_tree_open_elements_find(tree, list[af_idx], NULL))
        return;
    
    // step 4--6
    while (af_idx)
    {
        af_idx--;
        
#ifdef MyCORE_BUILD_DEBUG
        if(list[af_idx] == NULL) {
            MyCORE_DEBUG("Formatting reconstruction; Step 4--6; list[" MyCORE_FORMAT_Z "] is NULL", af_idx);
        }
#endif
        
        if(myhtml_tree_active_formatting_is_marker(tree, list[af_idx]) ||
           myhtml_tree_open_elements_find(tree, list[af_idx], NULL))
        {
            af_idx++; // need if 0
            break;
        }
    }
    
    while (af_idx < af->length)
    {
#ifdef MyCORE_BUILD_DEBUG
        if(list[af_idx] == NULL) {
            MyCORE_DEBUG("Formatting reconstruction; Next steps; list[" MyCORE_FORMAT_Z "] is NULL", af_idx);
        }
#endif
        
        myhtml_tree_node_t* node = myhtml_tree_node_clone(list[af_idx]);
        myhtml_tree_node_insert_by_node(tree, node);
        
        list[af_idx] = node;
        
        af_idx++;
    }
}

bool myhtml_tree_adoption_agency_algorithm(myhtml_tree_t* tree, myhtml_token_node_t* token, myhtml_tag_id_t subject_tag_idx)
{
    if(tree->open_elements->length == 0)
        return false;
    
    size_t oel_curr_index = tree->open_elements->length - 1;
    
    myhtml_tree_node_t**  oel_list     = tree->open_elements->list;
    myhtml_tree_node_t**  afe_list     = tree->active_formatting->list;
    myhtml_tree_node_t*   current_node = oel_list[oel_curr_index];
    
#ifdef MyCORE_BUILD_DEBUG
    if(current_node == NULL) {
        MyCORE_DEBUG_ERROR("Adoption agency algorithm; Current node is NULL");
    }
#endif
    
    // step 1
    if(current_node->ns == MyHTML_NAMESPACE_HTML && current_node->tag_id == subject_tag_idx &&
       myhtml_tree_active_formatting_find(tree, current_node, NULL) == false)
    {
        myhtml_tree_open_elements_pop(tree);
        return false;
    }
    
    // step 2, 3
    int loop = 0;
    
    while (loop < 8)
    {
        // step 4
        loop++;
        
        // step 5
        size_t afe_index = 0;
        myhtml_tree_node_t* formatting_element = NULL;
        {
            myhtml_tree_node_t** list = tree->active_formatting->list;
            
            size_t i = tree->active_formatting->length;
            while(i)
            {
                i--;
                
                if(myhtml_tree_active_formatting_is_marker(tree, list[i]))
                    return false;
                else if(list[i]->tag_id == subject_tag_idx) {
                    afe_index = i;
                    formatting_element = list[i];
                    break;
                }
            }
        }
        
        //myhtml_tree_node_t* formatting_element = myhtml_tree_active_formatting_between_last_marker(tree, subject_tag_idx, &afe_index);
        
        // If there is no such element, then abort these steps and instead act as described in the
        // ===> "any other end tag" entry above.
        if(formatting_element == NULL) {
            return true;
        }
        
        // step 6
        size_t oel_format_el_idx;
        if(myhtml_tree_open_elements_find(tree, formatting_element, &oel_format_el_idx) == false) {
            myhtml_tree_active_formatting_remove(tree, formatting_element);
            return false;
        }
        
        // step 7
        if(myhtml_tree_element_in_scope_by_node(formatting_element, MyHTML_TAG_CATEGORIES_SCOPE) == false) {
            /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:AAA_FORMATTING_ELEMENT_NOT_FOUND LEVEL:ERROR NODE:formatting_element */
            return false;
        }
        
        // step 8
        //if(afe_last != list[i])
        //    fprintf(stderr, "oh");
        
        // step 9
        myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
        if(current_node->ns != formatting_element->ns ||
           current_node->tag_id != formatting_element->tag_id) {
            // parse error
            /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR */
            /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:current_node->token NEED:formatting_element->token HAVE_TAG_ID:current_node->tag_id HAVE_NS:current_node->ns NEED_TAG_ID:formatting_element->tag_id NEED_NS:formatting_element->ns */
        }
        
        // 10
        // Let furthest block be the topmost node in the stack of open elements
        // that is lower in the stack than formatting element, and is an element in the special category. T
        // here might not be one.
        myhtml_tree_node_t* furthest_block = NULL;
        size_t idx_furthest_block = 0;
        for (idx_furthest_block = oel_format_el_idx; idx_furthest_block < tree->open_elements->length; idx_furthest_block++)
        {
            const myhtml_tag_context_t *tag_ctx = myhtml_tag_get_by_id(tree->tags, oel_list[idx_furthest_block]->tag_id);
            
            if(tag_ctx->cats[oel_list[idx_furthest_block]->ns] & MyHTML_TAG_CATEGORIES_SPECIAL) {
                furthest_block = oel_list[idx_furthest_block];
                break;
            }
        }
        
        // step 11
        // If there is no furthest block, then the UA must first pop all the nodes from the bottom
        // of the stack of open elements, from the current node up to and including formatting element,
        // then remove formatting element from the list of active formatting elements, and finally abort these steps.
        if(furthest_block == NULL)
        {
            while(myhtml_tree_current_node(tree) != formatting_element) {
                myhtml_tree_open_elements_pop(tree);
            }
            
            myhtml_tree_open_elements_pop(tree); // and including formatting element
            myhtml_tree_active_formatting_remove(tree, formatting_element);
            
            return false;
        }
        
        /* %EXTERNAL% VALIDATOR:RULES TOKEN STATUS:AAA_BEGIN LEVEL:INFO */
        
#ifdef MyCORE_BUILD_DEBUG
        if(oel_format_el_idx == 0) {
            MyCORE_DEBUG_ERROR("Adoption agency algorithm; Step 11; oel_format_el_idx is 0; Bad!");
        }
#endif
        
        // step 12
        myhtml_tree_node_t* common_ancestor = oel_list[oel_format_el_idx - 1];
        
#ifdef MyCORE_BUILD_DEBUG
        if(common_ancestor == NULL) {
            MyCORE_DEBUG_ERROR("Adoption agency algorithm; Step 11; common_ancestor is NULL");
        }
#endif
        
        // step 13
        size_t bookmark = afe_index + 1;
        
        // step 14
        myhtml_tree_node_t *node = furthest_block, *last = furthest_block;
        size_t index_oel_node = idx_furthest_block;
        
        // step 14.1
        for(int inner_loop = 0;;)
        {
            // step 14.2
            inner_loop++;
            
            // step 14.3
            size_t node_index;
            if(myhtml_tree_open_elements_find(tree, node, &node_index) == false) {
                node_index = index_oel_node;
            }
            
            if(node_index > 0)
                node_index--;
            else {
                MyCORE_DEBUG_ERROR("Adoption agency algorithm; decrement node_index, node_index is null");
                return false;
            }
            
            index_oel_node = node_index;
            
            node = oel_list[node_index];
            
#ifdef MyCORE_BUILD_DEBUG
            if(node == NULL) {
                MyCORE_DEBUG_ERROR("Adoption agency algorithm; Step 13.3; node is NULL");
            }
#endif
            // step 14.4
            if(node == formatting_element)
                break;
            
            // step 14.5
            size_t afe_node_index;
            bool is_exists = myhtml_tree_active_formatting_find(tree, node, &afe_node_index);
            if(inner_loop > 3 && is_exists) {
                myhtml_tree_active_formatting_remove_by_index(tree, afe_node_index);
                
                if(afe_node_index < bookmark)
                    bookmark--;
                
                // If inner loop counter is greater than three and node is in the list of active formatting elements,
                // then remove node from the list of active formatting elements.
                continue;
            }
            
            // step 14.6
            if(is_exists == false) {
                myhtml_tree_open_elements_remove(tree, node);
                continue;
            }
            
            // step 14.7
            myhtml_tree_node_t* clone = myhtml_tree_node_clone(node);
            
            clone->ns = MyHTML_NAMESPACE_HTML;
            
            afe_list[afe_node_index] = clone;
            oel_list[node_index] = clone;
            
            node = clone;
            
            // step 14.8
            if(last == furthest_block) {
                bookmark = afe_node_index + 1;
                
#ifdef MyCORE_BUILD_DEBUG
                if(bookmark >= tree->active_formatting->length) {
                    MyCORE_DEBUG_ERROR("Adoption agency algorithm; Step 13.8; bookmark >= open_elements length");
                }
#endif
            }
            
            // step 14.9
            if(last->parent)
                myhtml_tree_node_remove(last);
            
            myhtml_tree_node_add_child(node, last);
            
            // step 14.10
            last = node;
        }
        
        if(last->parent)
            myhtml_tree_node_remove(last);
        
        // step 15
        enum myhtml_tree_insertion_mode insert_mode;
        common_ancestor = myhtml_tree_appropriate_place_inserting(tree, common_ancestor, &insert_mode);
        myhtml_tree_node_insert_by_mode(common_ancestor, last, insert_mode);
        
        // step 16
        myhtml_tree_node_t* new_formatting_element = myhtml_tree_node_clone(formatting_element);
        
        new_formatting_element->ns = MyHTML_NAMESPACE_HTML;
        
        // step 17
        myhtml_tree_node_t * furthest_block_child = furthest_block->child;
        
        while (furthest_block_child) {
            myhtml_tree_node_t *next = furthest_block_child->next;
            myhtml_tree_node_remove(furthest_block_child);
            
            myhtml_tree_node_add_child(new_formatting_element, furthest_block_child);
            furthest_block_child = next;
        }
        
        // step 18
        myhtml_tree_node_add_child(furthest_block, new_formatting_element);
        
        // step 19
        if(myhtml_tree_active_formatting_find(tree, formatting_element, &afe_index) == false)
            return false;
        
        if(afe_index < bookmark)
            bookmark--;
        
#ifdef MyCORE_BUILD_DEBUG
        if(bookmark >= tree->active_formatting->length) {
            MyCORE_DEBUG_ERROR("Adoption agency algorithm; Before Step 18; bookmark (" MyCORE_FORMAT_Z ") >= open_elements length", bookmark);
        }
#endif
        
        myhtml_tree_active_formatting_remove_by_index(tree, afe_index);
        myhtml_tree_list_insert_by_index(tree->active_formatting, new_formatting_element, bookmark);
        
        // step 20
        myhtml_tree_open_elements_remove(tree, formatting_element);
        
        if(myhtml_tree_open_elements_find(tree, furthest_block, &idx_furthest_block)) {
            myhtml_tree_list_insert_by_index(tree->open_elements, new_formatting_element, idx_furthest_block + 1);
        }
        else {
            MyCORE_DEBUG_ERROR("Adoption agency algorithm; Step 19; can't find furthest_block in open elements");
        }
    }
    
    return false;
}

myhtml_tree_node_t * myhtml_tree_appropriate_place_inserting(myhtml_tree_t* tree, myhtml_tree_node_t* override_target,
                                                             enum myhtml_tree_insertion_mode* mode)
{
    *mode = MyHTML_TREE_INSERTION_MODE_DEFAULT;
    
    // step 1
    myhtml_tree_node_t* target = override_target ? override_target : myhtml_tree_current_node(tree);
    
    // step 2
    myhtml_tree_node_t* adjusted_location;
    
    if(tree->foster_parenting) {
#ifdef MyCORE_BUILD_DEBUG
        if(target == NULL) {
            MyCORE_DEBUG_ERROR("Appropriate place inserting; Step 2; target is NULL in return value! This IS very bad");
        }
#endif
        if(target->ns != MyHTML_NAMESPACE_HTML)
            return target;
        
        switch (target->tag_id) {
            case MyHTML_TAG_TABLE:
            case MyHTML_TAG_TBODY:
            case MyHTML_TAG_TFOOT:
            case MyHTML_TAG_THEAD:
            case MyHTML_TAG_TR:
            {
                size_t idx_template, idx_table;
                
                // step 2.1-2
                myhtml_tree_node_t* last_template = myhtml_tree_open_elements_find_by_tag_idx_reverse(tree, MyHTML_TAG_TEMPLATE, MyHTML_NAMESPACE_HTML, &idx_template);
                myhtml_tree_node_t* last_table = myhtml_tree_open_elements_find_by_tag_idx_reverse(tree, MyHTML_TAG_TABLE, MyHTML_NAMESPACE_HTML, &idx_table);
                
                // step 2.3
                if(last_template && (last_table == NULL || idx_template > idx_table))
                {
                    return last_template;
                }
                
                // step 2.4
                else if(last_table == NULL)
                {
                    adjusted_location = tree->open_elements->list[0];
                    break;
                }
                
                // step 2.5
                else if(last_table->parent)
                {
                    //adjusted_location = last_table->parent;
                    
                    if(last_table->prev) {
                        adjusted_location = last_table->prev;
                        *mode = MyHTML_TREE_INSERTION_MODE_AFTER;
                    }
                    else {
                        adjusted_location = last_table;
                        *mode = MyHTML_TREE_INSERTION_MODE_BEFORE;
                    }
                    
                    break;
                }
                
#ifdef MyCORE_BUILD_DEBUG
                if(idx_table == 0) {
                    MyCORE_DEBUG_ERROR("Appropriate place inserting; Step 2.5; idx_table is 0");
                }
#endif
                
                // step 2.6-7
                adjusted_location = tree->open_elements->list[idx_table - 1];
                
                break;
            }
                
            default:
                adjusted_location = target;
                break;
        }
    }
    else {
#ifdef MyCORE_BUILD_DEBUG
        if(target == NULL) {
            MyCORE_DEBUG_ERROR("Appropriate place inserting; Step 3-5; target is NULL in return value! This IS very bad");
        }
#endif
        
        // step 3-4
        return target;
    }
    
    // step 3-4
    return adjusted_location;
}

myhtml_tree_node_t * myhtml_tree_appropriate_place_inserting_in_tree(myhtml_tree_node_t* target, enum myhtml_tree_insertion_mode* mode)
{
    *mode = MyHTML_TREE_INSERTION_MODE_BEFORE;
    
    // step 2
    myhtml_tree_node_t* adjusted_location;
    
    if(target->tree->foster_parenting) {
#ifdef MyCORE_BUILD_DEBUG
        if(target == NULL) {
            MyCORE_DEBUG_ERROR("Appropriate place inserting; Step 2; target is NULL in return value! This IS very bad");
        }
#endif
        
        if(target->ns != MyHTML_NAMESPACE_HTML)
            return target;
        
        switch (target->tag_id) {
            case MyHTML_TAG_TABLE:
            case MyHTML_TAG_TBODY:
            case MyHTML_TAG_TFOOT:
            case MyHTML_TAG_THEAD:
            case MyHTML_TAG_TR:
            {
                // step 2.1-2
                myhtml_tree_node_t* last_template = myhtml_tree_node_find_parent_by_tag_id(target, MyHTML_TAG_TEMPLATE);
                myhtml_tree_node_t* last_table = myhtml_tree_node_find_parent_by_tag_id(target, MyHTML_TAG_TABLE);
                myhtml_tree_node_t* last_table_in_template = NULL;
                
                if(last_template) {
                    last_table_in_template = myhtml_tree_node_find_parent_by_tag_id(last_template, MyHTML_TAG_TABLE);
                }
                
                // step 2.3
                if(last_template && (last_table == NULL || last_table != last_table_in_template))
                {
                    *mode = MyHTML_TREE_INSERTION_MODE_DEFAULT;
                    adjusted_location = last_template;
                    break;
                }
                
                // step 2.4
                else if(last_table == NULL)
                {
                    adjusted_location = target;
                    break;
                }
                
                // step 2.5
                else if(last_table->parent)
                {
                    //adjusted_location = last_table->parent;
                    
                    if(last_table->prev) {
                        adjusted_location = last_table->prev;
                        *mode = MyHTML_TREE_INSERTION_MODE_AFTER;
                    }
                    else {
                        adjusted_location = last_table;
                    }
                    
                    break;
                }
                
                // step 2.6-7
                adjusted_location = target;
                
                break;
            }
                
            default:
                *mode = MyHTML_TREE_INSERTION_MODE_DEFAULT;
                adjusted_location = target;
                break;
        }
    }
    else {
#ifdef MyCORE_BUILD_DEBUG
        if(target == NULL) {
            MyCORE_DEBUG_ERROR("Appropriate place inserting; Step 3-5; target is NULL in return value! This IS very bad");
        }
#endif
        
        *mode = MyHTML_TREE_INSERTION_MODE_DEFAULT;
        // step 3-4
        return target;
    }
    
    // step 3-4
    return adjusted_location;
}


// stack of template insertion modes
myhtml_tree_insertion_list_t * myhtml_tree_template_insertion_init(myhtml_tree_t* tree)
{
    myhtml_tree_insertion_list_t* list = mycore_malloc(sizeof(myhtml_tree_insertion_list_t));
    
    list->length = 0;
    list->size = 1024;
    list->list = (enum myhtml_insertion_mode*)mycore_malloc(sizeof(enum myhtml_insertion_mode) * list->size);
    
    tree->template_insertion = list;
    
    return list;
}

void myhtml_tree_template_insertion_clean(myhtml_tree_t* tree)
{
    tree->template_insertion->length = 0;
}

myhtml_tree_insertion_list_t * myhtml_tree_template_insertion_destroy(myhtml_tree_t* tree)
{
    if(tree->template_insertion == NULL)
        return NULL;
        
    if(tree->template_insertion->list)
        mycore_free(tree->template_insertion->list);
    
    if(tree->template_insertion)
        mycore_free(tree->template_insertion);
    
    return NULL;
}

void myhtml_tree_template_insertion_append(myhtml_tree_t* tree, enum myhtml_insertion_mode insert_mode)
{
    myhtml_tree_insertion_list_t* list = tree->template_insertion;
    
    if(list->length >= list->size) {
        list->size <<= 1;
        
        enum myhtml_insertion_mode* tmp = (enum myhtml_insertion_mode*)mycore_realloc(list->list,
                                                                         sizeof(enum myhtml_insertion_mode) * list->size);
        
        if(tmp)
            list->list = tmp;
    }
    
    list->list[list->length] = insert_mode;
    list->length++;
}

void myhtml_tree_template_insertion_pop(myhtml_tree_t* tree)
{
    if(tree->template_insertion->length)
        tree->template_insertion->length--;

#ifdef MyCORE_BUILD_DEBUG
    if(tree->template_insertion->length == 0) {
        MyCORE_DEBUG("Pop template insertion; length is 0");
    }
#endif
}

size_t myhtml_tree_template_insertion_length(myhtml_tree_t* tree)
{
    return tree->template_insertion->length;
}

// token list
myhtml_tree_token_list_t * myhtml_tree_token_list_init(void)
{
    myhtml_tree_token_list_t* list = mycore_malloc(sizeof(myhtml_tree_token_list_t));
    
    list->length = 0;
    list->size = 4096;
    list->list = (myhtml_token_node_t**)mycore_malloc(sizeof(myhtml_token_node_t*) * list->size);
    
    return list;
}

void myhtml_tree_token_list_clean(myhtml_tree_token_list_t* list)
{
    list->length = 0;
}

myhtml_tree_token_list_t * myhtml_tree_token_list_destroy(myhtml_tree_token_list_t* list, bool destroy_self)
{
    if(list == NULL)
        return NULL;
    
    if(list->list)
        mycore_free(list->list);
    
    if(destroy_self && list) {
        mycore_free(list);
        return NULL;
    }
    
    return list;
}

void myhtml_tree_token_list_append(myhtml_tree_token_list_t* list, myhtml_token_node_t* token)
{
    if(list->length >= list->size) {
        list->size <<= 1;
        
        myhtml_token_node_t** tmp = (myhtml_token_node_t**)mycore_realloc(list->list, sizeof(myhtml_token_node_t*) * list->size);
        
        if(tmp)
            list->list = tmp;
    }
    
    list->list[list->length] = token;
    list->length++;
}

void myhtml_tree_token_list_append_after_index(myhtml_tree_token_list_t* list, myhtml_token_node_t* token, size_t index)
{
    if(list->length >= list->size) {
        list->size <<= 1;
        
        myhtml_token_node_t** tmp = (myhtml_token_node_t**)mycore_realloc(list->list, sizeof(myhtml_token_node_t*) * list->size);
        
        if(tmp)
            list->list = tmp;
    }
    
    myhtml_token_node_t** node_list = list->list;
    size_t el_idx = index;
    
    while(el_idx > list->length) {
        node_list[(el_idx + 1)] = node_list[el_idx];
        el_idx++;
    }
    
    list->list[(index + 1)] = token;
    list->length++;
}

myhtml_token_node_t * myhtml_tree_token_list_current_node(myhtml_tree_token_list_t* list)
{
    if(list->length == 0) {
        MyCORE_DEBUG("Token list current node; length is 0");
        return NULL;
    }
    
    return list->list[ list->length - 1 ];
}

// other
void myhtml_tree_tags_close_p(myhtml_tree_t* tree, myhtml_token_node_t* token)
{
    myhtml_tree_generate_implied_end_tags(tree, MyHTML_TAG_P, MyHTML_NAMESPACE_HTML);
    
    myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
    if(myhtml_is_html_node(current_node, MyHTML_TAG_P) == false) {
        // parse error
        /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_NO_EXPECTED LEVEL:ERROR */
        /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:current_node->token NEED:NULL HAVE_TAG_ID:current_node->tag_id HAVE_NS:current_node->ns NEED_TAG_ID:MyHTML_TAG_P NEED_NS:MyHTML_NAMESPACE_HTML */
    }
    
    myhtml_tree_open_elements_pop_until(tree, MyHTML_TAG_P, MyHTML_NAMESPACE_HTML, false);
}

myhtml_tree_node_t * myhtml_tree_generic_raw_text_element_parsing_algorithm(myhtml_tree_t* tree, myhtml_token_node_t* token_node)
{
    myhtml_tree_node_t* node = myhtml_tree_node_insert_by_token(tree, token_node, MyHTML_NAMESPACE_HTML);
    
    tree->orig_insert_mode = tree->insert_mode;
    tree->insert_mode      = MyHTML_INSERTION_MODE_TEXT;
    
    return node;
}

void myhtml_tree_clear_stack_back_table_context(myhtml_tree_t* tree)
{
    myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
    
    while(!((current_node->tag_id   == MyHTML_TAG_TABLE       ||
             current_node->tag_id      == MyHTML_TAG_TEMPLATE ||
             current_node->tag_id      == MyHTML_TAG_HTML)    &&
            current_node->ns == MyHTML_NAMESPACE_HTML))
    {
        myhtml_tree_open_elements_pop(tree);
        current_node = myhtml_tree_current_node(tree);
    }
}

void myhtml_tree_clear_stack_back_table_body_context(myhtml_tree_t* tree)
{
    myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
    
    while(!((current_node->tag_id == MyHTML_TAG_TBODY    ||
             current_node->tag_id == MyHTML_TAG_TFOOT    ||
             current_node->tag_id == MyHTML_TAG_THEAD    ||
             current_node->tag_id == MyHTML_TAG_TEMPLATE ||
             current_node->tag_id == MyHTML_TAG_HTML)    &&
            current_node->ns == MyHTML_NAMESPACE_HTML))
    {
        myhtml_tree_open_elements_pop(tree);
        current_node = myhtml_tree_current_node(tree);
    }
}

void myhtml_tree_clear_stack_back_table_row_context(myhtml_tree_t* tree)
{
    myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
    
    while(!((current_node->tag_id == MyHTML_TAG_TR       ||
            current_node->tag_id == MyHTML_TAG_TEMPLATE  ||
            current_node->tag_id == MyHTML_TAG_HTML)     &&
           current_node->ns == MyHTML_NAMESPACE_HTML))
    {
        myhtml_tree_open_elements_pop(tree);
        current_node = myhtml_tree_current_node(tree);
    }
}

void myhtml_tree_close_cell(myhtml_tree_t* tree, myhtml_tree_node_t* tr_or_th_node, myhtml_token_node_t* token)
{
    // step 1
    myhtml_tree_generate_implied_end_tags(tree, 0, MyHTML_NAMESPACE_UNDEF);
    
    // step 2
    myhtml_tree_node_t* current_node = myhtml_tree_current_node(tree);
    if(!((current_node->tag_id == MyHTML_TAG_TD ||
       current_node->tag_id == MyHTML_TAG_TH)  &&
       current_node->ns == MyHTML_NAMESPACE_HTML))
    {
        // parse error
        /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED STATUS:ELEMENT_OPEN_NOT_FOUND LEVEL:ERROR */
        /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:NULL NEED:NULL HAVE_TAG_ID:MyHTML_TAG__UNDEF HAVE_NS:MyHTML_NAMESPACE_UNDEF NEED_TAG_ID:MyHTML_TAG_TD NEED_NS:MyHTML_NAMESPACE_HTML */
        /* %EXTERNAL% VALIDATOR:RULES HAVE_NEED_ADD HAVE:NULL NEED:NULL HAVE_TAG_ID:MyHTML_TAG__UNDEF HAVE_NS:MyHTML_NAMESPACE_UNDEF NEED_TAG_ID:MyHTML_TAG_TH NEED_NS:MyHTML_NAMESPACE_HTML */
    }
    
    // step 3
    myhtml_tree_open_elements_pop_until(tree, tr_or_th_node->tag_id, tr_or_th_node->ns, false);
    
    // step 4
    myhtml_tree_active_formatting_up_to_last_marker(tree);
    
    // step 5
    tree->insert_mode = MyHTML_INSERTION_MODE_IN_ROW;
}

bool myhtml_tree_is_mathml_integration_point(myhtml_tree_t* tree, myhtml_tree_node_t* node)
{
    if(node->ns == MyHTML_NAMESPACE_MATHML &&
       (node->tag_id == MyHTML_TAG_MI ||
        node->tag_id == MyHTML_TAG_MO ||
        node->tag_id == MyHTML_TAG_MN ||
        node->tag_id == MyHTML_TAG_MS ||
        node->tag_id == MyHTML_TAG_MTEXT)
       )
        return true;
        
    return false;
}

bool myhtml_tree_is_html_integration_point(myhtml_tree_t* tree, myhtml_tree_node_t* node)
{
    if(node->ns == MyHTML_NAMESPACE_SVG &&
       (node->tag_id == MyHTML_TAG_FOREIGNOBJECT ||
        node->tag_id == MyHTML_TAG_DESC ||
        node->tag_id == MyHTML_TAG_TITLE)
       )
        return true;
    
    if(node->ns == MyHTML_NAMESPACE_MATHML &&
       node->tag_id == MyHTML_TAG_ANNOTATION_XML && node->token &&
       (node->token->type & MyHTML_TOKEN_TYPE_CLOSE) == 0)
    {
        myhtml_token_node_wait_for_done(tree->token, node->token);
        
        myhtml_token_attr_t* attr = myhtml_token_attr_match_case(tree->token, node->token,
                                                                 "encoding", 8, "text/html", 9);
        if(attr)
            return true;
        
        attr = myhtml_token_attr_match_case(tree->token, node->token,
                                                            "encoding", 8, "application/xhtml+xml", 21);
        if(attr)
            return true;
    }
    
    return false;
}

// temp tag name
mystatus_t myhtml_tree_temp_tag_name_init(myhtml_tree_temp_tag_name_t* temp_tag_name)
{
    temp_tag_name->size   = 1024;
    temp_tag_name->length = 0;
    temp_tag_name->data   = (char *)mycore_malloc(temp_tag_name->size * sizeof(char));
    
    if(temp_tag_name->data == NULL)
        return MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
    
    return MyHTML_STATUS_OK;
}

void myhtml_tree_temp_tag_name_clean(myhtml_tree_temp_tag_name_t* temp_tag_name)
{
    temp_tag_name->length = 0;
}

myhtml_tree_temp_tag_name_t * myhtml_tree_temp_tag_name_destroy(myhtml_tree_temp_tag_name_t* temp_tag_name, bool self_destroy)
{
    if(temp_tag_name == NULL)
        return NULL;
    
    if(temp_tag_name->data) {
        mycore_free(temp_tag_name->data);
        temp_tag_name->data = NULL;
    }
    
    if(self_destroy) {
        mycore_free(temp_tag_name);
        return NULL;
    }
    
    return temp_tag_name;
}

mystatus_t myhtml_tree_temp_tag_name_append_one(myhtml_tree_temp_tag_name_t* temp_tag_name, const char name)
{
    if(temp_tag_name->length >= temp_tag_name->size) {
        size_t nsize = temp_tag_name->size << 1;
        char *tmp = (char *)mycore_realloc(temp_tag_name->data, nsize * sizeof(char));
        
        if(tmp) {
            temp_tag_name->size = nsize;
            temp_tag_name->data = tmp;
        }
        else
            return MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
    }
    
    temp_tag_name->data[temp_tag_name->length] = name;
    
    temp_tag_name->length++;
    return MyHTML_STATUS_OK;
}

mystatus_t myhtml_tree_temp_tag_name_append(myhtml_tree_temp_tag_name_t* temp_tag_name, const char* name, size_t name_len)
{
    if(temp_tag_name->data == NULL || name_len == 0)
        return MyHTML_STATUS_OK;
    
    if((temp_tag_name->length + name_len) >= temp_tag_name->size) {
        size_t nsize = (temp_tag_name->size << 1) + name_len;
        char *tmp = (char *)mycore_realloc(temp_tag_name->data, nsize * sizeof(char));
        
        if(tmp) {
            temp_tag_name->size = nsize;
            temp_tag_name->data = tmp;
        }
        else
            return MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
    }
    
    memcpy(&temp_tag_name->data[temp_tag_name->length], name, name_len);
    
    temp_tag_name->length += name_len;
    return MyHTML_STATUS_OK;
}

void myhtml_tree_wait_for_last_done_token(myhtml_tree_t* tree, myhtml_token_node_t* token_for_wait)
{
#ifndef MyCORE_BUILD_WITHOUT_THREADS
    
    while(tree->token_last_done != token_for_wait) {mythread_nanosleep_sleep(tree->myhtml->thread_stream->timespec);}
    
#endif
}

/* special tonek list */
mystatus_t myhtml_tree_special_list_init(myhtml_tree_special_token_list_t* special)
{
    special->size   = 1024;
    special->length = 0;
    special->list   = (myhtml_tree_special_token_t *)mycore_malloc(special->size * sizeof(myhtml_tree_special_token_t));
    
    if(special->list == NULL)
        return MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
    
    return MyHTML_STATUS_OK;
}

void myhtml_tree_special_list_clean(myhtml_tree_temp_tag_name_t* temp_tag_name)
{
    temp_tag_name->length = 0;
}

myhtml_tree_special_token_list_t * myhtml_tree_special_list_destroy(myhtml_tree_special_token_list_t* special, bool self_destroy)
{
    if(special == NULL)
        return NULL;
    
    if(special->list) {
        mycore_free(special->list);
        special->list = NULL;
    }
    
    if(self_destroy) {
        mycore_free(special);
        return NULL;
    }
    
    return special;
}

mystatus_t myhtml_tree_special_list_append(myhtml_tree_special_token_list_t* special, myhtml_token_node_t *token, myhtml_namespace_t ns)
{
    if(special->length >= special->size) {
        size_t nsize = special->size << 1;
        myhtml_tree_special_token_t *tmp = (myhtml_tree_special_token_t *)mycore_realloc(special->list, nsize * sizeof(myhtml_tree_special_token_t));
        
        if(tmp) {
            special->size = nsize;
            special->list = tmp;
        }
        else
            return MyHTML_STATUS_ERROR_MEMORY_ALLOCATION;
    }
    
    special->list[special->length].ns = ns;
    special->list[special->length].token = token;
    
    special->length++;
    return MyHTML_STATUS_OK;
}

size_t myhtml_tree_special_list_length(myhtml_tree_special_token_list_t* special)
{
    return special->length;
}

size_t myhtml_tree_special_list_pop(myhtml_tree_special_token_list_t* special)
{
    if(special->length)
        special->length--;
    
    return special->length;
}


myhtml_tree_special_token_t * myhtml_tree_special_list_get_last(myhtml_tree_special_token_list_t* special)
{
    if(special->length == 0)
        return NULL;
    
    return &special->list[special->length];
}

/* incoming buffer */
mycore_incoming_buffer_t * myhtml_tree_incoming_buffer_first(myhtml_tree_t *tree)
{
    return tree->incoming_buf_first;
}

const char * myhtml_tree_incomming_buffer_make_data(myhtml_tree_t *tree, size_t begin, size_t length)
{
    mycore_incoming_buffer_t *buffer = mycore_incoming_buffer_find_by_position(tree->incoming_buf_first, begin);
    size_t relative_begin = begin - buffer->offset;
    
    if((relative_begin + length) <= buffer->size) {
        return &buffer->data[ relative_begin ];
    }
    
    if(tree->temp_tag_name.data == NULL)
        myhtml_tree_temp_tag_name_init(&tree->temp_tag_name);
    else
        myhtml_tree_temp_tag_name_clean(&tree->temp_tag_name);
    
    while(buffer) {
        if((relative_begin + length) > buffer->size)
        {
            size_t relative_end = (buffer->size - relative_begin);
            length -= relative_end;
            
            myhtml_tree_temp_tag_name_append(&tree->temp_tag_name, &buffer->data[relative_begin], relative_end);
            
            relative_begin = 0;
            buffer         = buffer->next;
        }
        else {
            myhtml_tree_temp_tag_name_append(&tree->temp_tag_name, &buffer->data[relative_begin], length);
            break;
        }
    }
    
    return tree->temp_tag_name.data;
}


