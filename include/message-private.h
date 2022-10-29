#ifndef _MESSAGE_T_H
#define _MESSAGE_T_H
#define BUFFER_MAX_SIZE 1024
#include "sdmessage.pb-c.h"

#define message_t _MessageT

#define M_OPCODE_BAD MESSAGE_T__OPCODE__OP_BAD
#define M_OPCODE_SIZE MESSAGE_T__OPCODE__OP_SIZE
#define M_OPCODE_HEIGHT MESSAGE_T__OPCODE__OP_HEIGHT
#define M_OPCODE_DEL MESSAGE_T__OPCODE__OP_DEL
#define M_OPCODE_GET MESSAGE_T__OPCODE__OP_GET
#define M_OPCODE_PUT MESSAGE_T__OPCODE__OP_PUT
#define M_OPCODE_GETKEYS MESSAGE_T__OPCODE__OP_GETKEYS
#define M_OPCODE_GETVALUES MESSAGE_T__OPCODE__OP_GETVALUES
#define M_OPCODE_ERROR MESSAGE_T__OPCODE__OP_ERROR

#define M_TYPE_BAD MESSAGE_T__C_TYPE__CT_BAD
#define M_TYPE_KEY MESSAGE_T__C_TYPE__CT_KEY
#define M_TYPE_VALUE MESSAGE_T__C_TYPE__CT_VALUE
#define M_TYPE_ENTRY MESSAGE_T__C_TYPE__CT_ENTRY
#define M_TYPE_KEYS MESSAGE_T__C_TYPE__CT_KEYS
#define M_TYPE_VALUES MESSAGE_T__C_TYPE__CT_VALUES
#define M_TYPE_RESULT MESSAGE_T__C_TYPE__CT_RESULT
#define M_TYPE_NONE MESSAGE_T__C_TYPE__CT_NONE

#endif
