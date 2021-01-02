/*******************************************************************************
 * Copyright (C) Maxim Integrated Products, Inc., All rights Reserved.
 *
 * This software is protected by copyright laws of the United States and
 * of foreign countries. This material may also be protected by patent laws
 * and technology transfer regulations of the United States and of foreign
 * countries. This software is furnished under a license agreement and/or a
 * nondisclosure agreement and may only be used or reproduced in accordance
 * with the terms of those agreements. Dissemination of this information to
 * any party or parties not specified in the license agreement and/or
 * nondisclosure agreement is expressly prohibited.
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of Maxim Integrated
 * Products, Inc. shall not be used except as stated in the Maxim Integrated
 * Products, Inc. Branding Policy.
 *
 * The mere transfer of this software does not imply any licenses
 * of trade secrets, proprietary technology, copyrights, patents,
 * trademarks, maskwork rights, or any other form of intellectual
 * property whatsoever. Maxim Integrated Products, Inc. retains all
 * ownership rights.
 *******************************************************************************
 */

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <string.h>

#include "max32666_ble_command.h"
#include "max32666_ble_queue.h"
#include "max32666_data.h"
#include "max32666_debug.h"
#include "maxrefdes178_definitions.h"


//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define S_MODULE_NAME   "comm"


//-----------------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------------
typedef enum {
    BLE_COMMAND_STATE_IDLE = 0,
    BLE_COMMAND_STATE_RX_STARTED,
    BLE_COMMAND_STATE_TX_STARTED,

    BLE_COMMAND_STATE_LAST
} ble_command_state_e;

typedef struct {
    uint8_t command;
    ble_command_state_e command_state;
    uint32_t received_payload_size;
    uint32_t total_payload_size;
    uint8_t total_payload_buffer[MAX32666_BLE_COMMAND_BUFFER_SIZE];
} ble_command_buffer_t;


//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
static ble_packet_container_t tmp_container;
static ble_command_buffer_t ble_command_buffer;


//-----------------------------------------------------------------------------
// Local function declarations
//-----------------------------------------------------------------------------
static int ble_command_form_single_packet(ble_command_e ble_command, uint32_t payload_size, uint8_t *payload);
static int ble_command_handle_rx(void);
static int ble_command_handle_tx(void);


//-----------------------------------------------------------------------------
// Function definitions
//-----------------------------------------------------------------------------
// TODO: handle small MTU, multiple packet cases
static int ble_command_form_single_packet(ble_command_e ble_command, uint32_t payload_size, uint8_t *payload)
{
    if (payload_size > sizeof(tmp_container.packet.command_packet.payload)) {
        PR_ERROR("invalid command %d payload size %d", ble_command, payload_size);
        return E_BAD_PARAM;
    }
    tmp_container.packet.command_packet.header.packet_info.type = BLE_PACKET_TYPE_COMMAND;
    tmp_container.packet.command_packet.header.packet_info.seq = device_status.ble_next_tx_seq;
    tmp_container.packet.command_packet.header.command = ble_command;
    tmp_container.packet.command_packet.header.total_payload_size = payload_size;
    memcpy(&tmp_container.packet.command_packet.payload, payload, payload_size);
    tmp_container.size = payload_size + sizeof(ble_command_packet_header_t);

    return E_SUCCESS;
}

int ble_command_send_audio_classification(void)
{
    int ret;

    if (ble_command_buffer.command_state != BLE_COMMAND_STATE_IDLE) {
        PR_ERROR("command state is not idle");
        return E_BAD_STATE;
    }

    ret = ble_command_form_single_packet(BLE_COMMAND_GET_MAX78000_AUDIO_CLASSIFICATION_RES,
            sizeof(device_status.classification_audio), (uint8_t *) &device_status.classification_audio);
    if (ret != E_SUCCESS) {
        PR_ERROR("form_single_packet_command failed %d", ret);
        return ret;
    }

    ret = ble_queue_enq_tx(&tmp_container);
    if (ret != E_SUCCESS) {
        PR_ERROR("ble_queue_enq_tx failed %d", ret);
        return ret;
    }

    return ret;
}

int ble_command_send_video_classification(void)
{
    int ret;

    if (ble_command_buffer.command_state != BLE_COMMAND_STATE_IDLE) {
        PR_ERROR("command state is not idle");
        return E_BAD_STATE;
    }

    ret = ble_command_form_single_packet(BLE_COMMAND_GET_MAX78000_VIDEO_CLASSIFICATION_RES,
            sizeof(device_status.classification_video), (uint8_t *) &device_status.classification_video);
    if (ret != E_SUCCESS) {
        PR_ERROR("form_single_packet_command failed %d", ret);
        return ret;
    }

    ret = ble_queue_enq_tx(&tmp_container);
    if (ret != E_SUCCESS) {
        PR_ERROR("ble_queue_enq_tx failed %d", ret);
        return ret;
    }

    return E_SUCCESS;
}

int ble_command_send_statistics(void)
{
    int ret;

    if (ble_command_buffer.command_state != BLE_COMMAND_STATE_IDLE) {
        PR_ERROR("command state is not idle");
        return E_BAD_STATE;
    }

    ret = ble_command_form_single_packet(BLE_COMMAND_GET_STATISTICS_RES,
            sizeof(device_status.statistics), (uint8_t *) &device_status.statistics);
    if (ret != E_SUCCESS) {
        PR_ERROR("form_single_packet_command failed %d", ret);
        return ret;
    }

    ret = ble_queue_enq_tx(&tmp_container);
    if (ret != E_SUCCESS) {
        PR_ERROR("ble_queue_enq_tx failed ret", ret);
        return ret;
    }

    return E_SUCCESS;
}

