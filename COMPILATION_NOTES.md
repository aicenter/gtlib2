# Compiler flags explained:

## `-std=c++17`
 
 adhere to the revision of the C++ standard, published in 2017 https://gcc.gnu.org/projects/cxx-status.html#cxx17

## `-O0` vs `-O3`

https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html

Level of optimizations. 

`-O0` is no optimizations, `-O3` are the highest ones.

## `-m64`

Target 64bit architectures.

## `-ffast-math`

From https://stackoverflow.com/a/22135559/1233675

`-ffast-math` does a lot more than just break strict IEEE compliance.

First of all, of course, it does break strict IEEE compliance, allowing e.g. the reordering of instructions to something which is mathematically the same (ideally) but not exactly the same in floating point.

Second, it disables setting errno after single-instruction math functions, which means avoiding a write to a thread-local variable (this can make a 100% difference for those functions on some architectures).

Third, it makes the assumption that all math is finite, which means that no checks for NaN (or zero) are made in place where they would have detrimental effects. It is simply assumed that this isn't going to happen.

Fourth, it enables reciprocal approximations for division and reciprocal square root.

Further, it disables signed zero (code assumes signed zero does not exist, even if the target supports it) and rounding math, which enables among other things constant folding at compile-time.

Last, it generates code that assumes that no hardware interrupts can happen due to signalling/trapping math (that is, if these cannot be disabled on the target architecture and consequently do happen, they will not be handled).

## `-fsanitize=address` 

From https://clang.llvm.org/docs/AddressSanitizer.html

AddressSanitizer is a fast memory error detector. It consists of a compiler instrumentation module and a run-time library. The tool can detect the following types of bugs:

    Out-of-bounds accesses to heap, stack and globals
    Use-after-free
    Use-after-return (runtime flag ASAN_OPTIONS=detect_stack_use_after_return=1)
    Use-after-scope (clang flag -fsanitize-address-use-after-scope)
    Double-free, invalid free
    Memory leaks (experimental)

Typical slowdown introduced by AddressSanitizer is 2x.

## `-Wall`

Enable all the warnings about constructions that some users consider questionable, and that are easy to avoid (or modify to prevent the warning), even in conjunction with macros, in other words some set of warnings.

## `-Wextra`
 
Enable some more warnings, still not all.

## `-Werror`
 
Make all warnings into errors.

## `-Wpedantic` or just `-pedantic`

Issue all the warnings demanded by strict ISO C and ISO C++; reject all programs that use forbidden extensions, and some other programs that do not follow ISO C and ISO C++.

## `-pedantic-errors`
 
give an error whenever the base standard (see -Wpedantic) requires a diagnostic.

## `-Wshadow`

Give warning about shadowing variable names.

## `Weffc++`

From https://cpptruths.blogspot.com/2006/08/g-compiler-option-weffc.html

Warn about violations of the following style guidelines from Scott Meyers’ Effective C++ book:

* Item 11: Define a copy constructor and an assignment operator for classes with dynamically allocated memory.
* Item 12: Prefer initialization to assignment in constructors.
* Item 14: Make destructors virtual in base classes.
* Item 15: Have "operator=" return a reference to *this.
* Item 23: Don’t try to return a reference when you must return an object.

Also warn about violations of the following style guidelines from Scott Meyers’ More Effective C++ book:

* Item 6: Distinguish between prefix and postfix forms of increment and decrement operators.
* Item 7: Never overload "&&", "││", or ",".

When selecting this option, be aware that the standard library headers do not obey all of these guidelines.

## `Wstrict-aliasing`

https://gist.github.com/shafik/848ae25ee209f698763cffee272a58f8#catching-strict-aliasing-violations

Catch some strict aliasing cases in C++
