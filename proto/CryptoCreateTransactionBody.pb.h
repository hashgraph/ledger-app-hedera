/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.5 */

#ifndef PB_PROTO_CRYPTOCREATETRANSACTIONBODY_PB_H_INCLUDED
#define PB_PROTO_CRYPTOCREATETRANSACTIONBODY_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
typedef struct _HederaCryptoCreateTransactionBody { 
    uint64_t initialBalance; 
} HederaCryptoCreateTransactionBody;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define HederaCryptoCreateTransactionBody_init_default {0}
#define HederaCryptoCreateTransactionBody_init_zero {0}

/* Field tags (for use in manual encoding/decoding) */
#define HederaCryptoCreateTransactionBody_initialBalance_tag 2

/* Struct field encoding specification for nanopb */
#define HederaCryptoCreateTransactionBody_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, UINT64,   initialBalance,    2)
#define HederaCryptoCreateTransactionBody_CALLBACK NULL
#define HederaCryptoCreateTransactionBody_DEFAULT NULL

extern const pb_msgdesc_t HederaCryptoCreateTransactionBody_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define HederaCryptoCreateTransactionBody_fields &HederaCryptoCreateTransactionBody_msg

/* Maximum encoded size of messages (where known) */
#define HederaCryptoCreateTransactionBody_size   11

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