static int ble_command_execute_rx_command(void)
{
    PR_INFO("Execute command %d", ble_command_buffer.command);
    PR_INFO("Total payload: %d", ble_command_buffer.total_payload_size);
    for (int i = 0; i < ble_command_buffer.total_payload_size; i++) {
        PR("%02hhX ", ble_command_buffer.total_payload_buffer[i]);
    }
    PR("\n");

    return E_SUCCESS;
}

static int ble_command_handle_rx(void)
{
    uint8_t packet_payload_size;

    // BLE RX, check new packet on ble rx queue
    if (ble_queue_deq_rx(&tmp_container) != E_SUCCESS) {
        return E_NO_ERROR;
    }

    // Check sequence number of the packet
    if (device_status.ble_expected_rx_seq != tmp_container.packet.packet_info.seq) {
        PR_ERROR("Incorrect seq expected %d received %d", device_status.ble_expected_rx_seq,
                tmp_container.packet.packet_info.seq);
//        return E_BAD_STATE;
    }

    PR_INFO("BLE RX packet size %d", tmp_container.size);
    if (tmp_container.packet.packet_info.type == BLE_PACKET_TYPE_COMMAND) {
        packet_payload_size = tmp_container.size - sizeof(ble_command_packet_header_t);
        PR_INFO("Command %d", tmp_container.packet.command_packet.header.command);
        PR_INFO("Total payload size %d", tmp_container.packet.command_packet.header.total_payload_size);
        PR_INFO("Payload: %d", packet_payload_size);
        for (int i = 0; i < packet_payload_size; i++) {
            PR("%02hhX ", tmp_container.packet.command_packet.payload[i]);
        }
        PR("\n");

        // Check abort command
        if (tmp_container.packet.command_packet.header.command == BLE_COMMAND_ABORT_CMD) {
            PR_INFO("reset command state");
            ble_command_buffer.command_state = BLE_COMMAND_STATE_IDLE;
            return E_SUCCESS;
        }

        // Check state
        if (ble_command_buffer.command_state != BLE_COMMAND_STATE_IDLE) {
            PR_ERROR("command packet is not expected");
            return E_BAD_STATE;
        }

        // Check size
        if (packet_payload_size > tmp_container.packet.command_packet.header.total_payload_size) {
            PR_ERROR("packet payload size is larger than total payload size");
            return E_BAD_PARAM;
        }

        // TODO: handle big files
        // Check max size
        if (tmp_container.packet.command_packet.header.total_payload_size > MAX32666_BLE_COMMAND_BUFFER_SIZE) {
            PR_ERROR("total payload size is too big");
            return E_BAD_PARAM;
        }

        ble_command_buffer.command = tmp_container.packet.command_packet.header.command;
        ble_command_buffer.total_payload_size = tmp_container.packet.command_packet.header.total_payload_size;
        ble_command_buffer.received_payload_size = packet_payload_size;
        ble_command_buffer.command_state = BLE_COMMAND_STATE_RX_STARTED;
        memcpy(&ble_command_buffer.total_payload_buffer, &tmp_container.packet.command_packet.payload, packet_payload_size);

        // Check if single packet command
        if (tmp_container.packet.command_packet.header.total_payload_size <= packet_payload_size) {
            ble_command_execute_rx_command();
            ble_command_buffer.command_state = BLE_COMMAND_STATE_IDLE;
        }

        return E_SUCCESS;
    } else if (tmp_container.packet.packet_info.type == BLE_PACKET_TYPE_PAYLOAD) {
        packet_payload_size = tmp_container.size - sizeof(ble_payload_packet_header_t);
        PR_INFO("Payload %d: ", packet_payload_size);
        for (int i = 0; i < packet_payload_size; i++) {
            PR("%02hhX ", tmp_container.packet.payload_packet.payload[i]);
        }
        PR("\n");

        // Check state
        if (ble_command_buffer.command_state != BLE_COMMAND_STATE_RX_STARTED) {
            PR_ERROR("payload packet is not expected");
            return E_BAD_STATE;
        }

        if (ble_command_buffer.received_payload_size + packet_payload_size > MAX32666_BLE_COMMAND_BUFFER_SIZE) {
            PR_ERROR("payload overflow");
            return E_OVERFLOW;
        }

        memcpy((&ble_command_buffer.total_payload_buffer) + ble_command_buffer.received_payload_size,
                &tmp_container.packet.command_packet.payload, packet_payload_size);

        ble_command_buffer.received_payload_size += packet_payload_size;

        // Check if payload receive is completed
        if (tmp_container.packet.command_packet.header.total_payload_size <= ble_command_buffer.received_payload_size) {
            ble_command_execute_rx_command();
            ble_command_buffer.command_state = BLE_COMMAND_STATE_IDLE;
        }

        return E_SUCCESS;
    }

    return E_SUCCESS;
}

static int ble_command_handle_tx(void)
{
    // BLE TX, check new packet to enqueue ble tx queue
    if (ble_command_buffer.command_state != BLE_COMMAND_STATE_TX_STARTED) {
        return E_NO_ERROR;
    }
    // TODO

    return E_SUCCESS;
}

int ble_command_reset(void)
{
    ble_command_buffer.command_state = BLE_COMMAND_STATE_IDLE;

    // TODO close open files

    return E_NO_ERROR;
}

int ble_command_worker(void)
{
    ble_command_handle_rx();
    ble_command_handle_tx();

    return E_SUCCESS;
}

int ble_command_init(void)
{
    ble_command_reset();

    return E_NO_ERROR;
}