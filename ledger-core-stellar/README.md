# Ledger Core Stellar (XLM) library

This library provides everything you need to work with the [Stellar] currency and all of its
derived currencies.

<!-- vim-markdown-toc GFM -->

* [How to build](#how-to-build)
* [Using the Ledger Core Stellar library](#using-the-ledger-core-stellar-library)
  * [Native](#native)

<!-- vim-markdown-toc -->

## How to build

This project depends on [ledger-core]. In order to build it, you must have the public API of
[ledger-core] available. You don’t necessarily have to build the whole library; the sole interface
is needed.

From the root folder of this git repository, run the following commands:

  1. `./tools/lc api`: this will generate the needed interface, if not yet generated.
  2. `./tools/lc project api stellar`: this will generate the Stellar API.
  3. `cd ledger-core-stellar`.
  4. `mkdir build && cd build`: creates a build directory if none exists yet.
  5. `cmake -DBUILD_COINS=stellar -DCMAKE_BUILD_TYPE=Debug ..`: this will configure the stellar project in debug mode.
  6. `make -j8`:  builds with 8 threads.

Once everything is compiled, you can start playing with the C++ library or generating foreign
bindings.

## Using the Ledger Core Stellar library

### Native

If you want to use the native interface, you want to include the [inc/] folder in your project and
compile with the library generated at `build/src/libledger-core-stellar*`. You will find a dynamic
version, `libledger-core-stellar`, and a static version, `libledger-core-stellar-static`.

[Stellar]: https://www.stellar.org
[ledger-core]: ../ledger-core
[inc/]: ./inc
