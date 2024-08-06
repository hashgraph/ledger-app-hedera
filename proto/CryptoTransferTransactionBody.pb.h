/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.5 */

#ifndef PB_PROTO_CRYPTOTRANSFERTRANSACTIONBODY_PB_H_INCLUDED
#define PB_PROTO_CRYPTOTRANSFERTRANSACTIONBODY_PB_H_INCLUDED
#include <pb.h>
#include "proto/BasicTypes.pb.h"

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
typedef struct _HederaAccountAmount { 
    bool has_accountID;
    HederaAccountID accountID; 
    int64_t amount; 
} HederaAccountAmount;

typedef struct _HederaTransferList { 
    pb_size_t accountAmounts_count;
    HederaAccountAmount accountAmounts[2]; 
} HederaTransferList;

typedef struct _HederaCryptoTransferTransactionBody { 
    bool has_transfers;
    HederaTransferList transfers; 
} HederaCryptoTransferTransactionBody;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define HederaAccountAmount_init_default         {false, HederaAccountID_init_default, 0}
#define HederaTransferList_init_default          {0, {HederaAccountAmount_init_default, HederaAccountAmount_init_default}}
#define HederaCryptoTransferTransactionBody_init_default {false, HederaTransferList_init_default}
#define HederaAccountAmount_init_zero            {false, HederaAccountID_init_zero, 0}
#define HederaTransferList_init_zero             {0, {HederaAccountAmount_init_zero, HederaAccountAmount_init_zero}}
#define HederaCryptoTransferTransactionBody_init_zero {false, HederaTransferList_init_zero}

/* Field tags (for use in manual encoding/decoding) */
#define HederaAccountAmount_accountID_tag        1
#define HederaAccountAmount_amount_tag           2
#define HederaTransferList_accountAmounts_tag    1
#define HederaCryptoTransferTransactionBody_transfers_tag 1

/* Struct field encoding specification for nanopb */
#define HederaAccountAmount_FIELDLIST(X, a) \
X(a, STATIC,   OPTIONAL, MESSAGE,  accountID,         1) \
X(a, STATIC,   SINGULAR, SINT64,   amount,            2)
#define HederaAccountAmount_CALLBACK NULL
#define HederaAccountAmount_DEFAULT NULL
#define HederaAccountAmount_accountID_MSGTYPE HederaAccountID

#define HederaTransferList_FIELDLIST(X, a) \
X(a, STATIC,   REPEATED, MESSAGE,  accountAmounts,    1)
#define HederaTransferList_CALLBACK NULL
#define HederaTransferList_DEFAULT NULL
#define HederaTransferList_accountAmounts_MSGTYPE HederaAccountAmount

#define HederaCryptoTransferTransactionBody_FIELDLIST(X, a) \
X(a, STATIC,   OPTIONAL, MESSAGE,  transfers,         1)
#define HederaCryptoTransferTransactionBody_CALLBACK NULL
#define HederaCryptoTransferTransactionBody_DEFAULT NULL
#define HederaCryptoTransferTransactionBody_transfers_MSGTYPE HederaTransferList

extern const pb_msgdesc_t HederaAccountAmount_msg;
extern const pb_msgdesc_t HederaTransferList_msg;
extern const pb_msgdesc_t HederaCryptoTransferTransactionBody_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define HederaAccountAmount_fields &HederaAccountAmount_msg
#define HederaTransferList_fields &HederaTransferList_msg
#define HederaCryptoTransferTransactionBody_fields &HederaCryptoTransferTransactionBody_msg

/* Maximum encoded size of messages (where known) */
#define HederaAccountAmount_size                 46
#define HederaCryptoTransferTransactionBody_size 98
#define HederaTransferList_size                  96

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif