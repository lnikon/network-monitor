cmake 3.17
ninja 1.10.1

conan install . -if=build -pr=default --build=missing
cmake .. -DCMAKE_BUILD_TYPE=Debug -G"Ninja" && ninja
