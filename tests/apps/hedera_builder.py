from typing import Dict

from proto import basic_types_pb2 as basics
from proto import crypto_create_pb2 as create
from proto import crypto_update_pb2 as update
from proto import crypto_transfer_pb2 as transfer
from proto import token_associate_pb2 as associate
from proto import token_dissociate_pb2 as dissociate
from proto import token_burn_pb2 as burn
from proto import token_mint_pb2 as mint
from proto import transaction_body_pb2 as tx_body
from proto import wrappers_pb2 as wrappers


def hedera_transaction(
    operator_shard_num: int,
    operator_realm_num: int,
    operator_account_num: int,
    transaction_fee: int,
    memo: str,
    conf: Dict,
) -> bytes:
    operator = basics.AccountID(
        shardNum=operator_shard_num,
        realmNum=operator_realm_num,
        accountNum=operator_account_num,
    )

    hedera_transaction_id = basics.TransactionID(
        accountID=operator,
    )

    transaction = tx_body.TransactionBody(
        transactionID=hedera_transaction_id,
        transactionFee=transaction_fee,
        memo=memo,
        **conf,
    )

    return transaction.SerializeToString()


def crypto_create_account_conf(initialBalance: int) -> Dict:
    crypto_create_account = create.CryptoCreateTransactionBody(
        initialBalance=initialBalance,
    )

    return {"cryptoCreateAccount": crypto_create_account}


def crypto_create_account_stake_account_conf() -> Dict:
    stake_target = basics.AccountID(shardNum=1, realmNum=2, accountNum=3)

    crypto_create_account = create.CryptoCreateTransactionBody(
        initialBalance=5, staked_account_id=stake_target
    )

    return {"cryptoCreateAccount": crypto_create_account}


def crypto_create_account_stake_node_conf() -> Dict:
    stake_target = 3

    crypto_create_account = create.CryptoCreateTransactionBody(
        initialBalance=5, staked_node_id=stake_target
    )

    return {"cryptoCreateAccount": crypto_create_account}


def crypto_create_account_stake_toggle_rewards_conf() -> Dict:
    stake_target = basics.AccountID(shardNum=6, realmNum=6, accountNum=6)

    crypto_create_account = create.CryptoCreateTransactionBody(
        initialBalance=5, staked_account_id=stake_target, decline_reward=True
    )

    return {"cryptoCreateAccount": crypto_create_account}


def account_update_conf() -> Dict:
    stake_target = basics.AccountID(shardNum=6, realmNum=6, accountNum=6)

    crypto_update_account = update.CryptoUpdateTransactionBody(
        staked_account_id=stake_target
    )

    return {"cryptoUpdateAccount": crypto_update_account}


def crypto_transfer_token_conf(
    token_shardNum: int,
    token_realmNum: int,
    token_tokenNum: int,
    sender_shardNum: int,
    sender_realmNum: int,
    sender_accountNum: int,
    recipient_shardNum: int,
    recipient_realmNum: int,
    recipient_accountNum: int,
    amount: int,
    decimals: int,
) -> Dict:

    hedera_token_id = basics.TokenID(
        shardNum=token_shardNum,
        realmNum=token_realmNum,
        tokenNum=token_tokenNum,
    )

    hedera_account_id_sender = basics.AccountID(
        shardNum=sender_shardNum,
        realmNum=sender_realmNum,
        accountNum=sender_accountNum,
    )

    hedera_account_amount_sender = basics.AccountAmount(
        accountID=hedera_account_id_sender,
        amount=0,
    )

    hedera_account_id_recipient = basics.AccountID(
        shardNum=recipient_shardNum,
        realmNum=recipient_realmNum,
        accountNum=recipient_accountNum,
    )

    hedera_account_amount_recipient = basics.AccountAmount(
        accountID=hedera_account_id_recipient,
        amount=amount,
    )

    hedera_transfer_list = basics.TransferList(
        accountAmounts=[],
    )

    decimalsUInt32 = wrappers.UInt32Value(
        value=decimals,
    )

    hedera_token_transfer_list = basics.TokenTransferList(
        token=hedera_token_id,
        transfers=[hedera_account_amount_recipient, hedera_account_amount_sender],
        expected_decimals=decimalsUInt32,
    )

    crypto_transfer = transfer.CryptoTransferTransactionBody(
        transfers=hedera_transfer_list,
        tokenTransfers=[hedera_token_transfer_list],
    )

    return {"cryptoTransfer": crypto_transfer}


