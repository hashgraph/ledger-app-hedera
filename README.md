### ledger-app-hedera

Ledger BOLOS app for Hedera Hashgraph

Don't forget to clone me with submodules!

`git clone --recurse-submodules git@github.com:hashgraph/ledger-app-hedera.git`

##### Building

- `make proto` makes the protobufs for Hedera

- `make` builds the app

- `make load` uses the ledgerblue python module to load the app onto the device

- `make delete` uses the module to delete the app

##### Notes

I have left explanatory comments throughout the codebase

Have a look at the awesome Sia app if you are new to BOLOS development and want a quick primer:

https://github.com/LedgerHQ/ledger-app-sia
