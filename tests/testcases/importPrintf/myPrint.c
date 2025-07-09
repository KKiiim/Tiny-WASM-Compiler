// NOLINTBEGIN
extern int myPrintf(const char *msg, ...);
void _start() {
  myPrintf("[test] -d is %d\n", 123);
  // myPrintf("[test] d is %i\n", -20);
  // myPrintf("[test] d is %u\n", 20);
  // myPrintf("[test] d is %lu\n", 88);
  // myPrintf("[test] %c %c %c %u\n", 'X', 'Y', 'Z', 99);
  // myPrintf("[test] %c %c %c %llu\n", 'J', 'K', 'L', (unsigned long long)88);
  // myPrintf("[test] %c %c %c %d\n", 'X', 'Y', 'Z', -88);
  // myPrintf("[test] %c %c %c %lld\n", 'X', 'Y', 'Z', (long long)-1024);
  // myPrintf("[test] str is %s\n", "abc");
  // myPrintf("[test] c is %c, s is %s\n", 'R', "abc");
  // int data2 = 0xffff;
  // int *pData = &data2;
  // myPrintf("[test] data is %p\n", pData);
  // myPrintf("[test] data is %p %c\n", pData, 'R');
}
// NOLINTEND

/**
./wasi-sdk-25.0-x86_64-linux/bin/clang -fdata-sections -ffunction-sections -mcpu=mvp -mmutable-globals
-Wl,--allow-undefined,--export-table,--gc-sections,--no-entry,--export=_start -lstdc++ -z stack-size=8192 -nostartfiles
-o ./main.wasm ./myPrint.c

runtime.callByName<void>("_start", "()", params[0], params[1], params[2], params[3], params[4], params[5], params[6]);
*/