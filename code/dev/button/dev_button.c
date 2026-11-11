#include "..\Include\spinmaster.h"

static const BTN_Config btn_configs[BTN_Key_Count] = {
    {GPIOA, 0, BTN_INPUT_PATTERN, BTN_ACTIVE_LEVEL},
    {GPIOA, 1, BTN_INPUT_PATTERN, BTN_ACTIVE_LEVEL},
    {GPIOA, 2, BTN_INPUT_PATTERN, BTN_ACTIVE_LEVEL},
    {GPIOA, 3, BTN_INPUT_PATTERN, BTN_ACTIVE_LEVEL},
    {GPIOA, 4, BTN_INPUT_PATTERN, BTN_ACTIVE_LEVEL},
    {GPIOA, 5, BTN_INPUT_PATTERN, BTN_ACTIVE_LEVEL},
    {GPIOA, 6, BTN_INPUT_PATTERN, BTN_ACTIVE_LEVEL},
    {GPIOA, 7, BTN_INPUT_PATTERN, BTN_ACTIVE_LEVEL}};
static BTN_State btn_states[BTN_Key_Count];
static SWT_Timer _tim_fd;
static u16 btn_ctrl = 0;

/*
 *@ brief    : Scan all buttons periodically from the software timer.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Completed key gestures published to subscribers.
 *@ theme    : Software timer callback; uses BTN_ALLOW_DOUBLE mode.
 */
static void _btn_scan_timer(void) { btn_service_scan(BTN_ALLOW_DOUBLE); }

/*
 *@ brief    : Publish one button message to the shared message bus.
 *@ type     : private
 *@ argu in  : key: button ID; event: non-null gesture.
 *@ argu out : Subscribers matching MSG_Button receive a borrowed BTN_Message payload.
 *@ theme    : Broadcasts to all MSG_Button subscribers with key and event in data.
 */
static void _btn_publish_event(BTN_Id key, BTN_Event event)
{
    BTN_Message payload = {key, event};
    MSG_Message message = {MSG_Button, null, &payload};
    msg_publish_message(&message);
}

/*
 *@ brief    : Queue a click event without merging two completed clicks.
 *@ type     : private
 *@ argu in  : state: button runtime state.
 *@ argu out : One or two click messages queued in flags.
 *@ theme    : Preserve both clicks if double-click mode is disabled on second release.
 */
static void _btn_queue_click(BTN_State *state)
{
    if ((state->flags & BTN_MSG_CLICK) != 0)
    {
        state->flags |= BTN_MSG_CLICK2;
    }
    else
    {
        state->flags |= BTN_MSG_CLICK;
    }
}

/*
 *@ brief    : Queue a hold event and flush any waiting first click.
 *@ type     : private
 *@ argu in  : state: button runtime state.
 *@ argu out : Hold queued once; any pending first click precedes it.
 *@ theme    : Do not treat a held second press as a double click.
 */
static void _btn_queue_hold(BTN_State *state)
{
    if ((state->flags & BTN_FLAG_WAIT) != 0)
    {
        _btn_queue_click(state);
    }
    state->flags &= ~(BTN_FLAG_WAIT | BTN_FLAG_SECOND);
    state->flags |= BTN_FLAG_HELD | BTN_MSG_HOLD;
}

/*
 *@ brief    : Sample and update one button's state machine.
 *@ type     : private
 *@ argu in  : cfg: ROM pin config; state: RAM state; allow_double: gesture mode.
 *@ argu out : State updated and completed gestures queued in flags.
 *@ theme    : Use compact scan-count timers; sample all keys before publishing.
 */
