

#include <string.h>
#include "esp_log.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/ble_store.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include "poma_bleconnector.h"

bool is_nvs_initialized(void)
{
    nvs_handle_t my_handle;
    // Attempt to open a temporary/dummy namespace in read-only mode
    esp_err_t err = nvs_open("storage", NVS_READONLY, &my_handle);

    if (err == ESP_ERR_NVS_NOT_INITIALIZED)
    {
        return false;
    }

    // If it succeeded or failed for a different reason (like NOT_FOUND),
    // it means the driver itself IS initialized.
    if (err == ESP_OK)
    {
        nvs_close(my_handle);
    }

    return true;
}

/**GATT stuff */

static const char *TAG = "poma_gatt";

/* ---- UUIDs ----
 * Base UUID reused from the design proposal (Nordic UART Service base),
 * with the last two bytes of the 3rd group distinguishing service /
 * characteristics: ...0001 (service), ...0002 (RX), ...0003 (TX),
 * ...0004 (status). BLE_UUID128_INIT takes bytes in little-endian order,
 * i.e. reversed from the standard XXXXXXXX-XXXX-... string form.
 */
static const ble_uuid128_t poma_svc_uuid =
    BLE_UUID128_INIT(0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0,
                     0x93, 0xF3, 0xA3, 0xB5, 0x01, 0x00, 0x40, 0x6E);

static const ble_uuid128_t poma_rx_uuid =
    BLE_UUID128_INIT(0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0,
                     0x93, 0xF3, 0xA3, 0xB5, 0x02, 0x00, 0x40, 0x6E);

static const ble_uuid128_t poma_tx_uuid =
    BLE_UUID128_INIT(0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0,
                     0x93, 0xF3, 0xA3, 0xB5, 0x03, 0x00, 0x40, 0x6E);

// static const ble_uuid128_t echo_status_uuid =
//     BLE_UUID128_INIT(0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0,
//                       0x93, 0xF3, 0xA3, 0xB5, 0x04, 0x00, 0x40, 0x6E);

#define POMA_MAX_PAYLOAD 512 /* generous cap on a single RX write */

static uint16_t s_tx_val_handle;
static uint16_t s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
static bool s_subscribed = false;

typedef enum
{
    ECHO_STATE_IDLE = 0,
    ECHO_STATE_ECHOING = 1,
    ECHO_STATE_ERROR = 2,
} poma_state_t;
static volatile poma_state_t s_state = ECHO_STATE_IDLE;

static int gatt_svr_chr_access_rx(uint16_t conn_handle, uint16_t attr_handle,
                                  struct ble_gatt_access_ctxt *ctxt, void *arg);
static int gatt_svr_chr_access_status(uint16_t conn_handle, uint16_t attr_handle,
                                      struct ble_gatt_access_ctxt *ctxt, void *arg);
static int gatt_svr_chr_access_tx(uint16_t conn_handle, uint16_t attr_handle,
                                  struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &poma_svc_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]){
            {
                /* RX: central writes here. Write-without-response is the
                 * primary path for throughput; plain Write is also
                 * enabled for clients whose stack prefers it (design §4.2).
                 * To require pairing before writes are accepted
                 * (design §8), replace the flags line with the ENC
                 * variant below. */
                .uuid = &poma_rx_uuid.u,
                .access_cb = gatt_svr_chr_access_rx,
                .flags = BLE_GATT_CHR_F_WRITE /*| BLE_GATT_CHR_F_WRITE_NO_RSP,*/
                /* .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP
                 *        | BLE_GATT_CHR_F_WRITE_ENC, */
            },
            {
                /* TX: server notifies echoed data here. Notify-only,
                 * no read/write access callback needed. */
                .uuid = &poma_tx_uuid.u,
                .access_cb = gatt_svr_chr_access_tx,
                .val_handle = &s_tx_val_handle,
                .flags = BLE_GATT_CHR_F_NOTIFY,
            },
            //{
            /* Status: optional read-only diagnostic byte (design §4.2). */
            //    .uuid = &echo_status_uuid.u,
            //    .access_cb = gatt_svr_chr_access_status,
            //    .flags = BLE_GATT_CHR_F_READ,
            //},
            {
                0, /* terminator */
            },
        },
    },
    {
        0, /* terminator */
    },
};

/* ---- Connection / subscription state, driven from main.c's GAP handler ---- */

static void
poma_svc_set_conn_handle(uint16_t conn_handle, bool connected)
{
    s_conn_handle = connected ? conn_handle : BLE_HS_CONN_HANDLE_NONE;
    s_state = ECHO_STATE_IDLE;
}

static void
poma_svc_set_subscribed(bool subscribed)
{
    s_subscribed = subscribed;
    ESP_LOGI(TAG, "TX notifications %s", subscribed ? "enabled" : "disabled");
}

static uint16_t
poma_svc_get_mtu(void)
{
    if (s_conn_handle == BLE_HS_CONN_HANDLE_NONE)
    {
        return 0;
    }
    return ble_att_mtu(s_conn_handle);
}

