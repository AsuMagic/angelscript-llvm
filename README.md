# asllvm

A JIT compiler for AngelScript using LLVM.

## Current status

asllvm is currently highly experimental **and will *not* work with your current application** as feature support
is currently very limited.

The tests scattered around `.cpp` and `.as` files in the [`tests/`](tests/) directory should provide an idea of the
current support.

## Implementation status

In its current state, this is an **incomplete** TODO list:

### General

- [x] Generate, build, optimize and execute translated IR
- [ ] Complete bytecode support

### Planned platform support

Some specific features might be platform specific behavior and may need changes to port asllvm to other platforms, e.g.:
- `CommonDefinitions::iptr` is always 64-bit even on 32-bit platforms. This should be detected as needed and used in
    more places than it currently is.
- There may be unexpected C++ ABI differences between platforms, so generated external calls may be incorrect.

- [x] x86-64
  - [x] Linux
  - [ ] Windows (MinGW)

### Bytecode and feature support

This part is fairly incomplete, but provided to give a general idea:

- [x] Integral arithmetic\*
- [x] Floating-point arithmetic\*
- [x] Variables
  - [x] Globals
- [x] Branching (`if`, `for`, `while`, `switch` statements)
- [x] Script function calls
  - [x] Regular functions
  - [ ] Imported functions
  - [ ] Function pointers
- [x] Application interface
  - [ ] List constructors
  - [x] Object types
    - [x] Allocation
    - [x] Pass by value
    - [x] Pass by reference
    - [x] Return by value (from system function)
    - [x] Return by value (from script function)
    - [x] Virtual method calls
    - [ ] Composite calls
  - [ ] Reference-counted types
  - [x] Function calls
- [x] Script classes
  - [x] Constructing and destructing script classes
  - [x] Virtual script calls
    - [x] Devirtualization optimization\*\*\*
  - [x] Reference counted types\*\*
- [ ] VM execution status support
  - [ ] Exception on null pointer dereference
  - [ ] Exception on division by zero
  - [ ] Exception on overflow for some specific arithmetic ops
  - [ ] Support VM register introspection in system calls (for debugging, etc.)
  - [ ] VM suspend support

\*: `a ** b` (i.e. `pow`) is not implemented yet for any type.

\*\*: Reference counting through handles is implemented as stubs and don't actually perform any freeing for now.

\*\*\*: Implemented for trivial cases (method was originally declared as `final`).

**Due to the design of asllvm, it is not possible to partially JIT modules, or to skip JITting for specific modules.**
The JIT assumes that only supported features are used by your scripts and application interface, or it will yield an
assertion failure or broken codegen. Any script call from JIT'd code *must* point to a JIT'd function.

## Comparison, rationale and implementation status

This project was partly created for learning purposes.

An existing AngelScript JIT compiler from BlindMindStudios exists. I believe that asllvm may have several significant
differences with BMS's JIT compiler.

### Cross-platform support

BMS's JIT compiler was made with x86/x86-64 in mind, and cannot be ported to other architectures without a massive
rewrite.

asllvm is currently only implementing x86-64 Linux gcc support, but porting it to both x86 and x86-64 for Windows, Linux
and macOS should not be complicated.

x86-64 Linux and x86-64 Windows MinGW support is planned.

Porting asllvm to other architectures is non-trivial, but feasible, but may require some trickery and debugging around
the ABI support.

### Performance

BMS's JIT compiler has good compile times, potentially better than asllvm, but the generated code is likely to be
less efficient for several reasons:
- BMS's JIT compiler does not really perform any optimization, whereas LLVM does (and does so quite well).
- BMS's JIT compiler fallbacks to the VM in many occasions. asllvm aims to not ever require to use the AS VM as a
    fallback.
- There are some optimizations potentially planned for asllvm that may simplify the generated logic.

Note, however, that asllvm is a *much* larger dependency as it depends on LLVM. Think several tens of megabytes.

## Requirements

You will need:
- A C++17 compliant compiler.
- A recent LLVM version (tested on LLVM9).

Extra dependencies (`{fmt}` and `Catch2`) will be fetched automatically using `hunter`.

## Usage

There is currently no simple usage example, but you can check [`tests/common.cpp`](tests/common.cpp) to get an idea.

You have to register the JIT as usual. In its current state (this should be changed later on), you *need* to call
`JitInterface::BuildModules` before any script call.

## Recommendations

### Performance: using `final` whenever possible

asllvm is able to statically dispatch virtual function calls in a specific scenario: Either the original method
declaration either the class where the original method declaration resides must be `final`.

For example, devirtualization will work in the following scenario:

```angelscript
final class A
{
    void foo() { print("hi"); }
}

void main()
{
    A().foo();
}
```

but not in the following one:

```angelscript
class A
{
    void foo() { print("hi"); }
}

class B : A
{
    final void foo() { print("hello"); }
}

void main()
{
    B().foo();
}
```

(Note, devirtualization might be extended to support more cases in the future.)

This is an important optimization for classes dealing with a lot of classes. A virtual function lookup is somewhat
expensive and disallows some optimizations (such as inlining).
