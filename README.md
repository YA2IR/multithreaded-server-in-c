# multithreaded-server-in-c

This is a simple multithreaded web server written in C. It serves static HTML pages from the "files" directory.

### Features:
 - Multithreaded, thread-safe. No leaks (as per Valgrind at least).
 - Has static routing and redirect functionalities.
 - Has a thread pool (reuses threads) instead of forking or creating threads on the fly.
 - Uses condition variables instead of busy waiting.

### Limitations:
 - It does not fully implement HTTP 1.1, but it can communicate with browsers.
 - It should not be used in production, as I haven't checked it for security beyond the basic requirements


### Getting it Running:
Compile and run the server:

```
gcc queue.c hash_table.c server.c -o server
./server
```
Then write in your browser:
```
localhost:4444
```
