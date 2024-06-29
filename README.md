# multithreaded-server-in-c

This is a *very* simple multithreaded server written in C, it serves a random file from the "files" directory.

### Features: 
 - Multithreaded, thread-safe. 
 - Has some expected efficiency requirements, such as using conditional variables instead of busy waiting.
 - Has a client.py script where it can be tested. 

### Limitations:
 - It is *not* a file server in the traditional sense (e.g. you don't specify a file path), it's just a way for me to expreience the socket interface.
 - It does not fully implement HTTP.


### Getting it Running:
Compile and run the server:

```
gcc queue.c server.c -o server
./server
```
Then run the client:
```
python client.py [NUM_REQUESTS]
```
