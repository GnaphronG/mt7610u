/*
 *************************************************************************
 * Ralink Tech Inc.
 * 5F., No.36, Taiyuan St., Jhubei City,
 * Hsinchu County 302,
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2010, Ralink Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program; if not, write to the                         *
 * Free Software Foundation, Inc.,                                       *
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 *                                                                       *
 *************************************************************************/


#ifndef __MCU_AND_H__
#define __MCU_AND_H__

#include "mcu.h"

struct rtmp_adapter;

#define MCU_WAIT_ACK_CMD_THRESHOLD 0x0f
#define MCU_RX_CMD_THRESHOLD 0x0f

enum cmd_msg_state {
	ILLEGAL,
	TX_START,
	TX_KICKOUT_FAIL,
	TX_TIMEOUT_FAIL,
	TX_DONE,
	WAIT_CMD_OUT,
	WAIT_CMD_OUT_AND_ACK,
	WAIT_ACK,
	RX_START,
	RX_RECEIVE_FAIL,
	RX_DONE,
};

enum {
	MCU_INIT,
	MCU_TX_HALT,
	MCU_RX_HALT,
};

enum cmd_msg_error_type {
	ERROR_TX_KICKOUT_FAIL,
	ERROR_TX_TIMEOUT_FAIL,
	ERROR_RX_RECEIVE_FAIL,
};

struct mt7610u_mcu_ctrl {
	u8 cmd_seq;
	unsigned long flags;
	RTMP_NET_TASK_STRUCT cmd_msg_task;
	spinlock_t txq_lock;
	DL_LIST txq;
	spinlock_t rxq_lock;
	DL_LIST rxq;
	spinlock_t ackq_lock;
	DL_LIST ackq;
	spinlock_t kickq_lock;
	DL_LIST kickq;
	spinlock_t rx_doneq_lock;
	DL_LIST rx_doneq;
	unsigned long tx_kickout_fail_count;
	unsigned long tx_timeout_fail_count;
	unsigned long rx_receive_fail_count;
	bool power_on;
};


struct cmd_msg;
typedef void (*MSG_RSP_HANDLER)(struct cmd_msg *msg, char *payload, u16 payload_len);
typedef void (*MSG_EVENT_HANDLER)(struct rtmp_adapter *ad, char *payload, u16 payload_len);

struct cmd_msg_cb {
	struct cmd_msg *msg;
};

#define USB_END_PADDING 4
#define UPLOAD_PATCH_UNIT 2048
#define FW_INFO_SIZE 32
#define GET_SEMAPHORE_RETRY_MAX 600
#define UPLOAD_FW_UNIT 14592
#define UPLOAD_FW_TIMEOUT 1000
#define CMD_MSG_CB(pkt) ((struct cmd_msg_cb *)(GET_OS_PKT_CB(pkt)))
#define CMD_MSG_RETRANSMIT_TIMES 3
#define CMD_MSG_TIMEOUT 500

struct cmd_msg {
	DL_LIST list;
	u8 type;
	u8 seq;
	u16 timeout;
	u16 rsp_payload_len;
	bool need_wait;
	bool need_rsp;
	struct completion ack_done;
	char *rsp_payload;
	MSG_RSP_HANDLER rsp_handler;
	enum cmd_msg_state state;
	struct rtmp_adapter *priv;
	struct sk_buff *skb;
	struct urb *urb;
};

/*
 * Calibration ID
 */
enum mt7610u_mcu_calibration {
	R_CALIBRATION = 1,
	RXDCOC_CALIBRATION,
	LC_CALIBRATION,
	LOFT_CALIBRATION,
	TXIQ_CALIBRATION,
	BW_CALIBRATION,
	DPD_CALIBRATION,
	RXIQ_CALIBRATION,
	TXDCOC_CALIBRATION,
	RX_GROUP_DELAY_CALIBRATION,
	TX_GROUP_DELAY_CALIBRATION,
};

enum CALIBRATION_TYPE {
	FULL_CALIBRATION,
	PARTIAL_CALIBRATION,
};

enum SWITCH_CHANNEL_STAGE {
	NORMAL_OPERATING = 0x01,
	SCANNING = 0x02,
	TEMPERATURE_CHANGE = 0x04,
};

/*
 * Function set ID
 */
enum FUN_ID {
	Q_SELECT = 1,
	BW_SETTING = 2,
};

/*
 * Command response RX Ring selection
 */
enum RX_RING_ID {
	RX_RING0,
	RX_RING1,
};

enum BW_SETTING {
	BW20 = 1,
	BW40 = 2,
	BW10 = 4,
	BW80 = 8,
};

/*
 * Command type table
 */

enum mcu_cmd_type {
	CMD_FUN_SET_OP = 1,
	CMD_BURST_WRITE = 8,
	CMD_READ_MODIFY_WRITE,
	CMD_RANDOM_READ,
	CMD_BURST_READ,
	CMD_RANDOM_WRITE = 12,
	CMD_LED_MODE_OP = 16,
	CMD_POWER_SAVING_OP = 20,
	CMD_WOW_ENTRY,
	CMD_WOW_QUERY,
	CMD_CARRIER_DETECT_OP = 28,
	CMD_RADOR_DETECT_OP,
	CMD_SWITCH_CHANNEL_OP,
	CMD_CALIBRATION_OP,
	CMD_BEACON_OP,
	CMD_ANTENNA_OP
};

/*
 * Event type table
 */
enum EVENT_TYPE {
	CMD_DONE,
	CMD_ERROR,
	CMD_RETRY,
	EVENT_PWR_RSP,
	EVENT_WOW_RSP,
	EVENT_CARRIER_DETECT_RSP,
	EVENT_DFS_DETECT_RSP,
};

enum mcu_skb_state {
	ILLEAGAL = 0,
	MCU_CMD_START,
	MCU_CMD_DONE,
	MCU_RSP_START,
	MCU_RSP_DONE,
	MCU_RSP_CLEANUP,
	UNLINK_START,
};

struct mcu_skb_data {
	enum mcu_skb_state state;
};

int mt7610u_mcu_usb_loadfw(struct rtmp_adapter *ad);
void mt7610u_mcu_ctrl_init(struct rtmp_adapter *ad);
void mt7610u_mcu_ctrl_exit(struct rtmp_adapter *ad);
int mt7610u_mcu_burst_write(struct rtmp_adapter *ad, u32 offset, u32 *data, u32 cnt);
int mt7610u_mcu_random_read(struct rtmp_adapter *ad, struct rtmp_reg_pair *reg_pair, u32 num);
int mt7610u_mcu_rf_random_read(struct rtmp_adapter *ad, BANK_RF_REG_PAIR *reg_pair, u32 num);
int mt7610u_mcu_random_write(struct rtmp_adapter *ad, struct rtmp_reg_pair *reg_pair, u32 num);
int mt7610u_mcu_rf_random_write(struct rtmp_adapter *ad, BANK_RF_REG_PAIR *reg_pair, u32 num);
int mt7610u_mcu_fun_set(struct rtmp_adapter *ad, u32 fun_id, u32 param);
int mt7610u_mcu_calibration(struct rtmp_adapter *ad, enum mt7610u_mcu_calibration cal_id, u32 param);
int mt7610u_mcu_led_op(struct rtmp_adapter *ad, u32 led_idx, u32 link_status);
int usb_rx_cmd_msgs_receive(struct rtmp_adapter *ad);

#define MT_INBAND_PACKET_MAX_LEN	192	/* must be 48 multible */
#define MT_COMMAND_BULK_OUT_ADDR	0x8
#define MT_COMMAND_RSP_BULK_IN_ADDR	0x85

#endif
