# myhtml
Multithreaded HTML parser

# WARNING

This is draft! Wait beta version (if it is).

# Build

For XCode:
* Open myhtml.xcodeproj
* Change load_html("/new/C/myhtml/test/test_full.html"); path in "main" function for yours html file
* Run it

For unix:
* Change load_html("/new/C/myhtml/test/test_full.html"); path in "main" function for yours html file
* Run make (gnu make) in project folder (make, make clean)
* Start program in bin folder

# printf and pthread_mutex_lock

Now it works with locks. Without asynchronous parsing. It is necessary for debugging. Turn off the lock pthread mutex in myhtml_tree_worker in myhtml_tree.c file

# Dependencies

None

# AUTHOR

Alexander Borisov <lex.borisov@gmail.com>

# COPYRIGHT AND LICENSE

This software is copyright 2015 by Alexander Borisov

