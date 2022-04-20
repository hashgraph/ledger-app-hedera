# Hedera Ledger App Unit Tests

## Prerequisites

-   CMake

    ```sh
    brew install cmake
    ```

-   CMocka

    ```sh
    brew install cmocka
    ```

## Build

```sh
cmake -B build -H. && make -C build
```

## Run

```sh
CTEST_OUTPUT_ON_FAILURE=1 make -C build test
```