/* ---- Fragmentation / notify logic (design §5) ----
 * Each outgoing packet is prefixed with a 1-byte header:
 *   bit 0 (LSB): more-fragments flag (1 = more fragments follow)
 *   bits 1-7:    sequence number, wraps mod 128
 * Notifications on a single BLE connection are delivered in order by
 * the controller, so this header is for the client's own bookkeeping
 * (detecting an unexpected gap) rather than reordering.
 */
static void poma_send_fragments(const uint8_t *data, size_t len)
{
    if (s_conn_handle == BLE_HS_CONN_HANDLE_NONE || !s_subscribed)
    {
        ESP_LOGW(TAG, "cannot notify: no connection or client not subscribed");
        s_state = ECHO_STATE_ERROR;
        return;
    }

    uint16_t mtu = poma_svc_get_mtu();
    /* usable payload = ATT_MTU - 3 (ATT op+handle header) - 1 (our seq header) */
    size_t chunk_size = (mtu > 4) ? (size_t)(mtu - 3 - 1) : 16;

    s_state = ECHO_STATE_ECHOING;

    size_t offset = 0;
    uint8_t seq = 0;
    uint8_t buf[512 + 1]; /* bounded by POMA_MAX_PAYLOAD + header byte */

    if (chunk_size > sizeof(buf) - 1)
    {
        chunk_size = sizeof(buf) - 1;
    }

    while (offset < len)
    {
        size_t remaining = len - offset;
        size_t n = remaining > chunk_size ? chunk_size : remaining;
        bool more = (offset + n) < len;

        buf[0] = (uint8_t)((seq << 1) | (more ? 1 : 0));
        memcpy(&buf[1], data + offset, n);

        struct os_mbuf *om = ble_hs_mbuf_from_flat(buf, n + 1);
        if (om == NULL)
        {
            ESP_LOGE(TAG, "mbuf alloc failed");
            s_state = ECHO_STATE_ERROR;
            return;
        }

        int rc = ble_gatts_notify_custom(s_conn_handle, s_tx_val_handle, om);
        if (rc != 0)
        {
            ESP_LOGE(TAG, "notify failed; rc=%d", rc);
            s_state = ECHO_STATE_ERROR;
            return;
        }

        offset += n;
        seq = (seq + 1) & 0x7F;
    }

    s_state = ECHO_STATE_IDLE;
}

/* ---- TX access handler ----
 * TX is notify-driven (see poma_send_fragments); this callback only
 * exists because NimBLE requires a non-NULL access_cb per characteristic
 * at registration time. A direct GATT Read against TX (rather than
 * subscribing) just returns an empty value. */
static int gatt_svr_chr_access_tx(uint16_t conn_handle, uint16_t attr_handle,
                                  struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    if (ctxt->op != BLE_GATT_ACCESS_OP_READ_CHR)
    {
        return BLE_ATT_ERR_UNLIKELY;
    }
    return 0; /* empty read */
}

/* ---- RX write handler ---- */
static int
gatt_svr_chr_access_rx(uint16_t conn_handle, uint16_t attr_handle,
                       struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    if (ctxt->op != BLE_GATT_ACCESS_OP_WRITE_CHR)
    {
        return BLE_ATT_ERR_UNLIKELY;
    }

    uint16_t om_len = OS_MBUF_PKTLEN(ctxt->om);
    if (om_len == 0)
    {
        return 0;
    }
    if (om_len > POMA_MAX_PAYLOAD)
    {
        /* design §9: reject oversize writes explicitly */
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    static uint8_t rx_buf[POMA_MAX_PAYLOAD];
    uint16_t out_len = 0;
    int rc = ble_hs_mbuf_to_flat(ctxt->om, rx_buf, sizeof(rx_buf), &out_len);
    if (rc != 0)
    {
        return BLE_ATT_ERR_UNLIKELY;
    }

    rx_buf[0] = '!'; // here is the echo and call to poma_core
    ESP_LOGI(TAG, "RX %u bytes, echoing back", (unsigned)out_len);
    poma_send_fragments(rx_buf, out_len);

    return 0;
}

/* ---- Status read handler (optional, design §4.2) ---- */
static int gatt_svr_chr_access_status(uint16_t conn_handle, uint16_t attr_handle,
                                      struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    if (ctxt->op != BLE_GATT_ACCESS_OP_READ_CHR)
    {
        return BLE_ATT_ERR_UNLIKELY;
    }
    uint8_t status = (uint8_t)s_state;
    int rc = os_mbuf_append(ctxt->om, &status, sizeof(status));
    return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

static int gatt_svr_init(void)
{
    int rc;

    ble_svc_gap_init();
    ble_svc_gatt_init();

    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0)
    {
        return rc;
    }

    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0)
    {
        return rc;
    }

    return 0;
}

/*BLE conf************************************************************************************ */