def crypto_transfer_hbar_conf(
    sender_shardNum: int,
    sender_realmNum: int,
    sender_accountNum: int,
    recipient_shardNum: int,
    recipient_realmNum: int,
    recipient_accountNum: int,
    amount: int,
) -> Dict:

    hedera_account_id_sender = basics.AccountID(
        shardNum=sender_shardNum,
        realmNum=sender_realmNum,
        accountNum=sender_accountNum,
    )

    hedera_account_amount_sender = basics.AccountAmount(
        accountID=hedera_account_id_sender,
        amount=0,
    )

    hedera_account_id_recipient = basics.AccountID(
        shardNum=recipient_shardNum,
        realmNum=recipient_realmNum,
        accountNum=recipient_accountNum,
    )

    hedera_account_amount_recipient = basics.AccountAmount(
        accountID=hedera_account_id_recipient,
        amount=amount,
    )

    hedera_transfer_list = basics.TransferList(
        accountAmounts=[hedera_account_amount_recipient, hedera_account_amount_sender],
    )

    crypto_transfer = transfer.CryptoTransferTransactionBody(
        transfers=hedera_transfer_list,
        tokenTransfers=[],
    )

    return {"cryptoTransfer": crypto_transfer}


def crypto_transfer_verify(
    sender_shardNum: int, sender_realmNum: int, sender_accountNum: int
) -> Dict:

    hedera_account_id_sender = basics.AccountID(
        shardNum=sender_shardNum,
        realmNum=sender_realmNum,
        accountNum=sender_accountNum,
    )

    hedera_account_amount_sender = basics.AccountAmount(
        accountID=hedera_account_id_sender,
        amount=0,
    )

    hedera_transfer_list = basics.TransferList(
        accountAmounts=[hedera_account_amount_sender],
    )

    crypto_transfer = transfer.CryptoTransferTransactionBody(
        transfers=hedera_transfer_list,
        tokenTransfers=[],
    )

    return {"cryptoTransfer": crypto_transfer}


def token_associate_conf(
    token_shardNum: int,
    token_realmNum: int,
    token_tokenNum: int,
    sender_shardNum: int,
    sender_realmNum: int,
    sender_accountNum: int,
) -> Dict:

    hedera_account_id_sender = basics.AccountID(
        shardNum=sender_shardNum,
        realmNum=sender_realmNum,
        accountNum=sender_accountNum,
    )

    hedera_token_id = basics.TokenID(
        shardNum=token_shardNum,
        realmNum=token_realmNum,
        tokenNum=token_tokenNum,
    )

    token_associate = associate.TokenAssociateTransactionBody(
        account=hedera_account_id_sender,
        tokens=[hedera_token_id],
    )

    return {"tokenAssociate": token_associate}


def token_dissociate_conf(
    token_shardNum: int,
    token_realmNum: int,
    token_tokenNum: int,
    sender_shardNum: int,
    sender_realmNum: int,
    sender_accountNum: int,
) -> Dict:

    hedera_account_id_sender = basics.AccountID(
        shardNum=sender_shardNum,
        realmNum=sender_realmNum,
        accountNum=sender_accountNum,
    )

    hedera_token_id = basics.TokenID(
        shardNum=token_shardNum,
        realmNum=token_realmNum,
        tokenNum=token_tokenNum,
    )

    token_dissociate = dissociate.TokenDissociateTransactionBody(
        account=hedera_account_id_sender,
        tokens=[hedera_token_id],
    )

    return {"tokenDissociate": token_dissociate}


def token_burn_conf(
    token_shardNum: int,
    token_realmNum: int,
    token_tokenNum: int,
    amount: int,
) -> Dict:
    hedera_token_id = basics.TokenID(
        shardNum=token_shardNum,
        realmNum=token_realmNum,
        tokenNum=token_tokenNum,
    )

    token_burn = burn.TokenBurnTransactionBody(token=hedera_token_id, amount=amount)

    return {"tokenBurn": token_burn}


def token_mint_conf(
    token_shardNum: int,
    token_realmNum: int,
    token_tokenNum: int,
    amount: int,
) -> Dict:

    hedera_token_id = basics.TokenID(
        shardNum=token_shardNum,
        realmNum=token_realmNum,
        tokenNum=token_tokenNum,
    )

    token_mint = mint.TokenMintTransactionBody(
        token=hedera_token_id,
        amount=amount,
    )

    return {"tokenMint": token_mint}
