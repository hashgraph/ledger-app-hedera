from typing import List, Generator, Dict
from enum import IntEnum
from contextlib import contextmanager
from time import sleep

from ragger.backend.interface import BackendInterface, RAPDU

from ..utils import validate_displayed_message
from .hedera_builder import hedera_transaction


class INS(IntEnum):
    INS_GET_APP_CONFIGURATION   = 0x01
    INS_GET_PUBLIC_KEY          = 0x02
    INS_SIGN_TRANSACTION        = 0x04

CLA = 0xE0

P1_CONFIRM = 0x00
P1_NON_CONFIRM = 0x01

P2_EXTEND = 0x01
P2_MORE = 0x02


PUBLIC_KEY_LENGTH = 32

MAX_CHUNK_SIZE = 255


STATUS_OK = 0x9000

class ErrorType:
    EXCEPTION_USER_REJECTED = 0x6985


def to_zigzag(n):
    return n + n + (n < 0)


class HederaClient:
    client: BackendInterface

    def __init__(self, client):
        self._client = client

    def get_public_key_non_confirm(self, index: int) -> RAPDU:
        index_b = index.to_bytes(4, "little")
        return self._client.exchange(CLA, INS.INS_GET_PUBLIC_KEY, P1_NON_CONFIRM, 0, index_b)

    @contextmanager
    def get_public_key_confirm(self, index: int) -> Generator[None, None, None]:
        index_b = index.to_bytes(4, "little")
        with self._client.exchange_async(CLA, INS.INS_GET_PUBLIC_KEY, P1_CONFIRM, 0, index_b):
            sleep(0.5)
            yield

    def validate(self):
        self._client.right_click()

    def refuse(self):
        self._client.left_click()

    def validate_screen(self, right_clicks: int):
        validate_displayed_message(self._client, right_clicks)

    def get_async_response(self) -> RAPDU:
        return self._client.last_async_response

    @contextmanager
    def send_sign_transaction(self,
                              index: int,
                              operator_shard_num: int,
                              operator_realm_num: int,
                              operator_account_num: int,
                              transaction_fee: int,
                              memo: str,
                              conf: Dict) -> Generator[None, None, None]:

        transaction = hedera_transaction(operator_shard_num,
                                         operator_realm_num,
                                         operator_account_num,
                                         transaction_fee,
                                         memo,
                                         conf)

        payload = index.to_bytes(4, "little") + transaction

        with self._client.exchange_async(CLA, INS.INS_SIGN_TRANSACTION, P1_CONFIRM, 0, payload):
            sleep(0.5)
            yield
