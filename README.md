# CheriOS-microkernel

CheriOS is a minimal microkernel that demonstrates "clean-slate" CHERI memory protection and object capabilities.

### Note

CheriOS-microkernel is still in a very early state.

 * The code is not well documented
 * It lacks several security checks/actions (thus the model is not secure yet)
 * Expect races, non-recovery on error, ...

### Building CheriOS

You need a Cheri SDK ([LLVM] and [Clang]) to build CheriOS. Note that this IoT version uses less capability registers, and you need to hack LLVM to support this:

* C0: default data capability.
* C1, C2: Temporary registers, caller save.
* C3 - C8: capability argument registers. C6 - C7 are used as PCC, IDC.
* C17, C18: callee save.
* C25: exception handling in user space.
* KR1C: Trusted stack register.
* KCC, KDC, EPCC: kernel capability registers.

256-bit Cheri SDK works out of the box.
128-bit Cheri SDK is supported, but requires sealed capabilities to support at least CAP\_SIZE granularity.

The following snipset will build CheriOS for a 256-bit SDK targetting [cheri-qemu] (defaults).
```sh
$ git clone https://github.com/CTSRD-CHERI/cherios.git cherios
$ cd cherios
$ ./build.sh
```

### Running CheriOS

CheriOS can run on:

 * [cheri-qemu]
 * the CHERI fpga model
 * the CHERI l3 simulator

The target can be choosen by setting xxx in `CMakelists.txt`

The following snipset shows how to run CheriOS on [cheri-qemu]:
```sh
$ qemu-system-cheri -M malta -kernel cherios.elf -nographic -no-reboot -m 2048
```

### Code organisation

CheriOS code is organized as follow:

* __kernel__: kernel (the interesting part)
* __boot__: boot code
* __include__: generic includes used by several modules
* __ldscripts__: link scripts user by modules
* __libuser__: all modules are linked againt it. Provides several libc function as well as cherios-related functions
* __memmgt__: provides the system-wide mmap
* __namespace__: provides a directory of registered activations
* __uart__: module providing print services
* __qsort, AES, stringsearch, dijkstra, spam, sha, CRC32, bitcount__: MiBench benchmarks


   [cheri-qemu]: <https://github.com/CTSRD-CHERI/qemu>
   [LLVM]: <http://github.com/CTSRD-CHERI/llvm>
   [Clang]: <https://github.com/CTSRD-CHERI/clang>

