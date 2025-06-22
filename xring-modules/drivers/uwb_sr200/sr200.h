/*====================================================================================*/
/*                                                                                    */
/*                        Copyright 2022-2024 NXP                                     */
/*                                                                                    */
/* This program is free software; you can redistribute it and/or modify               */
/* it under the terms of the GNU General Public License as published by               */
/* the Free Software Foundation; either version 2 of the License, or                  */
/* (at your option) any later version.                                                */
/*                                                                                    */
/* This program is distributed in the hope that it will be useful,                    */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of                     */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                      */
/* GNU General Public License for more details.                                       */
/*                                                                                    */
/* You should have received a copy of the GNU General Public License                  */
/* along with this program; if not, write to the Free Software                        */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA          */
/*                                                                                    */
/*====================================================================================*/

#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/poll.h>
#include <linux/regulator/consumer.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>

#define SR200_MAGIC 0xEA
#define SR200_SET_PWR _IOW(SR200_MAGIC, 0x01, uint32_t)
#define SR200_SET_FWD _IOW(SR200_MAGIC, 0x02, uint32_t)
#define SR200_ESE_RESET _IOW(SR200_MAGIC, 0x03, uint32_t)
#define SR200_GET_THROUGHPUT _IOW(SR200_MAGIC, 0x04, uint32_t)
#define UCI_HEADER_LEN 4
#define HDLL_MODE_HEADER_LEN 2
#define NORMAL_MODE_LEN_OFFSET 3
#define UCI_NORMAL_PKT_SIZE 0
#define DIRECTIONAL_BYTE_LEN 1
#define CRC_BYTES_LEN 2
#define UCI_MT_MASK 0xE0
#define UCI_EXTND_LEN_INDICATOR_OFFSET 1
#define UCI_EXTND_LEN_INDICATOR_OFFSET_MASK 0x80
#define UCI_EXTENDED_LENGTH_OFFSET 2

#define SR200_TXBUF_SIZE 4201 //4200 + one directional byte
#define SR200_RXBUF_SIZE 4202 //4200 + two directional bytes
#define SR200_MAX_TX_BUF_SIZE 4200
#define MAX_READ_RETRY_COUNT 10
/* Macro to define SPI clock frequency */
#define SR200_SPI_CLOCK_20MHZ 20000000L
/* Maximum UCI packet size supported from the driver */
#define MAX_UCI_PKT_SIZE 4200
#define MAX_SPEED_20MHZ 20
#define MAX_SPEED_40MHZ 40
#define SR200_SPI_CLOCK_40MHZ 40000000L

/* Different driver debug lever */
struct sr200_spi_platform_data {
  unsigned int irq_gpio;
  unsigned int reset_gpio;
};
enum {
  PWR_DISABLE = 0,
  PWR_ENABLE,
  ABORT_READ_PENDING
};
enum SR200_DEBUG_LEVEL {
  SR200_DEBUG_OFF,
  SR200_FULL_DEBUG
};
enum spi_status_codes {
  spi_transcive_success,
  spi_transcive_fail,
  spi_irq_wait_request,
  spi_irq_wait_timeout
};
enum spi_operation_modes {
  SR200_WRITE_MODE,
  SR200_READ_MODE
};

/* Device specific macro and structure */
struct sr200_dev {
  wait_queue_head_t irq_wq;      /* wait queue for read interrupt */
  struct spi_device *spi;         /* spi device structure */
  struct miscdevice sr200_device; /* char device as misc driver */
  unsigned int reset_gpio;           /* SW Reset gpio */
  unsigned int irq_gpio;          /* sr200 will interrupt DH for any ntf */
  bool irq_enabled;  /* flag to indicate disable/enable irq sequence */
  bool irq_received; /* flag to indicate that irq is received */
  spinlock_t irq_enabled_lock; /* spin lock for read irq */
  unsigned char *tx_buffer;    /* transmit buffer */
  unsigned char *rx_dir_byte_buff; /* rx directional byte buffer */
  unsigned int write_count;    /* Holds nubers of  byte writen*/
  unsigned int read_count;     /* Hold nubers of  byte read */
  size_t total_bytes_to_read;
  size_t is_ext_payload_len_bit_set;
  int mode;
  long timeout_in_ms;
  struct task_struct *spi_thread;
  bool read_wait;
  struct semaphore queue_lock;
  struct semaphore transceive_lock;
  wait_queue_head_t read_wq;      /* wait queue for read interrupt */
  bool dev_suspend;
};

struct rx_buffer_node {
  struct list_head list;     //linux kernel list implementation
  unsigned char *rx_buffer;
  unsigned int rx_count;
};
