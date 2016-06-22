/*
 Copyright (C) 2015-2016 Alexander Borisov
 
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

#include "myhtml/callback.h"


/* callback functions for tokens */
myhtml_callback_token_f myhtml_callback_before_token_done(myhtml_tree_t *tree)
{
    return tree->callback_before_token;
}

myhtml_callback_token_f myhtml_callback_after_token_done(myhtml_tree_t *tree)
{
    return tree->callback_after_token;
}

void myhtml_callback_before_token_done_set(myhtml_tree_t *tree, myhtml_callback_token_f func, void* ctx)
{
    tree->callback_before_token = func;
    tree->callback_before_token_ctx = ctx;
}

void myhtml_callback_after_token_done_set(myhtml_tree_t *tree, myhtml_callback_token_f func, void* ctx)
{
    tree->callback_after_token = func;
    tree->callback_after_token_ctx = ctx;
}


