from ragger.backend.interface import RAPDU, RaisePolicy

from .apps.hedera import HederaClient, ErrorType
from .apps.hedera_builder import crypto_create_account_conf
from .apps.hedera_builder import crypto_create_account_stake_account_conf
from .apps.hedera_builder import crypto_create_account_stake_node_conf
from .apps.hedera_builder import crypto_create_account_stake_toggle_rewards_conf
from .apps.hedera_builder import account_update_conf
from .apps.hedera_builder import crypto_transfer_token_conf
from .apps.hedera_builder import crypto_transfer_hbar_conf
from .apps.hedera_builder import crypto_transfer_verify
from .apps.hedera_builder import token_associate_conf
from .apps.hedera_builder import token_dissociate_conf
from .apps.hedera_builder import token_burn_conf
from .apps.hedera_builder import token_mint_conf


def test_hedera_get_public_key_ok(client, firmware):
    hedera = HederaClient(client)
    values = [
        (0, "78be747e6894ee5f965e3fb0e4c1628af2f9ae0d94dc01d9b9aab75484c3184b"),
        (11095, "644ef690d394e8140fa278273913425bc83c59067a392a9e7f703ead4973caf8"),
        (294967295, "02357008e57f96bb250f789c63eb3a241c1eae034d461468b76b8174a59bdc9b"),
        (
            2294967295,
            "2cbd40ac0a3e25a315aed7e211fd0056127075dfa4ba1717a7a047a2030b5efb",
        ),
    ]
    for (index, key) in values:
        from_public_key = hedera.get_public_key_non_confirm(index).data
        assert from_public_key.hex() == key
        with hedera.get_public_key_confirm(index):
            if firmware.device == "nanos":
                hedera.validate()
            else:
                hedera.validate_screen(1)

        from_public_key = hedera.get_async_response().data
        assert from_public_key.hex() == key


def test_hedera_get_public_key_refused(client, firmware):
    hedera = HederaClient(client)
    with hedera.get_public_key_confirm(0):
        client.raise_policy = RaisePolicy.RAISE_NOTHING
        if firmware.device == "nanos":
            hedera.refuse()
        else:
            hedera.validate_screen(1 + 1)

    rapdu = hedera.get_async_response()
    assert rapdu.status == ErrorType.EXCEPTION_USER_REJECTED


def test_hedera_crypto_create_account_ok(client, firmware):
    hedera = HederaClient(client)
    conf = crypto_create_account_conf(initialBalance=5)
    with hedera.send_sign_transaction(
        index=0,
        operator_shard_num=1,
        operator_realm_num=2,
        operator_account_num=3,
        transaction_fee=5,
        memo="this_is_the_memo",
        conf=conf,
    ):
        hedera.validate_screen(7)


def test_hedera_crypto_create_account_refused(client, firmware):
    hedera = HederaClient(client)
    conf = crypto_create_account_conf(initialBalance=5)
    with hedera.send_sign_transaction(
        index=0,
        operator_shard_num=1,
        operator_realm_num=2,
        operator_account_num=3,
        transaction_fee=5,
        memo="this_is_the_memo",
        conf=conf,
    ):
        client.raise_policy = RaisePolicy.RAISE_NOTHING
        hedera.validate_screen(8)

    rapdu = hedera.get_async_response()
    assert rapdu.status == ErrorType.EXCEPTION_USER_REJECTED


def test_hedera_crypto_create_account_stake_account(client, firmware):
    hedera = HederaClient(client)
    conf = crypto_create_account_stake_account_conf()
    with hedera.send_sign_transaction(
        index=0,
        operator_shard_num=1,
        operator_realm_num=2,
        operator_account_num=3,
        transaction_fee=5,
        memo="this_is_the_memo",
        conf=conf,
    ):
        hedera.validate_screen(7)


def test_hedera_crypto_create_account_stake_node(client, firmware):
    hedera = HederaClient(client)
    conf = crypto_create_account_stake_node_conf()
    with hedera.send_sign_transaction(
        index=0,
        operator_shard_num=1,
        operator_realm_num=2,
        operator_account_num=3,
        transaction_fee=5,
        memo="this_is_the_memo",
        conf=conf,
    ):
        hedera.validate_screen(7)


def test_hedera_crypto_create_account_stake_toggle_rewards(client, firmware):
    hedera = HederaClient(client)
    conf = crypto_create_account_stake_toggle_rewards_conf()
    with hedera.send_sign_transaction(
        index=0,
        operator_shard_num=1,
        operator_realm_num=2,
        operator_account_num=3,
        transaction_fee=5,
        memo="this_is_the_memo",
        conf=conf,
    ):
        hedera.validate_screen(7)


