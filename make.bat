cmake -S . -B build -G Ninja
ninja -C build

move build\Dx11SDL2.exe .\
move build\compile_commands.json .\
