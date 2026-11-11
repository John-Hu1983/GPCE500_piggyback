#include "..\Include\spinmaster.h"

static MSG_Sub *msg_head = null;
static u16 msg_protect = 0;

/*
 *@ brief    : Validate a message type or subscription filter.
 *@ type     : private
 *@ argu in  : type: category; allow_any: permit the wildcard subscription filter.
 *@ argu out : None.
 *@ theme    : Reject unknown types and publishing the wildcard.
 */
static void _msg_check_type(MSG_Type type, bool allow_any)
{
    if (type < MSG_Any || type >= MSG_TypeCount || (!allow_any && type == MSG_Any))
    {
        bsp_halt_fault(BSP_Fault_MSG_Type);
    }
}

/*
 *@ brief    : Find a subscriber link or the tail link.
 *@ type     : private
 *@ argu in  : sub: caller-owned node.
 *@ argu out : Pointer to the matching link or null tail link.
 *@ theme    : Preserve registration order without reading a new node's fields.
 */
static MSG_Sub **_msg_find_sub(MSG_Sub *sub)
{
    MSG_Sub **link = &msg_head;

    if (sub == null)
    {
        bsp_halt_fault(BSP_Fault_MSG_Sub);
    }
    while (*link != null && *link != sub)
    {
        link = &(*link)->next;
    }
    return link;
}

/*
 *@ brief    : Release a detached subscriber's state.
 *@ type     : private
 *@ argu in  : sub: node no longer linked.
 *@ argu out : Links, filter and handler reset.
 *@ theme    : Do not leave pointers to publishers after unregistration.
 */
static void _msg_clear_sub(MSG_Sub *sub)
{
    sub->next = null;
    sub->type = MSG_Any;
    sub->source = null;
    sub->handler = null;
}

/*
 *@ brief    : Unlink canceled nodes after all nested publications finish.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Only active subscriptions remain.
 *@ theme    : Keep traversal links stable throughout every active publication.
 */
static void _msg_remove_subs(void)
{
    MSG_Sub **link = &msg_head;

    while (*link != null)
    {
        MSG_Sub *sub = *link;

        if (sub->handler != null)
        {
            link = &sub->next;
        }
        else
        {
            *link = sub->next;
            _msg_clear_sub(sub);
        }
    }
}

/*
 *@ brief    : Reset the shared foreground message bus.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : All registered nodes detached.
 *@ theme    : Initialize at startup; never reset an active publication.
 */
void msg_init_module(void)
{
    if (msg_protect != 0)
    {
        bsp_halt_fault(BSP_Fault_MSG_State);
    }
    while (msg_head != null)
    {
        msg_unsubscribe_node(msg_head);
    }
}

/*
 *@ brief    : Subscribe to a message type and optional source.
 *@ type     : public
 *@ argu in  : sub: node; type: category/MSG_Any; source: filter/null; handler: callback.
 *@ argu out : Node linked once or its existing filter and handler updated.
 *@ theme    : Append new nodes without fixed slots or per-type allocation.
 */
void msg_subscribe_type(MSG_Sub *sub, MSG_Type type, const void *source, MSG_Handler handler)
{
    MSG_Sub **link = _msg_find_sub(sub);

    _msg_check_type(type, true);
    if (handler == null)
    {
        bsp_halt_fault(BSP_Fault_MSG_Handler);
    }
    sub->type = type;
    sub->source = source;
    sub->handler = handler;
    if (*link == null)
    {
        sub->next = null;
        *link = sub;
    }
}

/*
 *@ brief    : Cancel a subscription.
 *@ type     : public
 *@ argu in  : sub: caller-owned node.
 *@ argu out : Delivery canceled; an unregistered node is unchanged.
 *@ theme    : Defer unlinking until the outermost publish returns.
 */
void msg_unsubscribe_node(MSG_Sub *sub)
{
    MSG_Sub **link = _msg_find_sub(sub);

    if (*link == null)
    {
        return;
    }
    sub->handler = null;
    if (msg_protect == 0)
    {
        *link = sub->next;
        _msg_clear_sub(sub);
    }
}

/*
 *@ brief    : Publish one typed message synchronously.
 *@ type     : public
 *@ argu in  : message: read-only header and borrowed typed data.
 *@ argu out : Matching subscribers called in registration order.
 *@ theme    : Snapshot the tail; nested delivery never invalidates outer links.
 */
void msg_publish_message(const MSG_Message *message)
{
    MSG_Sub *last = msg_head;
    MSG_Sub *sub;

    if (message == null)
    {
        bsp_halt_fault(BSP_Fault_MSG_Message);
    }
    _msg_check_type(message->type, false);
    if (msg_protect == 0xFFFFU)
    {
        bsp_halt_fault(BSP_Fault_MSG_State);
    }
    while (last != null && last->next != null)
    {
        last = last->next;
    }
    ++msg_protect;
    for (sub = msg_head; sub != null; sub = sub->next)
    {
        MSG_Handler handler = sub->handler;

        if (handler != null && (sub->type == MSG_Any || sub->type == message->type) &&
            (sub->source == null || sub->source == message->source))
        {
            handler(message);
        }
        if (sub == last)
        {
            break;
        }
    }
    --msg_protect;
    if (msg_protect == 0)
    {
        _msg_remove_subs();
    }
}
