#include "my_serial.h"
#include "usbd_cdc_if.h" /* CDC_Transmit_FS */

#define MY_SERIAL_RX_SIZE 128U

static volatile uint8_t rx_buf[MY_SERIAL_RX_SIZE];
static volatile uint32_t rx_head; /* 中断里写 */
static volatile uint32_t rx_tail; /* 主循环里读 */

void my_serial_init(void)
{
    rx_head = 0;
    rx_tail = 0;
}

void my_serial_on_rx(const uint8_t *data, uint32_t len)
{
    uint32_t i;
    for (i = 0; i < len; i++)
    {
        uint32_t next = (rx_head + 1U) % MY_SERIAL_RX_SIZE;
        if (next == rx_tail)
        {
            /* 缓冲满了：丢掉最旧的一个字节，腾出位置 */
            rx_tail = (rx_tail + 1U) % MY_SERIAL_RX_SIZE;
        }
        rx_buf[rx_head] = data[i];
        rx_head = next;
    }
}

uint32_t my_serial_available(void)
{
    return (rx_head - rx_tail + MY_SERIAL_RX_SIZE) % MY_SERIAL_RX_SIZE;
}

uint8_t my_serial_read(void)
{
    uint8_t ch;
    if (my_serial_available() == 0U)
    {
        return 0;
    }
    ch = rx_buf[rx_tail];
    rx_tail = (rx_tail + 1U) % MY_SERIAL_RX_SIZE;
    return ch;
}

void my_serial_send(const uint8_t *data, uint16_t len)
{
    /* CDC_Transmit_FS 忙时（USBD_BUSY）这包会被丢弃；
       对回显/简单调试来说够用，需要可靠发送时可再排队重试。 */
    (void)CDC_Transmit_FS((uint8_t *)data, len);
}