def test_hedera_account_update(client, firmware):
    hedera = HederaClient(client)
    conf = account_update_conf()
    with hedera.send_sign_transaction(
        index=0,
        operator_shard_num=1,
        operator_realm_num=2,
        operator_account_num=3,
        transaction_fee=5,
        memo="this_is_the_memo",
        conf=conf,
    ):
        hedera.validate_screen(7)


def test_hedera_transfer_token_ok(client, firmware):
    hedera = HederaClient(client)
    conf = crypto_transfer_token_conf(
        token_shardNum=15,
        token_realmNum=16,
        token_tokenNum=17,
        sender_shardNum=57,
        sender_realmNum=58,
        sender_accountNum=59,
        recipient_shardNum=100,
        recipient_realmNum=101,
        recipient_accountNum=102,
        amount=1234567890,
        decimals=9,
    )

    with hedera.send_sign_transaction(
        index=0,
        operator_shard_num=1,
        operator_realm_num=2,
        operator_account_num=3,
        transaction_fee=5,
        memo="this_is_the_memo",
        conf=conf,
    ):
        hedera.validate_screen(7)


def test_hedera_transfer_token_refused(client, firmware):
    hedera = HederaClient(client)
    conf = crypto_transfer_token_conf(
        token_shardNum=15,
        token_realmNum=16,
        token_tokenNum=17,
        sender_shardNum=57,
        sender_realmNum=58,
        sender_accountNum=59,
        recipient_shardNum=100,
        recipient_realmNum=101,
        recipient_accountNum=102,
        amount=1234567890,
        decimals=9,
    )

    with hedera.send_sign_transaction(
        index=0,
        operator_shard_num=1,
        operator_realm_num=2,
        operator_account_num=3,
        transaction_fee=5,
        memo="this_is_the_memo",
        conf=conf,
    ):
        client.raise_policy = RaisePolicy.RAISE_NOTHING
        hedera.validate_screen(7 + 1)

    rapdu = hedera.get_async_response()
    assert rapdu.status == ErrorType.EXCEPTION_USER_REJECTED


def test_hedera_transfer_hbar_ok(client, firmware):
    hedera = HederaClient(client)
    conf = crypto_transfer_hbar_conf(
        sender_shardNum=57,
        sender_realmNum=58,
        sender_accountNum=59,
        recipient_shardNum=100,
        recipient_realmNum=101,
        recipient_accountNum=102,
        amount=1234567890,
    )

    with hedera.send_sign_transaction(
        index=0,
        operator_shard_num=1,
        operator_realm_num=2,
        operator_account_num=3,
        transaction_fee=5,
        memo="this_is_the_memo",
        conf=conf,
    ):
        hedera.validate_screen(7)


def test_hedera_transfer_hbar_refused(client, firmware):
    hedera = HederaClient(client)
    conf = crypto_transfer_hbar_conf(
        sender_shardNum=57,
        sender_realmNum=58,
        sender_accountNum=59,
        recipient_shardNum=100,
        recipient_realmNum=101,
        recipient_accountNum=102,
        amount=1234567890,
    )

    with hedera.send_sign_transaction(
        index=0,
        operator_shard_num=1,
        operator_realm_num=2,
        operator_account_num=3,
        transaction_fee=5,
        memo="this_is_the_memo",
        conf=conf,
    ):
        client.raise_policy = RaisePolicy.RAISE_NOTHING
        hedera.validate_screen(7 + 1)

    rapdu = hedera.get_async_response()
    assert rapdu.status == ErrorType.EXCEPTION_USER_REJECTED


def test_hedera_token_associate_ok(client, firmware):
    hedera = HederaClient(client)
    conf = token_associate_conf(
        token_shardNum=57,
        token_realmNum=58,
        token_tokenNum=59,
        sender_shardNum=100,
        sender_realmNum=101,
        sender_accountNum=102,
    )

    with hedera.send_sign_transaction(
        index=0,
        operator_shard_num=1,
        operator_realm_num=2,
        operator_account_num=3,
        transaction_fee=5,
        memo="this_is_the_memo",
        conf=conf,
    ):
        hedera.validate_screen(6)


def test_hedera_token_associate_refused(client, firmware):
    hedera = HederaClient(client)
    conf = token_associate_conf(
        token_shardNum=57,
        token_realmNum=58,
        token_tokenNum=59,
        sender_shardNum=100,
        sender_realmNum=101,
        sender_accountNum=102,
    )

    with hedera.send_sign_transaction(
        index=0,
        operator_shard_num=1,
        operator_realm_num=2,
        operator_account_num=3,
        transaction_fee=5,
        memo="this_is_the_memo",
        conf=conf,
    ):
        client.raise_policy = RaisePolicy.RAISE_NOTHING
        hedera.validate_screen(7)

    rapdu = hedera.get_async_response()
    assert rapdu.status == ErrorType.EXCEPTION_USER_REJECTED