static void _btn_sample_key(const BTN_Config *cfg, BTN_State *state, bool allow_double)
{
    bool raw = gpio_read_level(cfg->port, cfg->pin) == (cfg->active_level != 0);
    bool down = (state->flags & BTN_FLAG_DOWN) != 0;
    u16 db_cnt = state->flags & BTN_DEBOUNCE_MASK;

    state->flags &= ~BTN_MSG_MASK;

    /* 1. Check double-click waiting window before state transitions. */
    if ((state->flags & BTN_FLAG_WAIT) != 0 && (state->flags & BTN_FLAG_SECOND) == 0)
    {
        if (!allow_double)
        {
            state->flags &= ~BTN_FLAG_WAIT;
            _btn_queue_click(state);
        }
        else
        {
            state->timer++;
            if (state->timer >= BTN_DOUBLE_SCANS)
            {
                state->flags &= ~BTN_FLAG_WAIT;
                _btn_queue_click(state);
            }
        }
    }
    else if (!allow_double && (state->flags & BTN_FLAG_WAIT) != 0)
    {
        state->flags &= ~(BTN_FLAG_WAIT | BTN_FLAG_SECOND);
        _btn_queue_click(state);
    }

    /* 2. Debounce raw input level against stable down state. */
    if (raw != down)
    {
        db_cnt++;
        if (db_cnt >= BTN_DEBOUNCE_SCANS)
        {
            db_cnt = 0;
            if (raw)
            {
                state->flags |= BTN_FLAG_DOWN;
                state->flags &= ~BTN_FLAG_HELD;
                state->timer = 0;
                if (allow_double && (state->flags & BTN_FLAG_WAIT) != 0)
                {
                    state->flags |= BTN_FLAG_SECOND;
                }
            }
            else
            {
                state->flags &= ~BTN_FLAG_DOWN;
                if ((state->flags & BTN_FLAG_HELD) != 0)
                {
                    state->flags &= ~(BTN_FLAG_HELD | BTN_FLAG_SECOND);
                }
                else if (allow_double && (state->flags & BTN_FLAG_SECOND) != 0)
                {
                    state->flags &= ~(BTN_FLAG_WAIT | BTN_FLAG_SECOND);
                    state->flags |= BTN_MSG_DOUBLE;
                }
                else if (allow_double)
                {
                    state->flags |= BTN_FLAG_WAIT;
                    state->timer = 0;
                }
                else
                {
                    _btn_queue_click(state);
                }
            }
        }
    }
    else
    {
        db_cnt = 0;
    }

    state->flags = (state->flags & ~BTN_DEBOUNCE_MASK) | (db_cnt & BTN_DEBOUNCE_MASK);

    /* 3. Count hold duration while stably pressed. */
    if ((state->flags & BTN_FLAG_DOWN) != 0 && (state->flags & BTN_FLAG_HELD) == 0)
    {
        state->timer++;
        if (state->timer >= BTN_HOLD_SCANS)
        {
            _btn_queue_hold(state);
        }
    }
}

/*
 *@ brief    : Initialize all configured buttons and start the scan software timer.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : GPIO inputs configured, button states cleared, and _tim_fd started.
 *@ theme    : Hardware table stays in Flash; starts repeating scan timer every BTN_SCAN_TICKS.
 */
void btn_init_module(void)
{
    u16 i;

    if ((btn_ctrl & BTN_STATE_BUSY) != 0)
    {
        bsp_halt_fault(BSP_Fault_BTN_State);
    }
    for (i = 0; i < BTN_Key_Count; ++i)
    {
        if ((btn_configs[i].pattern != GPIO_In_Float && btn_configs[i].pattern != GPIO_In_PullLow &&
             btn_configs[i].pattern != GPIO_In_PullHigh) ||
            btn_configs[i].active_level > 1U)
        {
            bsp_halt_fault(BSP_Fault_BTN_Config);
        }
        gpio_init_pattern(btn_configs[i].port, btn_configs[i].pin, btn_configs[i].pattern);
        btn_states[i].flags = 0;
        btn_states[i].timer = 0;
    }
    btn_ctrl = BTN_STATE_READY;
    swt_start_timer(&_tim_fd, BTN_SCAN_TICKS, SWT_Repeat, _btn_scan_timer);
}

/*
 *@ brief    : Scan all buttons and publish completed gestures.
 *@ type     : public
 *@ argu in  : allow_double: true enables double-click detection; false outputs immediate clicks.
 *@ argu out : Publishes MSG_Button messages to any registered subscribers.
 *@ theme    : Foreground only; sample all buttons first, then publish in key order.
 */
void btn_service_scan(bool allow_double)
{
    u16 i;

    if ((btn_ctrl & BTN_STATE_READY) == 0 || (btn_ctrl & BTN_STATE_BUSY) != 0)
    {
        bsp_halt_fault(BSP_Fault_BTN_State);
    }
    btn_ctrl |= BTN_STATE_BUSY;

    for (i = 0; i < BTN_Key_Count; ++i)
    {
        _btn_sample_key(&btn_configs[i], &btn_states[i], allow_double);
    }
    for (i = 0; i < BTN_Key_Count; ++i)
    {
        u16 events = btn_states[i].flags & BTN_MSG_MASK;

        btn_states[i].flags &= ~BTN_MSG_MASK;
        if ((events & BTN_MSG_CLICK) != 0)
        {
            _btn_publish_event((BTN_Id)i, BTN_Click);
        }
        if ((events & BTN_MSG_CLICK2) != 0)
        {
            _btn_publish_event((BTN_Id)i, BTN_Click);
        }
        if ((events & BTN_MSG_DOUBLE) != 0)
        {
            _btn_publish_event((BTN_Id)i, BTN_Double);
        }
        if ((events & BTN_MSG_HOLD) != 0)
        {
            _btn_publish_event((BTN_Id)i, BTN_Hold);
        }
    }

    btn_ctrl &= ~BTN_STATE_BUSY;
}