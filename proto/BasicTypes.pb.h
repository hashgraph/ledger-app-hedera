/* Automatically generated nanopb header */
/* Generated by nanopb-0.3.9.3 at Tue Oct 22 16:32:22 2019. */

#ifndef PB_BASICTYPES_PB_H_INCLUDED
#define PB_BASICTYPES_PB_H_INCLUDED
#include <pb.h>

/* @@protoc_insertion_point(includes) */
#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Struct definitions */
typedef struct _HederaAccountID {
    uint64_t shardNum;
    uint64_t realmNum;
    uint64_t accountNum;
/* @@protoc_insertion_point(struct:HederaAccountID) */
} HederaAccountID;

typedef struct _HederaDuration {
    uint64_t seconds;
/* @@protoc_insertion_point(struct:HederaDuration) */
} HederaDuration;

typedef PB_BYTES_ARRAY_T(32) HederaKey_ed25519_t;
typedef struct _HederaKey {
    pb_size_t which_key;
    union {
        HederaKey_ed25519_t ed25519;
    } key;
/* @@protoc_insertion_point(struct:HederaKey) */
} HederaKey;

typedef struct _HederaRealmID {
    uint64_t shardNum;
    uint64_t realmNum;
/* @@protoc_insertion_point(struct:HederaRealmID) */
} HederaRealmID;

typedef struct _HederaShardID {
    uint64_t shardNum;
/* @@protoc_insertion_point(struct:HederaShardID) */
} HederaShardID;

typedef struct _HederaTimestamp {
    uint64_t seconds;
    uint32_t nanos;
/* @@protoc_insertion_point(struct:HederaTimestamp) */
} HederaTimestamp;

typedef struct _HederaTransactionID {
    HederaTimestamp transactionValidStart;
    HederaAccountID accountID;
/* @@protoc_insertion_point(struct:HederaTransactionID) */
} HederaTransactionID;

/* Default values for struct fields */

/* Initializer values for message structs */
#define HederaKey_init_default                   {0, {{0, {0}}}}
#define HederaShardID_init_default               {0}
#define HederaRealmID_init_default               {0, 0}
#define HederaAccountID_init_default             {0, 0, 0}
#define HederaTimestamp_init_default             {0, 0}
#define HederaDuration_init_default              {0}
#define HederaTransactionID_init_default         {HederaTimestamp_init_default, HederaAccountID_init_default}
#define HederaKey_init_zero                      {0, {{0, {0}}}}
#define HederaShardID_init_zero                  {0}
#define HederaRealmID_init_zero                  {0, 0}
#define HederaAccountID_init_zero                {0, 0, 0}
#define HederaTimestamp_init_zero                {0, 0}
#define HederaDuration_init_zero                 {0}
#define HederaTransactionID_init_zero            {HederaTimestamp_init_zero, HederaAccountID_init_zero}

/* Field tags (for use in manual encoding/decoding) */
#define HederaAccountID_shardNum_tag             1
#define HederaAccountID_realmNum_tag             2
#define HederaAccountID_accountNum_tag           3
#define HederaDuration_seconds_tag               1
#define HederaKey_ed25519_tag                    2
#define HederaRealmID_shardNum_tag               1
#define HederaRealmID_realmNum_tag               2
#define HederaShardID_shardNum_tag               1
#define HederaTimestamp_seconds_tag              1
#define HederaTimestamp_nanos_tag                2
#define HederaTransactionID_transactionValidStart_tag 1
#define HederaTransactionID_accountID_tag        2

/* Struct field encoding specification for nanopb */
extern const pb_field_t HederaKey_fields[2];
extern const pb_field_t HederaShardID_fields[2];
extern const pb_field_t HederaRealmID_fields[3];
extern const pb_field_t HederaAccountID_fields[4];
extern const pb_field_t HederaTimestamp_fields[3];
extern const pb_field_t HederaDuration_fields[2];
extern const pb_field_t HederaTransactionID_fields[3];

/* Maximum encoded size of messages (where known) */
#define HederaKey_size                           34
#define HederaShardID_size                       11
#define HederaRealmID_size                       22
#define HederaAccountID_size                     33
#define HederaTimestamp_size                     17
#define HederaDuration_size                      11
#define HederaTransactionID_size                 54

/* Message IDs (where set with "msgid" option) */
#ifdef PB_MSGID

#define BASICTYPES_MESSAGES \


#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
/* @@protoc_insertion_point(eof) */

#endif