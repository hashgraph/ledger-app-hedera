# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: proto/crypto_create.proto
"""Generated protocol buffer code."""
from google.protobuf.internal import builder as _builder
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import symbol_database as _symbol_database
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()


import nanopb_pb2 as nanopb__pb2
from proto import basic_types_pb2 as proto_dot_basic__types__pb2
from proto import duration_pb2 as proto_dot_duration__pb2


DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\x19proto/crypto_create.proto\x12\x06Hedera\x1a\x0cnanopb.proto\x1a\x17proto/basic_types.proto\x1a\x14proto/duration.proto\"\xa4\x04\n\x1b\x43ryptoCreateTransactionBody\x12\x18\n\x03key\x18\x01 \x01(\x0b\x32\x0b.Hedera.Key\x12\x16\n\x0einitialBalance\x18\x02 \x01(\x04\x12-\n\x0eproxyAccountID\x18\x03 \x01(\x0b\x32\x11.Hedera.AccountIDB\x02\x18\x01\x12\x1f\n\x13sendRecordThreshold\x18\x06 \x01(\x04\x42\x02\x18\x01\x12\"\n\x16receiveRecordThreshold\x18\x07 \x01(\x04\x42\x02\x18\x01\x12\x1b\n\x13receiverSigRequired\x18\x08 \x01(\x08\x12)\n\x0f\x61utoRenewPeriod\x18\t \x01(\x0b\x32\x10.Hedera.Duration\x12 \n\x07shardID\x18\n \x01(\x0b\x32\x0f.Hedera.ShardID\x12 \n\x07realmID\x18\x0b \x01(\x0b\x32\x0f.Hedera.RealmID\x12%\n\x10newRealmAdminKey\x18\x0c \x01(\x0b\x32\x0b.Hedera.Key\x12\x13\n\x04memo\x18\r \x01(\tB\x05\x92?\x02\x08\x64\x12(\n max_automatic_token_associations\x18\x0e \x01(\x05\x12.\n\x11staked_account_id\x18\x0f \x01(\x0b\x32\x11.Hedera.AccountIDH\x00\x12\x18\n\x0estaked_node_id\x18\x10 \x01(\x03H\x00\x12\x16\n\x0e\x64\x65\x63line_reward\x18\x11 \x01(\x08\x42\x0b\n\tstaked_idb\x06proto3')

_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, globals())
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'proto.crypto_create_pb2', globals())
if _descriptor._USE_C_DESCRIPTORS == False:

  DESCRIPTOR._options = None
  _CRYPTOCREATETRANSACTIONBODY.fields_by_name['proxyAccountID']._options = None
  _CRYPTOCREATETRANSACTIONBODY.fields_by_name['proxyAccountID']._serialized_options = b'\030\001'
  _CRYPTOCREATETRANSACTIONBODY.fields_by_name['sendRecordThreshold']._options = None
  _CRYPTOCREATETRANSACTIONBODY.fields_by_name['sendRecordThreshold']._serialized_options = b'\030\001'
  _CRYPTOCREATETRANSACTIONBODY.fields_by_name['receiveRecordThreshold']._options = None
  _CRYPTOCREATETRANSACTIONBODY.fields_by_name['receiveRecordThreshold']._serialized_options = b'\030\001'
  _CRYPTOCREATETRANSACTIONBODY.fields_by_name['memo']._options = None
  _CRYPTOCREATETRANSACTIONBODY.fields_by_name['memo']._serialized_options = b'\222?\002\010d'
  _CRYPTOCREATETRANSACTIONBODY._serialized_start=99
  _CRYPTOCREATETRANSACTIONBODY._serialized_end=647
# @@protoc_insertion_point(module_scope)