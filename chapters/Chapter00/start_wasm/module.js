import { assert } from "node:console";
import { readFileSync } from "node:fs";

const wasmBuffer = readFileSync("./sum.wasm");
WebAssembly.instantiate(wasmBuffer).then((wasmModule) => {
  const { sum } = wasmModule.instance.exports;
  // ignore overflow
  const res = sum(5);
  assert(res == 15, "sum(5) = 15");
});

const wasmBuffer2 = readFileSync("./fibonacci.wasm");
WebAssembly.instantiate(wasmBuffer2).then((wasmModule) => {
  const { fibo } = wasmModule.instance.exports;
  const res = fibo(5);
  assert(res == 5, "fibo(5) = 5");
});
