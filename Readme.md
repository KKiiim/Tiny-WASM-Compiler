## Start With Spec Test

```sh
sudo apt-get update \
sudo apt-get install qemu-user gcc-aarch64-linux-gnu g++-aarch64-linux-gnu cmake make clang-tidy wabt nlohmann-json3-dev \

git submodule update --init --depth=1 \
cd third_party/googletest && mkdir -p build && cd build \
cmake .. -DCMAKE_CXX_COMPILER=/usr/bin/aarch64-linux-gnu-g++ -DCMAKE_C_COMPILER=/usr/bin/aarch64-linux-gnu-gcc \
make -j && sudo make install \

npm ci && npm run build && npm run spectest
```