def test_hedera_token_dissociate_ok(client, firmware):
    hedera = HederaClient(client)
    conf = token_dissociate_conf(
        token_shardNum=57,
        token_realmNum=58,
        token_tokenNum=59,
        sender_shardNum=666,
        sender_realmNum=666,
        sender_accountNum=666,
    )
    with hedera.send_sign_transaction(
        index=0,
        operator_shard_num=1,
        operator_realm_num=2,
        operator_account_num=3,
        transaction_fee=5,
        memo="this_is_the_memo",
        conf=conf,
    ):
        hedera.validate_screen(6)


def test_hedera_token_dissoicate_refused(client, firmware):
    hedera = HederaClient(client)
    conf = token_dissociate_conf(
        token_shardNum=57,
        token_realmNum=58,
        token_tokenNum=59,
        sender_shardNum=666,
        sender_realmNum=666,
        sender_accountNum=666,
    )
    with hedera.send_sign_transaction(
        index=0,
        operator_shard_num=1,
        operator_realm_num=2,
        operator_account_num=3,
        transaction_fee=5,
        memo="this_is_the_memo",
        conf=conf,
    ):
        client.raise_policy = RaisePolicy.RAISE_NOTHING
        hedera.validate_screen(7)

    rapdu = hedera.get_async_response()
    assert rapdu.status == ErrorType.EXCEPTION_USER_REJECTED


def test_hedera_token_burn_ok(client, firmware):
    hedera = HederaClient(client)
    conf = token_burn_conf(
        token_shardNum=57, token_realmNum=58, token_tokenNum=59, amount=7745309389
    )

    with hedera.send_sign_transaction(
        index=0,
        operator_shard_num=1,
        operator_realm_num=2,
        operator_account_num=3,
        transaction_fee=5,
        memo="this_is_the_memo",
        conf=conf,
    ):
        hedera.validate_screen(6)


def test_hedera_token_burn_refused(client, firmware):
    hedera = HederaClient(client)
    conf = token_burn_conf(
        token_shardNum=57, token_realmNum=58, token_tokenNum=59, amount=7745309389
    )

    with hedera.send_sign_transaction(
        index=0,
        operator_shard_num=1,
        operator_realm_num=2,
        operator_account_num=3,
        transaction_fee=5,
        memo="this_is_the_memo",
        conf=conf,
    ):
        client.raise_policy = RaisePolicy.RAISE_NOTHING
        hedera.validate_screen(7)

    rapdu = hedera.get_async_response()
    assert rapdu.status == ErrorType.EXCEPTION_USER_REJECTED


def test_hedera_token_mint_ok(client, firmware):
    hedera = HederaClient(client)
    conf = token_mint_conf(
        token_shardNum=57, token_realmNum=58, token_tokenNum=59, amount=7745309389
    )

    with hedera.send_sign_transaction(
        index=0,
        operator_shard_num=1,
        operator_realm_num=2,
        operator_account_num=3,
        transaction_fee=5,
        memo="this_is_the_memo",
        conf=conf,
    ):
        hedera.validate_screen(6)


def test_hedera_token_mint_refused(client, firmware):
    hedera = HederaClient(client)
    conf = token_mint_conf(
        token_shardNum=57, token_realmNum=58, token_tokenNum=59, amount=7745309389
    )

    with hedera.send_sign_transaction(
        index=0,
        operator_shard_num=1,
        operator_realm_num=2,
        operator_account_num=3,
        transaction_fee=5,
        memo="this_is_the_memo",
        conf=conf,
    ):
        client.raise_policy = RaisePolicy.RAISE_NOTHING
        hedera.validate_screen(7)

    rapdu = hedera.get_async_response()
    assert rapdu.status == ErrorType.EXCEPTION_USER_REJECTED


def test_hedera_transfer_verify_ok(client, firmware):
    hedera = HederaClient(client)
    conf = crypto_transfer_verify(
        sender_shardNum=57, sender_realmNum=58, sender_accountNum=59
    )

    with hedera.send_sign_transaction(
        index=0,
        operator_shard_num=1,
        operator_realm_num=2,
        operator_account_num=3,
        transaction_fee=1,
        memo="this_is_the_memo",
        conf=conf,
    ):
        hedera.validate_screen(2)


def test_hedera_transfer_verify_refused(client, firmware):
    hedera = HederaClient(client)
    conf = crypto_transfer_verify(
        sender_shardNum=57, sender_realmNum=58, sender_accountNum=59
    )

    with hedera.send_sign_transaction(
        index=0,
        operator_shard_num=1,
        operator_realm_num=2,
        operator_account_num=3,
        transaction_fee=1,
        memo="this_is_the_memo",
        conf=conf,
    ):
        client.raise_policy = RaisePolicy.RAISE_NOTHING
        hedera.validate_screen(3)

    rapdu = hedera.get_async_response()
    assert rapdu.status == ErrorType.EXCEPTION_USER_REJECTED