static void
poma_advertise(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    const char *name;
    int rc;

    memset(&fields, 0, sizeof(fields));

    /* General discoverable, BR/EDR not supported (design §7). */
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0)
    {
        ESP_LOGE(TAG, "error setting advertisement data; rc=%d", rc);
        return;
    }

    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, gap_event_handler, NULL);
    if (rc != 0)
    {
        ESP_LOGE(TAG, "error enabling advertisement; rc=%d", rc);
    }
}

static int gap_event_handler(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI(TAG, "connection %s; status=%d",
                 event->connect.status == 0 ? "established" : "failed",
                 event->connect.status);
        if (event->connect.status == 0)
        {
            poma_svc_set_conn_handle(event->connect.conn_handle, true);
        }
        else
        {
            /* Failed connection attempt; resume advertising (design §10 IDLE). */
            poma_advertise();
        }
        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "disconnect; reason=%d", event->disconnect.reason);
        poma_svc_set_conn_handle(BLE_HS_CONN_HANDLE_NONE, false);
        poma_svc_set_subscribed(false);
        poma_advertise();
        return 0;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        poma_advertise();
        return 0;

    case BLE_GAP_EVENT_SUBSCRIBE:
        ESP_LOGI(TAG, "subscribe event; attr_handle=%d cur_notify=%d",
                 event->subscribe.attr_handle, event->subscribe.cur_notify);
        poma_svc_set_subscribed(event->subscribe.cur_notify != 0);
        return 0;

    case BLE_GAP_EVENT_MTU:
        ESP_LOGI(TAG, "mtu update; conn_handle=%d mtu=%d",
                 event->mtu.conn_handle, event->mtu.value);
        return 0;

    case BLE_GAP_EVENT_REPEAT_PAIRING:
        /* Client re-paired without deleting the old bond on our side;
         * delete our stale bond and let the pairing procedure retry. */
        {
            struct ble_gap_conn_desc desc;
            ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
            ble_store_util_delete_peer(&desc.peer_id_addr);
        }
        return BLE_GAP_REPEAT_PAIRING_RETRY;

    default:
        return 0;
    }
}

static void on_sync(void)
{
    int rc;

    rc = ble_hs_util_ensure_addr(0);
    if (rc != 0)
    {
        ESP_LOGE(TAG, "no address available; rc=%d", rc);
        return;
    }

    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0)
    {
        ESP_LOGE(TAG, "error determining address type; rc=%d", rc);
        return;
    }

    poma_advertise();
}

static void on_reset(int reason)
{
    ESP_LOGE(TAG, "nimble host reset; reason=%d", reason);
}

static void host_task(void *param)
{
    /* Blocks and processes host events until nimble_port_stop() is called. */
    nimble_port_run();
    nimble_port_freertos_deinit();
}

/*BLE POMA********************************************************************************** */

//static int ref_sockfd = -1;

static void error(char *msg)
{
    ESP_LOGE(TAG, "%s: errno %d (%s)", msg, errno, strerror(errno));
    vTaskDelete(NULL);
}
/*
static int BLEwriter(const void *response, size_t rsp_size)
{

    assert(ref_sockfd >= 0);
    write(ref_sockfd, (char *)response, rsp_size);
    return 1;
}
*/

// processMessage(&BLEwriter, buffer, topicsHead);

void processBLEMessagesLoop(PoMA_BLE_SPEC *spec, Topic *topicsHead)
{

    ble_store_config_init();
    nimble_port_freertos_init(host_task);
}

PoMA_BLE_SPEC *createPoMABLEConnectSpec(PoMA_BLE_SPEC *spec, uint8_t portno, int multiuser)
{
    esp_err_t ret = ESP_OK;
    int rc;

    if (is_nvs_initialized() == false)
        ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ret = nimble_port_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "failed to init nimble port; rc=%d", ret);
        return spec;
    }

    ble_hs_cfg.reset_cb = on_reset;
    ble_hs_cfg.sync_cb = on_sync;
    ble_hs_cfg.gatts_register_cb = NULL;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    /* Security defaults: Just Works pairing, bonding enabled, no MITM
     * protection (design §8 "baseline" tier). Raise sm_io_cap / sm_mitm
     * if you need passkey-based MITM protection for a non-lab deployment,
     * and pair that with BLE_GATT_CHR_F_WRITE_ENC on the RX characteristic
     * in gatt_svr.c. */
    ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_NO_IO;
    ble_hs_cfg.sm_bonding = 1;
    ble_hs_cfg.sm_mitm = 0;
    ble_hs_cfg.sm_sc = 1;

    rc = gatt_svr_init();
    if (rc != 0)
    {
        ESP_LOGE(TAG, "gatt_svr_init failed; rc=%d", rc);
        return spec;
    }

    rc = ble_svc_gap_device_name_set("PoMA-Srv-ESP32");
    if (rc != 0)
    {
        ESP_LOGE(TAG, "failed to set device name; rc=%d", rc);
    }

    ble_store_config_init();
    // spec->own_addr_type = 0; //0 for default address. 1 for ramdom
    spec->processClientsLoop = processBLEMessagesLoop;
    return spec;
}
