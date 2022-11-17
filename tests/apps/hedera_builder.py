from typing import List, Generator, Dict
from contextlib import contextmanager

from proto import transaction_body_pb2 as TransactionBody_pb2
from proto import basic_types_pb2 as BasicTypes_pb2
from proto import crypto_create_pb2 as CryptoCreate_pb2
from proto import token_associate_pb2 as TokenAssociate_pb2
from proto import crypto_transfer_pb2 as Transfer_pb2
from proto import token_burn_pb2 as TokenBurn_pb2
from proto import token_mint_pb2 as TokenMint_pb2
from proto import wrappers_pb2 as Wrappers_pb2


def hedera_transaction(operator_shard_num: int,
                       operator_realm_num: int,
                       operator_account_num: int,
                       transaction_fee: int,
                       memo: str,
                       conf: Dict) -> bytes:

    operator = BasicTypes_pb2.HederaAccountID(
        shardNum = operator_shard_num,
        realmNum = operator_realm_num,
        accountNum = operator_account_num,
    )
    hedera_transaction_id = BasicTypes_pb2.HederaTransactionID(
        accountID = operator,
    )
    transaction = TransactionBody_pb2.HederaTransactionBody(
        transactionID = hedera_transaction_id,
        transactionFee = transaction_fee,
        memo = memo,
        **conf,
    )
    return transaction.SerializeToString()


def crypto_create_account_conf(initialBalance: int) -> Dict:
    crypto_create_account = CryptoCreate_pb2.HederaCryptoCreateTransactionBody(
        initialBalance = initialBalance,
    )
    return {"cryptoCreateAccount": crypto_create_account}


def crypto_transfer_token_conf(token_shardNum: int,
                               token_realmNum: int,
                               token_tokenNum: int,
                               sender_shardNum: int,
                               sender_realmNum: int,
                               sender_accountNum: int,
                               recipient_shardNum: int,
                               recipient_realmNum: int,
                               recipient_accountNum: int,
                               amount: int,
                               decimals: int) -> Dict:

    hedera_token_id = BasicTypes_pb2.HederaTokenID(
        shardNum = token_shardNum,
        realmNum = token_realmNum,
        tokenNum = token_tokenNum,
    )
    hedera_account_id_sender = BasicTypes_pb2.HederaAccountID(
        shardNum = sender_shardNum,
        realmNum = sender_realmNum,
        accountNum = sender_accountNum,
    )
    hedera_account_amount_sender = Transfer_pb2.HederaAccountAmount(
        accountID = hedera_account_id_sender,
        amount = 0,
    )
    hedera_account_id_recipient = BasicTypes_pb2.HederaAccountID(
        shardNum = recipient_shardNum,
        realmNum = recipient_realmNum,
        accountNum = recipient_accountNum,
    )
    hedera_account_amount_recipient = Transfer_pb2.HederaAccountAmount(
        accountID = hedera_account_id_recipient,
        amount = amount,
    )
    hedera_transfer_list = Transfer_pb2.HederaTransferList(
        accountAmounts = [],
    )
    decimalsUInt32 = Wrappers_pb2.UInt32Value(
        value = decimals,
    )
    hedera_token_transfer_list = Transfer_pb2.HederaTokenTransferList(
        token = hedera_token_id,
        transfers = [hedera_account_amount_recipient, hedera_account_amount_sender],
        expected_decimals = decimalsUInt32,
    )
    crypto_transfer = Transfer_pb2.HederaCryptoTransferTransactionBody (
        transfers = hedera_transfer_list,
        tokenTransfers = [hedera_token_transfer_list],
    )
    return {"cryptoTransfer": crypto_transfer}


