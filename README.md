# Hedera Ledger App

Hedera™ Hashgraph BOLOS application for Ledger Nano S and Nano X.

## Development

### Prerequisite

-   Docker

### Compile

```
docker run -v $PWD:/app --platform linux/amd64 -it \
    ghcr.io/ledgerhq/ledger-app-builder/ledger-app-builder:latest \
    PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION=python make
```

### Check

```
docker run -v $PWD:/app --platform linux/amd64 -it \
    ghcr.io/ledgerhq/ledger-app-builder/ledger-app-builder:latest \
    PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION=python scan-build --use-cc=clang -analyze-headers -enable-checker security \
    -enable-checker unix -enable-checker valist -o scan-build \
    --status-bugs make default
```
