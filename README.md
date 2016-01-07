# MyHTML C/C++ HTML 5 Parser. Using threads.

[![Build Status](https://travis-ci.org/lexborisov/myhtml.svg?branch=master)](https://travis-ci.org/lexborisov/myhtml)

MyHTML is a fast HTML Parser using Threads implemented as a pure C99 library with no outside dependencies.

## Now

```text
The current version is 0.1.0 - this is a beta version
Release will have major version number 1
```

## Features

- Asynchronous Parsing, Build Tree and Indexation
- Fully conformant with the [HTML5 specification]
- Two API - [high] and [low]-level
- Manipulation elements attributes: add, change, delete and other
- Support Single Mode parsing
- Support for fragment parsing
- No outside dependencies
- C99 support

## Build and Installation

```bash
make
```

If successful copy lib/* and include/* at the right place for you

```bash
cp lib/* /usr/local/lib
cp -r include/* /usr/local/include
```

## Build with your program

I advise to build using clang, but decided to show examples of gcc

**for example**

*build with shared library*
```bash
gcc -Wall -Werror -O2 -lmyhtml your_program.c -o your_program
```

*build with static library*
```bash
gcc -Wall -Werror -O2 your_program.c /path/to/static_libmyhtml.a -o your_program
```

## Future

Works are in full swing

- make test && make install
- Build in Windows OS
- external bindings (Perl, PHP, Python, Erlang, Node.js, C#, R, Ruby, Rust and other)
- ...
- css parsing && selectors (is ready, but I do not like it and will remodel)
- html render without js (in fact, he is ready, we do refactoring before publication, see previous)

## Dependencies

None

## Examples

See [examples] directory

**Simple example**

```c
#include <stdio.h>
#include <stdlib.h>

#include <myhtml/api.h>

int main(int argc, const char * argv[])
{
    char html[] = "<div><span>HTML</span></div>";
    
    // basic init
    myhtml_t* myhtml = myhtml_create();
    myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    
    // first tree init 
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
    
    // parse html
    myhtml_parse(tree, html, (sizeof(html) - 1));
    
    // release resources
    myhtml_tree_destroy(tree);
    myhtml_destroy(myhtml);
    
    return 0;
}
```

## AUTHOR

Alexander Borisov <lex.borisov@gmail.com>

## COPYRIGHT AND LICENSE

Copyright 2015-2016 Alexander Borisov

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and limitations under the License.


[HTML5 specification]: https://html.spec.whatwg.org/multipage/
[high]: https://github.com/lexborisov/myhtml/blob/master/include/myhtml/api.h
[low]: https://github.com/lexborisov/myhtml/tree/master/include/myhtml
[examples]: https://github.com/lexborisov/myhtml/tree/master/examples
