## Start With Spec Test

```sh
sudo apt-get update &&
sudo apt-get install qemu-user gcc-aarch64-linux-gnu g++-aarch64-linux-gnu cmake make clang-tidy wabt nlohmann-json3-dev &&
npm ci && npm run build && npm run spectest
```
