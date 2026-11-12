#ifndef _DEV_BUTTON_H__
#define _DEV_BUTTON_H__

/*
 * Button hardware configuration macros.
 * Define each button's GPIO port, pin, input pattern, and active level here.
 * The hardware table itself lives in Flash/ROM (const), consuming 0 bytes of RAM.
 */
#define BTN_ACTIVE_LEVEL    1
#define BTN_INPUT_PATTERN   GPIO_In_PullLow

/*
 * Scan period in RTC ticks (at 2048Hz, 16 ticks = 7.8125ms).
 * Debounce, double-click window, and hold thresholds are measured in scan counts:
 * - BTN_DEBOUNCE_SCANS = 3   (~23.4ms)
 * - BTN_DOUBLE_SCANS   = 32  (250ms)
 * - BTN_HOLD_SCANS     = 64  (500ms)
 */
#define BTN_SCAN_TICKS      16U
#define BTN_DEBOUNCE_SCANS  3U
#define BTN_DOUBLE_SCANS    32U
#define BTN_HOLD_SCANS      64U
#define BTN_ALLOW_DOUBLE    false

#if BTN_ACTIVE_LEVEL != 0 && BTN_ACTIVE_LEVEL != 1
#error BTN_ACTIVE_LEVEL must be 0 or 1
#endif
#if BTN_SCAN_TICKS < 1 || BTN_SCAN_TICKS > 65535U || BTN_DEBOUNCE_SCANS < 1 ||                     \
    BTN_DOUBLE_SCANS < 1 || BTN_HOLD_SCANS < 1
#error Button scan and timing counts must be >= 1
#endif

// clang-format off
#define BTN_STATE_READY     0x0001U
#define BTN_STATE_BUSY      0x0002U

#define BTN_FLAG_DOWN       0x0100U
#define BTN_FLAG_HELD       0x0200U
#define BTN_FLAG_WAIT       0x0400U
#define BTN_FLAG_SECOND     0x0800U
#define BTN_MSG_CLICK       0x1000U
#define BTN_MSG_DOUBLE      0x2000U
#define BTN_MSG_HOLD        0x4000U
#define BTN_MSG_CLICK2      0x8000U
#define BTN_MSG_MASK        (BTN_MSG_CLICK | BTN_MSG_DOUBLE | BTN_MSG_HOLD | BTN_MSG_CLICK2)
#define BTN_DEBOUNCE_MASK   0x00FFU
// clang-format on

typedef struct
{
    u16 flags; /* Low 8 bits: debounce counter; high 8 bits: state & pending event flags. */
    u16 timer; /* Scan counter for hold duration or double-click window. */
} BTN_State;

/*
 * Button ID list and static IO mapping macro table.
 * Add or remove buttons here; each button is independent.
 */
typedef enum
{
    BTN_Key_0 = 0,
    BTN_Key_1,
    BTN_Key_2,
    BTN_Key_3,
    BTN_Key_4,
    BTN_Key_5,
    BTN_Key_6,
    BTN_Key_7,
    BTN_Key_Count
} BTN_Id;

typedef enum
{
    BTN_Null = 0,
    BTN_Click,
    BTN_Double,
    BTN_Hold
} BTN_Event;

typedef struct
{
    handle_t *port;
    uint8_t pin;
    GPIO_Pattern pattern;
    u16 active_level;
} BTN_Config;

typedef struct
{
    BTN_Id key;
    BTN_Event event;
} BTN_Message;

/*
 * Lightweight button driver without groups:
 * - IO mapping is stored in Flash (`const BTN_Config`), 0 RAM.
 * - Each button uses only 2 words (4 bytes) of RAM (`flags` + `timer`).
 * - Total RAM for 8 buttons is 16 words (32 bytes) + 1 control word.
 *
 * When a button gesture completes (`BTN_Click`, `BTN_Double`, `BTN_Hold`),
 * `btn_service_scan` publishes a message via `msg_publish_message`:
 *   message.type   = MSG_Button
 *   message.source = null
 *   message.data   = &BTN_Message { key, event }
 *
 * Example:
 *   static MSG_Sub sub = MSG_SUB_INIT;
 *   btn_init_module(); // Also starts internal main_button_timer
 *   msg_subscribe_type(&sub, MSG_Button, null, app_on_button);
 */
void btn_init_module(void);
void btn_service_scan(bool allow_double);

#endif /* _DEV_BUTTON_H__ */