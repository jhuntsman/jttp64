# Journey to Thunder Peak

An RPG adventure for the **Commodore 64**, developed in C using the **Oscar64** compiler.

## 🛠 Features
- Developed in modern C (Oscar64 toolchain).
- Modular architecture (graphics, sound, and game logic separated).
- Custom asset embedding for maps and sprites.

## 🚀 Getting Started

### Prerequisites
- **Oscar64 Compiler:** Ensure `/opt/oscar64/bin/oscar64` is in your path.
- **VICE Emulator:** Used for running and debugging the `.prg` output.
- **CMake:** Minimum version 3.20.

### Building
1. Open the project in CLion (or VSCode + VS64 extension).
2. Select the `JourneyToThunderPeak` target.
3. Build (Ctrl+F9). The output will be in `cmake-build-debug-oscar64/thunder-peak.prg`.

### Debugging
This project generates a `.dbj` debug symbol file. Use the VICE monitor (`Alt+H`) with `-moncommands` to see C labels in the assembly.

## 📜 License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.