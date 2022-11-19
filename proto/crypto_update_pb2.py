# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: proto/crypto_update.proto
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
from proto import timestamp_pb2 as proto_dot_timestamp__pb2
from proto import wrappers_pb2 as proto_dot_wrappers__pb2


DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\x19proto/crypto_update.proto\x12\x06Hedera\x1a\x0cnanopb.proto\x1a\x17proto/basic_types.proto\x1a\x14proto/duration.proto\x1a\x15proto/timestamp.proto\x1a\x14proto/wrappers.proto\"\xe5\x06\n\x1b\x43ryptoUpdateTransactionBody\x12,\n\x11\x61\x63\x63ountIDToUpdate\x18\x02 \x01(\x0b\x32\x11.Hedera.AccountID\x12\x18\n\x03key\x18\x03 \x01(\x0b\x32\x0b.Hedera.Key\x12-\n\x0eproxyAccountID\x18\x04 \x01(\x0b\x32\x11.Hedera.AccountIDB\x02\x18\x01\x12\x19\n\rproxyFraction\x18\x05 \x01(\x05\x42\x02\x18\x01\x12!\n\x13sendRecordThreshold\x18\x06 \x01(\x04\x42\x02\x18\x01H\x00\x12=\n\x1asendRecordThresholdWrapper\x18\x0b \x01(\x0b\x32\x13.Hedera.UInt64ValueB\x02\x18\x01H\x00\x12$\n\x16receiveRecordThreshold\x18\x07 \x01(\x04\x42\x02\x18\x01H\x01\x12@\n\x1dreceiveRecordThresholdWrapper\x18\x0c \x01(\x0b\x32\x13.Hedera.UInt64ValueB\x02\x18\x01H\x01\x12)\n\x0f\x61utoRenewPeriod\x18\x08 \x01(\x0b\x32\x10.Hedera.Duration\x12)\n\x0e\x65xpirationTime\x18\t \x01(\x0b\x32\x11.Hedera.Timestamp\x12!\n\x13receiverSigRequired\x18\n \x01(\x08\x42\x02\x18\x01H\x02\x12\x37\n\x1areceiverSigRequiredWrapper\x18\r \x01(\x0b\x32\x11.Hedera.BoolValueH\x02\x12!\n\x04memo\x18\x0e \x01(\x0b\x32\x13.Hedera.StringValue\x12<\n max_automatic_token_associations\x18\x0f \x01(\x0b\x32\x12.Hedera.Int32Value\x12.\n\x11staked_account_id\x18\x10 \x01(\x0b\x32\x11.Hedera.AccountIDH\x03\x12\x18\n\x0estaked_node_id\x18\x11 \x01(\x03H\x03\x12)\n\x0e\x64\x65\x63line_reward\x18\x12 \x01(\x0b\x32\x11.Hedera.BoolValueB\x1a\n\x18sendRecordThresholdFieldB\x1d\n\x1breceiveRecordThresholdFieldB\x1a\n\x18receiverSigRequiredFieldB\x0b\n\tstaked_idb\x06proto3')

_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, globals())
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'proto.crypto_update_pb2', globals())
if _descriptor._USE_C_DESCRIPTORS == False:

  DESCRIPTOR._options = None
  _CRYPTOUPDATETRANSACTIONBODY.fields_by_name['proxyAccountID']._options = None
  _CRYPTOUPDATETRANSACTIONBODY.fields_by_name['proxyAccountID']._serialized_options = b'\030\001'
  _CRYPTOUPDATETRANSACTIONBODY.fields_by_name['proxyFraction']._options = None
  _CRYPTOUPDATETRANSACTIONBODY.fields_by_name['proxyFraction']._serialized_options = b'\030\001'
  _CRYPTOUPDATETRANSACTIONBODY.fields_by_name['sendRecordThreshold']._options = None
  _CRYPTOUPDATETRANSACTIONBODY.fields_by_name['sendRecordThreshold']._serialized_options = b'\030\001'
  _CRYPTOUPDATETRANSACTIONBODY.fields_by_name['sendRecordThresholdWrapper']._options = None
  _CRYPTOUPDATETRANSACTIONBODY.fields_by_name['sendRecordThresholdWrapper']._serialized_options = b'\030\001'
  _CRYPTOUPDATETRANSACTIONBODY.fields_by_name['receiveRecordThreshold']._options = None
  _CRYPTOUPDATETRANSACTIONBODY.fields_by_name['receiveRecordThreshold']._serialized_options = b'\030\001'
  _CRYPTOUPDATETRANSACTIONBODY.fields_by_name['receiveRecordThresholdWrapper']._options = None
  _CRYPTOUPDATETRANSACTIONBODY.fields_by_name['receiveRecordThresholdWrapper']._serialized_options = b'\030\001'
  _CRYPTOUPDATETRANSACTIONBODY.fields_by_name['receiverSigRequired']._options = None
  _CRYPTOUPDATETRANSACTIONBODY.fields_by_name['receiverSigRequired']._serialized_options = b'\030\001'
  _CRYPTOUPDATETRANSACTIONBODY._serialized_start=144
  _CRYPTOUPDATETRANSACTIONBODY._serialized_end=1013
# @@protoc_insertion_point(module_scope)