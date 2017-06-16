# MyHTML: Build and Installation

## Linux Packages
See all packages on https://packagecloud.io/Modest/myhtml

## GNU Make

In root directory of project (`/`):
```bash
make
make test
sudo make install
```

Flags that can be passed to make:
- `prefix`, default /usr/local
- `OS`, if not defined try to get from "uname -s"
- `PROJECT_OPTIMIZATION_LEVEL`, default -O2
- `MyCORE_BUILD_WITHOUT_THREADS`, YES or (NO or undefined), default undefined
- `MyCORE_BUILD_DEBUG`, YES or (NO or undefined), default undefined
- `MyCORE_WITH_PERF`, YES or (NO or undefined), default undefined, try build with timers (rdtsc or some), OS dependent, may not work on some systems, 
- `PROJECT_INSTALL_HEADER`, default "include"
- `PROJECT_INSTALL_LIBRARY`, default "lib"
- `PROJECT_INSTALL_WITHOUT_HEADERS`, YES or (NO or undefined), default undefined

*for example*:
```bash
make -j4 prefix=/usr MyCORE_BUILD_WITHOUT_THREADS=YES
sudo make install
```

Makefile rules:
- `all` —- build all components (libraries, examples, tests) (default)
- `library` -- build only static and shared library
- `shared` -- build only shared library
- `static` -- build only static library
- `clean` -- clean up current build directory
- `clone` -- copy all headers from source to include directories and modify local include (`#include "..."`) to global (`#include <...>`)
- `clean_api` -- remove all headers from include directory
- `create` -- create directories for binary, libraries, tests
- `modules` -- print modules name, description, dependencies
- `install` -- install libraries and headers on your system
- `uninstall` -- delete libraries and headers on your system
- `make-pc-file` -- create pkg-config file

*for example*:
```bash
make shared
```
