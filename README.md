# MyHTML C/C++ HTML 5 Parser. Using threads.

[![Build Status](https://travis-ci.org/lexborisov/myhtml.svg?branch=master)](https://travis-ci.org/lexborisov/myhtml)

MyHTML is a fast HTML Parser using Threads implemented as a pure C99 library with no outside dependencies.

## Now

```text
The current version is 0.5.2 - this is a beta version
Release will have major version number 1
```

## Features

- Asynchronous Parsing, Build Tree and Indexation
- Fully conformant with the [HTML5 specification]
- Two API - [high] and [low]-level
- Manipulation of elements: add, change, delete and other
- Manipulation of elements attributes: add, change, delete and other
- Support 34 character encoding by specification [encoding.spec.whatwg.org]
- Support detecting character encodings
- Support Single Mode parsing
- Support Build without POSIX Threads
- Support for fragment parsing
- Support for [parsing by chunks]
- No outside dependencies
- C99 support
- Passes all tree construction tests from [html5lib-tests]

## Support encodings for InputStream

```text
X_USER_DEFINED, UTF_8, UTF_16LE, UTF_16BE, BIG5, EUC_KR, GB18030,
IBM866, ISO_8859_10, ISO_8859_13, ISO_8859_14, ISO_8859_15, ISO_8859_16, ISO_8859_2, ISO_8859_3,
ISO_8859_4, ISO_8859_5, ISO_8859_6, ISO_8859_7, ISO_8859_8, KOI8_R, KOI8_U, MACINTOSH,
WINDOWS_1250, WINDOWS_1251, WINDOWS_1252, WINDOWS_1253, WINDOWS_1254, WINDOWS_1255, WINDOWS_1256,
WINDOWS_1257, WINDOWS_1258, WINDOWS_874, X_MAC_CYRILLIC
```

## Support encodings for output

**Program working in UTF-8 and returns all in UTF-8**

## Detecting character encodings

Now it UTF-8, UTF-16LE, UTF16BE and russian windows-1251,  koi8-r, iso-8859-5, x-mac-cyrillic, ibm866

## The Story of Creation

[The Story of Creation]

## Benchmark

- [Article with charts]
- [Benchmark code]
- [Images and CSV]


## Build and Installation

**Make**

```bash
make
```

If successful copy lib/* and include/* at the right place for you

Flags that can be passed to make:
- `MyHTML_OPTIMIZATION_LEVEL=-O2` set compiler optimization level. Default: -O2
- `MyHTML_BUILD_WITHOUT_THREADS=YES` build without POSIX Threads. Default: NO

*for example*
```bash
make MyHTML_BUILD_WITHOUT_THREADS=NO
```


```bash
cp lib/* /usr/local/lib
cp -r include/* /usr/local/include
```

**CMake**

In myhtml/project directory:

```bash
cmake .
make
sudo make install
```

Flags that can be passed to CMake:
- `MyHTML_OPTIMIZATION_LEVEL=-O2` set compiler optimization level. Default: -O2
- `CMAKE_INSTALL_LIBDIR=lib` set path to install created library. Default: lib
- `MyHTML_BUILD_SHARED=ON` build shared library. Default: ON
- `MyHTML_BUILD_STATIC=ON` build static library. Default: ON
- `MyHTML_INSTALL_HEADER=OFF` install header files. Default OFF
- `MyHTML_BUILD_WITHOUT_THREADS=YES` build without POSIX Threads. Default: NO

*for example*
```bash
cmake . -DCMAKE_INSTALL_LIBDIR=lib64 -DMyHTML_INSTALL_HEADER=ON
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

## In other languages, external bindings

- [Perl 5] module


## Examples

See [examples] directory

**Simple example**

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    myhtml_parse(tree, MyHTML_ENCODING_UTF_8, html, strlen(html));
    
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
[parsing by chunks]: https://github.com/lexborisov/myhtml/blob/master/examples/chunks_high_level.c
[encoding.spec.whatwg.org]: https://encoding.spec.whatwg.org/
[html5lib-tests]: https://github.com/html5lib/html5lib-tests
[Perl 5]: https://metacpan.org/release/HTML-MyHTML
[The Story of Creation]: http://lexborisov.github.io/myhtml/
[Article with charts]: http://lexborisov.github.io/benchmark-html-persers/
[Benchmark code]: https://github.com/lexborisov/benchmark-html-persers/tree/master
[Images and CSV]: https://github.com/lexborisov/benchmark-html-persers/tree/master/Results
