#ifndef MESSAGE_H
#define MESSAGE_H

#include "user_config.h"

typedef enum
{
    MSG_Any = 0,
    MSG_Button,
    MSG_Charge,
    MSG_LowBattery,
    MSG_TypeCount
} MSG_Type;

typedef struct
{
    MSG_Type type;
    const void *source;
    const void *data;
} MSG_Message;

typedef void (*MSG_Handler)(const MSG_Message *message);

typedef struct MSG_Sub
{
    struct MSG_Sub *next;
    MSG_Type type;
    const void *source;
    MSG_Handler handler;
} MSG_Sub;

#define MSG_SUB_INIT {null, MSG_Any, null, null}

/*
 * One foreground message bus; caller-owned intrusive nodes, no heap or queue.
 * Native RAM: 7 words/subscriber and 3 global words (14N + 6 bytes).
 * Static nodes may use MSG_SUB_INIT. First registration initializes every field,
 * so a new node need not be zeroed. Keep nodes alive while registered and do not
 * change their fields yourself. Init detaches all subscriptions; run at startup,
 * not inside a handler. Unsubscribing an unregistered node is a no-op.
 *
 * Add new types before MSG_TypeCount; there is no per-type array to resize.
 * MSG_Any is a subscription filter only, never a published type. A null source
 * filter accepts all sources; a non-null filter matches that exact pointer.
 * Each node holds one type/source filter. Re-registering updates it in place,
 * without moving or duplicating the node. Use separate nodes for multiple filters.
 *
 * Publish is synchronous and FIFO among matching subscribers. No polling/service
 * loop is needed; no subscribers is valid. All APIs run in one foreground context,
 * never an ISR. ISR-originated events must be handed off to foreground first.
 * Message headers/data remain read-only until publish returns. Pointers are
 * borrowed during each callback only; copy the typed payload if retaining it.
 * Source identifies the publisher; data may be null for a payload-free signal.
 * Publishers and handlers agree on the payload type associated with each MSG_Type.
 *
 * Handlers may subscribe/unsubscribe and publish nested messages. New nodes do
 * not receive the current message, but can receive the next/nested message.
 * Canceling a node suppresses pending delivery immediately. Unlinking waits for
 * the outermost publish to return, so canceled nodes must stay alive until then.
 * Nested messages finish before the outer message continues. Avoid recursive
 * publication cycles; each nesting level uses the caller's stack, not a queue.
 * Invalid arguments halt through bsp_halt_fault with an MSG fault code.
 *
 * Example (inside initialization / foreground functions):
 *   static MSG_Sub sub = MSG_SUB_INIT;
 *   msg_subscribe_type(&sub, MSG_LowBattery, null, app_service_battery);
 *   u16 millivolts = 3200;
 *   MSG_Message message = { MSG_LowBattery, null, &millivolts };
 *   msg_publish_message(&message);
 *   // In app_service_battery: const u16 *voltage = message->data;
 *   // To cancel: msg_unsubscribe_node(&sub);
 */
void msg_init_module(void);
void msg_subscribe_type(MSG_Sub *sub, MSG_Type type, const void *source, MSG_Handler handler);
void msg_unsubscribe_node(MSG_Sub *sub);
void msg_publish_message(const MSG_Message *message);

#endif /* MESSAGE_H */
