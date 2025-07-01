### mmap page for code section

### malloc page for operand stack

- X28 point the next empty address of runtime operand stack

### malloc page for function index to function abs address mapping

- array[funcIndex] with 8 bytes element -> its Address after mmap
