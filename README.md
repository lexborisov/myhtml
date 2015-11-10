# myhtml
Multithreaded HTML parser

# WARNING

This is draft! Wait beta version (if it is).

# Build

For XCode:
1) Open myhtml.xcodeproj
2) Change load_html("/new/C/myhtml/test/test_full.html"); path in "main" function for yours html file
3) Run it

For unix:
1) Change load_html("/new/C/myhtml/test/test_full.html"); path in "main" function for yours html file
2) Run make (gnu make) in project folder (make, make clean)
3) Start program in bin folder

# printf and pthread_mutex_lock

Now it works with locks. Without asynchronous parsing. It is necessary for debugging. Turn off the lock pthread mutex in myhtml_tree_worker in myhtml_tree.c file.

# Dependencies

None

# AUTHOR

Alexander Borisov <lex.borisov@gmail.com>

# COPYRIGHT AND LICENSE

This software is copyright 2015 by Alexander Borisov

