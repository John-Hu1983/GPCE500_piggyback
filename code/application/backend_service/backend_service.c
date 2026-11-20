#include "..\..\Include\spinmaster.h"

static SCH_Task backend_task = SCH_TASK_INIT;
static bool last_usb_online = false;
static u16 last_disp_mv = 0U;
static u8 last_disp_pct = 0U;
static bool low_bat_warned = false;

/*
 *@ brief    : Periodic background service task for battery & USB charging management.
 *@ type     : private
 *@ argu in  : None.
 *@ argu out : Battery/USB state sampled; publishes MSG_Charge (with BAT_Info* in data)
 *             and MSG_LowBattery (with u16* bat_mv in data) when state/voltage changes.
 *@ theme    : Message-bus-decoupled background power management service.
 */
static void _backend_service_task(void)
{
    const BAT_Info *info;
    u16 mv_diff;
    bool chg_changed = false;

    if (pimd_is_phy_busy())
    {
        return;
    }

    battery_sample_update();
    info = battery_get_info();

    /* 1. Detect USB charger plug/unplug transition */
    if (info->usb_online != last_usb_online)
    {
        last_usb_online = info->usb_online;
        chg_changed = true;
        if (info->usb_online)
        {
            low_bat_warned = false;
        }
    }

    /* 2. Publish MSG_Charge with BAT_Info payload in message->data when status/voltage changes */
    mv_diff = (info->bat_mv >= last_disp_mv) ? (u16)(info->bat_mv - last_disp_mv)
                                             : (u16)(last_disp_mv - info->bat_mv);
    if (chg_changed || mv_diff >= 15U || info->percent != last_disp_pct)
    {
        MSG_Message msg = {MSG_Charge, null, info};

        last_disp_mv = info->bat_mv;
        last_disp_pct = info->percent;
        msg_publish_message(&msg);
    }

    /* 3. Detect low battery threshold while discharging and publish MSG_LowBattery once */
    if (!info->usb_online)
    {
        if (!low_bat_warned && info->bat_mv <= BAT_LOW_WARN_MV)
        {
            MSG_Message msg = {MSG_LowBattery, null, &info->bat_mv};

            low_bat_warned = true;
            msg_publish_message(&msg);
        }
        else if (low_bat_warned && info->bat_mv >= (u16)(BAT_LOW_WARN_MV + 100U))
        {
            low_bat_warned = false;
        }
    }
}

/*
 *@ brief    : Initialize battery/USB detection hardware and start the background service task.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : ADC & battery monitor initialized; periodic backend_task registered in scheduler.
 *@ theme    : Call at startup before entering Desktop UI.
 */
void backend_open_service(void)
{
    const BAT_Info *info;

    battery_init_module();
    info = battery_get_info();

    last_usb_online = info->usb_online;
    last_disp_mv = info->bat_mv;
    last_disp_pct = info->percent;
    low_bat_warned = false;

    sch_start_task(&backend_task, BACKEND_SERVICE_SLICE, BACKEND_SERVICE_SLICE,
                   _backend_service_task);
}

/*
 *@ brief    : Stop the background service task.
 *@ type     : public
 *@ argu in  : None.
 *@ argu out : backend_task unlinked from scheduler.
 *@ theme    : Task lifecycle control.
 */
void backend_service_stop(void) { sch_stop_task(&backend_task); }
