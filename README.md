# REQUIRMENTS

1. Works only for Windows 64-bit.
2. [clang](https://clang.llvm.org/). Add clang binaries in the PATH.
3. [CMake](https://cmake.org/). Add CMake binaries in the PATH.
4. [ninja](https://github.com/ninja-build/ninja).
5. Wake On Lan support. You can google what is the WOL and how setup it to your computer.

# BUILD & INSTALL

1. Run ```install.bat``` as Administrator. During installation, allow the script to run if asked.
2. Reboot your PC.
3. The application listens on UDP port 9 for a magic packet.
   
By default woloff installed in <ins>Program Files</ins> directory. Application works - check it in the Task Manager, in autorun.

# CHANGE INSTALLATION PATH

The default installation path is <ins>C:\Program Files\woloff</ins>. But if you want to change it, edit install.bat - ```set WOLOFF_INSTALL_PATH=[your_path]```.
