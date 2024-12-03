cmake.exe -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_C_COMPILER:FILEPATH=clang.exe -DCMAKE_CXX_COMPILER:FILEPATH=clang++.exe --no-warn-unused-cli -Bbuild -G Ninja
cmake.exe --build build --config Release --target all

rd /s /q build