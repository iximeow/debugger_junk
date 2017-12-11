You'll find four programs here, each part of what I was showing at a recent DC562 meet. Everything is linux oriented, though porting for another OS would be an interesting adventure.

**debugee** is a very simple program to poke at. It increments a counter, prints a number.

**debugger** uses `ptrace()` for debugger-like behavior - it attaches, single steps 100 times, and detaches. At each iteration, it reads the target process's registers and 16 bytes at RIP.
    A fun adventure would be to extend this to set/remove breakpoints, and to try using hardware breakpoints for code/memory access, rather than the more obvious software breakpoints.
    Additionally, finding how the counter is stored and resetting it with an external program would probably be fun!

**procexp** is more like a standalone `peek()`/`poke()` tool. It uses `/proc/<pid>/mem` to read/write single bytes of memory in a target process.  
Fun trick: use this to insert code for a clean exit into a running *debugee* instance, then patch the code to take that exit rather than loop.  
(Filippo's syscall table, https://filippo.io/linux-syscall-table/, might come in handy in stringing code together, along with the `syscall` instruction)

Because **procexp** doesn't use `ptrace()`, you can look at the memory of a process that *is* being `ptrace()`'d elsewhere, such as something being debugged by `gdb`. So if you set a breakpoint in `gdb`, you can `peek()` at that address to see when memory is patched to actually breakpoint!  
Hint: software breakpoints are only set when you continue for unbounded execution, single-stepping means the memory is probably the same as it was in the original file.

**antidebug** shows off a few tricks to detect and confuse an unsuspecting debugger. Both from a breakpoint-and-run and single stepping perspectives - run it and then run again with a debugger for different behavior.