def crypto_transfer_hbar_conf(sender_shardNum: int,
                              sender_realmNum: int,
                              sender_accountNum: int,
                              recipient_shardNum: int,
                              recipient_realmNum: int,
                              recipient_accountNum: int,
                              amount: int) -> Dict:

    hedera_account_id_sender = BasicTypes_pb2.HederaAccountID(
        shardNum = sender_shardNum,
        realmNum = sender_realmNum,
        accountNum = sender_accountNum,
    )
    hedera_account_amount_sender = Transfer_pb2.HederaAccountAmount(
        accountID = hedera_account_id_sender,
        amount = 0,
    )
    hedera_account_id_recipient = BasicTypes_pb2.HederaAccountID(
        shardNum = recipient_shardNum,
        realmNum = recipient_realmNum,
        accountNum = recipient_accountNum,
    )
    hedera_account_amount_recipient = Transfer_pb2.HederaAccountAmount(
        accountID = hedera_account_id_recipient,
        amount = amount,
    )
    hedera_transfer_list = Transfer_pb2.HederaTransferList(
        accountAmounts = [hedera_account_amount_recipient, hedera_account_amount_sender],
    )
    crypto_transfer = Transfer_pb2.HederaCryptoTransferTransactionBody (
        transfers = hedera_transfer_list,
        tokenTransfers = [],
    )
    return {"cryptoTransfer": crypto_transfer}


def crypto_transfer_verify(sender_shardNum: int,
                           sender_realmNum: int,
                           sender_accountNum: int) -> Dict:

    hedera_account_id_sender = BasicTypes_pb2.HederaAccountID(
        shardNum = sender_shardNum,
        realmNum = sender_realmNum,
        accountNum = sender_accountNum,
    )
    hedera_account_amount_sender = Transfer_pb2.HederaAccountAmount(
        accountID = hedera_account_id_sender,
        amount = 0,
    )
    hedera_transfer_list = Transfer_pb2.HederaTransferList(
        accountAmounts = [hedera_account_amount_sender],
    )
    crypto_transfer = Transfer_pb2.HederaCryptoTransferTransactionBody (
        transfers = hedera_transfer_list,
        tokenTransfers = [],
    )
    return {"cryptoTransfer": crypto_transfer}


def token_associate_conf(token_shardNum: int,
                         token_realmNum: int,
                         token_tokenNum: int,
                         sender_shardNum: int,
                         sender_realmNum: int,
                         sender_accountNum: int) -> Dict:

    hedera_account_id_sender = BasicTypes_pb2.HederaAccountID(
        shardNum = sender_shardNum,
        realmNum = sender_realmNum,
        accountNum = sender_accountNum,
    )
    hedera_token_id = BasicTypes_pb2.HederaTokenID(
        shardNum = token_shardNum,
        realmNum = token_realmNum,
        tokenNum = token_tokenNum,
    )
    token_associate = TokenAssociate_pb2.HederaTokenAssociateTransactionBody(
        account = hedera_account_id_sender,
        tokens = [hedera_token_id],
    )
    return {"tokenAssociate": token_associate}


def token_burn_conf(token_shardNum: int,
                    token_realmNum: int,
                    token_tokenNum: int,
                    amount: int,
                    decimals: int) -> Dict:

    hedera_token_id = BasicTypes_pb2.HederaTokenID(
        shardNum = token_shardNum,
        realmNum = token_realmNum,
        tokenNum = token_tokenNum,
    )
    decimalsUInt32 = Wrappers_pb2.UInt32Value(
        value = decimals,
    )
    token_burn = TokenBurn_pb2.HederaTokenBurnTransactionBody(
        token = hedera_token_id,
        amount = amount,
        expected_decimals = decimalsUInt32,
    )
    return {"tokenBurn": token_burn}


def token_mint_conf(token_shardNum: int,
                    token_realmNum: int,
                    token_tokenNum: int,
                    amount: int,
                    decimals: int) -> Dict:

    hedera_token_id = BasicTypes_pb2.HederaTokenID(
        shardNum = token_shardNum,
        realmNum = token_realmNum,
        tokenNum = token_tokenNum,
    )
    decimalsUInt32 = Wrappers_pb2.UInt32Value(
        value = decimals,
    )
    token_mint = TokenMint_pb2.HederaTokenMintTransactionBody(
        token = hedera_token_id,
        amount = amount,
        expected_decimals = decimalsUInt32,
    )
    return {"tokenMint": token_mint}
