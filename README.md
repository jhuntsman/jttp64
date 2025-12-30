# JOURNEY TO THUNDER PEAK

[![Platform](https://img.shields.io/badge/Platform-C64-blue)](https://en.wikipedia.org/wiki/Commodore_64)
[![Compiler](https://img.shields.io/badge/Compiler-Oscar64-orange)](https://github.com/ichigyo/oscar64)
[![License](https://img.shields.io/badge/License-MIT-green)](LICENSE)

> *The earth trembles, the entrance looms. Will you survive the depths?*

**Journey to Thunder Peak** is an 8-bit RPG odyssey for the Commodore 64. Descend into the shadow of a legendary dungeon in this C-powered retro adventure. Ancient secrets lie buried beneath the stone, and only those with the resolve to enter the deep will uncover what the Peak has guarded for centuries.

---

## ⚔️ The Adventure
* **Classic Dungeon Crawling** – Traverse a jagged, subterranean world built with authentic 8-bit aesthetics.
* **Atmospheric Mystery** – A journey focused on the echoes and dangers of the deep.
* **Handcrafted Visuals** – Custom-designed PETSCII environments and sprites.

## ⚙️ Project Build
While the heart of the game is adventure, the engine is built for reliability:
* Built with the **Oscar64** C toolchain for the MOS 6510.
* Modular game logic for a smooth, deep-crawl experience.
* Designed for original hardware and VICE emulation.

## 🚀 Getting Started

### Prerequisites
* **Oscar64 Compiler** – The core C toolchain for the MOS 6510.
* **VICE Emulator** – The primary environment for testing and debugging.
* **VSCode & VS64 Extension** – The recommended IDE setup for integrated building and symbol mapping.
* **Ninja Build** – High-speed build system used for compiling the project.

### Building & Running
The project is optimized for a **Ninja**-based workflow within VSCode:

1. **Open Project** – Open the project folder in VSCode.
2. **VS64 Setup** – Ensure the VS64 extension is configured with your Oscar64 installation path.
3. **Build** – Press `Ctrl+Shift+B`. The extension will invoke Ninja to compile the source and link the game.
4. **Run** – The generated `JTTP.PRG` can be set to auto-launch directly into the VICE emulator for immediate testing.

### Manual Build (Alternative)
If you prefer building from the terminal:
```bash
cmake -G Ninja -B build
ninja -C build
```
### Debugging
This project generates a `.dbj` debug symbol file. Use the VICE monitor (`Alt+H`) with `-moncommands` to see C labels in the assembly.

## 📜 License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.