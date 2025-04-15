/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

#include "uart.h"

/* convert baudrate to divisor */
#define UART_CONVERT_BAUD_TO_DIVISOR(peripheral_freq, baud)  ((peripheral_freq) / ((baud) * 16))

/**
 * @fn      bool uart_tx_ready (UART_Type *uart)
 * @brief   check whether uart is ready to send; 1 ready, 0 not ready
 * @note    none
 * @param   uart: Pointer to uart register set structure
 * @retval  1 ready to send, 0 not ready to send
 */
static inline bool uart_tx_ready (UART_Type *uart)
{
    /* read TFNF transmitt_fifo_not_full bit from usr uart status register */
    return ((uart->UART_USR & UART_USR_TRANSMIT_FIFO_NOT_FULL) ? 1 : 0);
}

/**
 * @fn      bool uart_rx_ready (UART_Type *uart)
 * @brief   check whether uart is ready to receive; 1 ready, 0 not ready
 * @note    none
 * @param   uart: Pointer to uart register set structure
 * @retval  1 ready to receive, 0 not ready to receive
 */
static inline bool uart_rx_ready (UART_Type *uart)
{
    /* read RFNE receive_fifo_not_empty bit from usr uart status register */
    return ((uart->UART_USR & UART_USR_RECEIVE_FIFO_NOT_EMPTY) ? 1 : 0);
}

/**
 * @fn      void uart_send_a_char_to_thr (UART_Type *uart,
                                          char       chr)
 * @brief   write a char to uart transmit holding register
 * @note    none
 * @param   uart : Pointer to uart register set structure
 * @param   chr          : char to send to thr register
 * @retval  none
 */
static inline void uart_send_a_char_to_thr (UART_Type *uart,
                                            char       chr)
{
    /* write a char to thr transmit holding register */
    uart->UART_THR = chr;
}

/**
 * @fn      int32_t uart_receive_a_char_from_rbr (UART_Type *uart)
 * @brief   read data from uart receive buffer register
 * @note    none
 * @param   uart: Pointer to uart register set structure
 * @retval  received data
 */
static inline int32_t uart_receive_a_char_from_rbr (UART_Type *uart)
{
    /* read a char from receive buffer register */
    return uart->UART_RBR;
}

/**
 * @fn      int32_t uart_get_tx_fifo_available_count (UART_Type *uart)
 * @brief   get available transmit fifo count
 * @note    useful in interrupt callback
 * @param   uart: Pointer to uart register set structure
 * @retval  available transmit fifo count
 */
static inline int32_t uart_get_tx_fifo_available_count (UART_Type *uart)
{
    /* read tfl transmit FIFO level register,
     *  TX_fifo available count = fifo depth - data entries in transmit fifo
     */
    return (UART_FIFO_DEPTH - uart->UART_TFL);
}

/**
 * @fn      int32_t uart_get_rx_fifo_available_count (UART_Type *uart)
 * @brief   get available receive fifo count
 * @note    useful in interrupt callback
 * @param   uart: Pointer to uart register set structure
 * @retval  available receive fifo count
 */
static inline int32_t uart_get_rx_fifo_available_count (UART_Type *uart)
{
    /* read rfl receive FIFO level register */
    return (uart->UART_RFL);
}

/**
 * @fn      void uart_send_blocking (UART_Type *uart, UART_TRANSFER *transfer)
 * @brief   uart send using blocking/polling method,
 *           this will block till uart sends all the data.
 * @param   uart     : Pointer to uart register set structure
 * @param   transfer : Pointer to uart transfer structure
 * @retval  none
 */
void uart_send_blocking (UART_Type *uart, UART_TRANSFER *transfer)
{
    /* TX fifo Available count. */
    uint32_t tx_fifo_available_cnt  = 0U;
    uint32_t i = 0U;

    /* block till uart sends all the data */
    while ( transfer->tx_curr_cnt < transfer->tx_total_num )
    {
        /* wait until uart is to ready to send */
        while (!uart_tx_ready(uart)); /* blocked */

        /* Query how many characters are available in TX fifo. */
        tx_fifo_available_cnt = uart_get_tx_fifo_available_count (uart);

        /* Write maximum number of characters to
         * the TX fifo as per available space. */
        for(i = 0; i < tx_fifo_available_cnt; i++)
        {
            if(transfer->tx_curr_cnt >= transfer->tx_total_num)
            {
                /* Come out as it transmitted all the user data. */
                break;
            }

            /* send character to thr register. */
            uart_send_a_char_to_thr(uart, transfer->tx_buf[transfer->tx_curr_cnt]);
            transfer->tx_curr_cnt++; /* increment the tx current count */
        }
    }
}

/**
 * @fn      void uart_receive_blocking (UART_Type *uart, UART_TRANSFER *transfer)
 * @brief   uart receive using blocking/polling method,
 *           this will block till uart receives all the data.
 * @param   uart     : Pointer to uart register set structure
 * @param   transfer : Pointer to uart transfer structure
 * @retval  none
 */
void uart_receive_blocking (UART_Type *uart, UART_TRANSFER *transfer)
{
    /* RX fifo Available count. */
    uint32_t rx_fifo_available_cnt  = 0U;

    /* RX line status. */
    uint32_t rx_line_status = 0U;
    uint32_t i = 0U;

    /* block till uart receives all the data. */
    while(transfer->rx_curr_cnt < transfer->rx_total_num)
    {
        /* wait until uart is ready to receive */
        while (!uart_rx_ready(uart)); /* blocked */

        /* Query how many characters are available in RX fifo. */
        rx_fifo_available_cnt = uart_get_rx_fifo_available_count (uart);

        /* Read maximum number of characters available from
         * the RX fifo or till rx total number. */
        for(i = 0; i < rx_fifo_available_cnt; i++)
        {
            if (transfer->rx_curr_cnt >= transfer->rx_total_num)
            {
                /* Come out as it received all the user data. */
                break;
            }

            /* check for any RX line status error. */
            rx_line_status = uart->UART_LSR;

            if (rx_line_status & (UART_LSR_RECEIVER_FIFO_ERR | UART_LSR_OVERRUN_ERR) )
            {
                /* mark status as error. */
                transfer->status = UART_TRANSFER_STATUS_ERROR;

                /* there can be multiple RX line status,
                 * break character implicitly generates a framing error / parity error.
                 */

                if (rx_line_status & UART_LSR_BREAK_INTERRUPT)
                {
                    transfer->status |= UART_TRANSFER_STATUS_ERROR_RX_BREAK;
                }

                if (rx_line_status & UART_LSR_FRAME_ERR)
                {
                    transfer->status |= UART_TRANSFER_STATUS_ERROR_RX_FRAMING;
                }

                if (rx_line_status & UART_LSR_PARITY_ERR)
                {
                    transfer->status |= UART_TRANSFER_STATUS_ERROR_RX_PARITY;
                }

                if (rx_line_status & UART_LSR_OVERRUN_ERR)
                {
                    transfer->status |= UART_TRANSFER_STATUS_ERROR_RX_OVERRUN;
                }
            }

            /* read character from rbr receive buffer register. */
            transfer->rx_buf[transfer->rx_curr_cnt] = uart_receive_a_char_from_rbr(uart);
            transfer->rx_curr_cnt++;
        }
    }
}

/**
 * @fn      void uart_set_baudrate (UART_Type  *uart, uint32_t clk, uint32_t baudrate)
 * @brief   set uart baudrate
 * @note    added support for fraction in dlf divisor latch fraction register
 * @param   uart     : Pointer to uart register set structure
 * @param   clk      : clock
 * @param   baudrate : baudrate
 * @retval  none
 */
void uart_set_baudrate (UART_Type  *uart, uint32_t clk, uint32_t baudrate)
{
    int32_t baud_divisor    = 0;
    int32_t fraction        = 0;
    float decimal           = 0.0;
    int32_t i               = 0;

    if(baudrate)
    {
        decimal = UART_CONVERT_BAUD_TO_DIVISOR((float)clk, (float)baudrate);
        baud_divisor = (int32_t)UART_CONVERT_BAUD_TO_DIVISOR(clk, baudrate);
        fraction = (int32_t)((decimal - baud_divisor) * (1 << UART_DLF_SIZE));
    }

    /* enable DLAB divisor latch access bit in lcr line control register  */
    uart->UART_LCR |= UART_LCR_DLAB;

    /* setting uart baudrate registers   */
    uart->UART_DLL = baud_divisor & 0xff;       /* DLL divisor latch low register       */
    uart->UART_DLH = (baud_divisor>>8) & 0xff;  /* DLH divisor latch high register      */
    uart->UART_DLF = fraction;                  /* DLF divisor latch fraction register  */

    /* disable DLAB */
    uart->UART_LCR &= ~(UART_LCR_DLAB);

    /* hardware requires this delay before operating on new baud */
    for(i = 0; i < (32 * baud_divisor * 1); i++);
}

/**
 * @fn      void uart_set_data_parity_stop_bits(UART_Type       *uart,
                                                UART_DATA_BITS   data_bits,
                                                UART_PARITY      parity,
                                                UART_STOP_BITS   stop_bits)
 * @brief   set uart asynchronous parameters:
 *          data bits, parity, stop bits
 * @note    none
 * @param   uart        : Pointer to uart register set structure
 * @param   data_bits   : data bits
 * @param   parity      : parity
 * @param   stop_bits   : stop bits
 * @retval  none
 */
void uart_set_data_parity_stop_bits(UART_Type      *uart,
                                    UART_DATA_BITS  data_bits,
                                    UART_PARITY     parity,
                                    UART_STOP_BITS  stop_bits)
{
    uint32_t lcr  = 0;

    /* UART Data bits */
    switch (data_bits)
    {
        /* Data bit is not configurable */
        /* set DLS data_length_select bit in lcr line control register */
        case UART_DATA_BITS_5: lcr |= (UART_LCR_DATA_LENGTH_5); break;
        case UART_DATA_BITS_6: lcr |= (UART_LCR_DATA_LENGTH_6); break;
        case UART_DATA_BITS_7: lcr |= (UART_LCR_DATA_LENGTH_7); break;
        case UART_DATA_BITS_8: lcr |= (UART_LCR_DATA_LENGTH_8); break;
    }

    /* UART Parity */
    switch (parity)
    {
        /* set PEN parity enable, EPS even parity select bit in
         * lcr line control register */
        case UART_PARITY_NONE: lcr |= (UART_LCR_PARITY_NONE); break;
        case UART_PARITY_EVEN: lcr |= (UART_LCR_PARITY_EVEN); break;
        case UART_PARITY_ODD:  lcr |= (UART_LCR_PARITY_ODD);  break;
    }

    /* UART Stop bits */
    switch (stop_bits)
    {
        /* set STOP number_of_stop_bits in lcr line control register */
        case UART_STOP_BITS_1: lcr |= (UART_LCR_STOP_1BIT); break;
        case UART_STOP_BITS_2: lcr |= (UART_LCR_STOP_2BIT); break;
    }

    /* clear data,parity,stop bits */
    uart->UART_LCR &= (~UART_LCR_DATA_PARITY_STOP_MASK);

    /* set data,parity,stop bits */
    uart->UART_LCR |= lcr;
}

/**
 * @fn      void uart_set_flow_control(UART_Type         *uart,
                                       UART_FLOW_CONTROL  flow_control)
 * @brief   set uart asynchronous parameters:
 *          flow control RTS/CTS
 * @note    none
 * @param   uart         : Pointer to uart register set structure
 * @param   flow_control : flow control RTS/CTS
 * @retval  none
 */
void uart_set_flow_control(UART_Type          *uart,
                           UART_FLOW_CONTROL   flow_control)
{
    /* uart flow control */
    switch (flow_control)
    {
        /* set flow control bit in mcr modem control register */
        case UART_FLOW_CONTROL_NONE:
            uart->UART_MCR &= ~(UART_MCR_AFCE|UART_MCR_RTS);
            break;

        case UART_FLOW_CONTROL_RTS:
            uart->UART_MCR |= (UART_MCR_AFCE|UART_MCR_RTS);
            break;

        case UART_FLOW_CONTROL_CTS:
            uart->UART_MCR |= (UART_MCR_AFCE);
            break;

        case UART_FLOW_CONTROL_RTS_CTS:
            uart->UART_MCR |= (UART_MCR_AFCE|UART_MCR_RTS);
            break;
    }
}

/**
 * @fn      void uart_irq_handler (UART_Type *uart, UART_TRANSFER *transfer)
 * @brief   uart interrupt handler
 * @note    only one combined interrupt for
 *              -TX / RX
 *              -RX_Character_Timeout
 *              -Modem status
 *              -Receiver Line status
 *          in RX_Character_Timeout case not clearing rx busy flag,
 *          it is up to user to decide whether
 *          to wait for remaining bytes or call the abort rx. \ref abort_rx
 * @param   uart     : Pointer to uart register set structure
 * @param   transfer : Pointer to uart transfer structure
 * @retval  none
 */
void uart_irq_handler (UART_Type *uart, UART_TRANSFER *transfer)
{
    uint32_t uart_int_status        = 0U;   /* uart interrupt status    */
    uint32_t rx_line_status         = 0U;   /* uart rx line status      */
    uint32_t tx_fifo_available_cnt  = 0U;   /* TX fifo Available count. */
    uint32_t rx_fifo_available_cnt  = 0U;   /* RX fifo Available count. */
    uint32_t i = 0U;

    /* get uart interrupt status from iir interrupt identity register */
    uart_int_status = (uart->UART_IIR) & UART_IIR_INTERRUPT_ID_MASK;

    switch (uart_int_status)
    {
        case UART_IIR_MODEM_STATUS: /* modem status */
            (void)(uart->UART_MSR);
            /* not yet implemented. */
            break;

        case UART_IIR_RECEIVER_LINE_STATUS: /* receiver line status */
            rx_line_status = uart->UART_LSR;

            /* check for any RX line status error. */
            if (rx_line_status & (UART_LSR_RECEIVER_FIFO_ERR | UART_LSR_OVERRUN_ERR) )
            {
                /* mark status as error. */
                transfer->status = UART_TRANSFER_STATUS_ERROR;

                /* there can be multiple RX line status,
                 * break character implicitly generates framing error / parity error.
                 */
                if (rx_line_status & UART_LSR_BREAK_INTERRUPT)
                {
                    transfer->status |= UART_TRANSFER_STATUS_ERROR_RX_BREAK;
                }

                if (rx_line_status & UART_LSR_FRAME_ERR)
                {
                    transfer->status |= UART_TRANSFER_STATUS_ERROR_RX_FRAMING;
                }

                if (rx_line_status & UART_LSR_PARITY_ERR)
                {
                    transfer->status |= UART_TRANSFER_STATUS_ERROR_RX_PARITY;
                }

                if (rx_line_status & UART_LSR_OVERRUN_ERR)
                {
                    transfer->status |= UART_TRANSFER_STATUS_ERROR_RX_OVERRUN;
                }
            }
            break;

        case UART_IIR_TRANSMIT_HOLDING_REG_EMPTY: /* transmit holding register empty */
            do
            {
                /* Query how many characters are available in TX fifo. */
                tx_fifo_available_cnt = uart_get_tx_fifo_available_count (uart);

                /* Write maximum number of characters to the TX fifo as per available space. */
                for(i=0; i<tx_fifo_available_cnt; i++)
                {
                    if(transfer->tx_curr_cnt >= transfer->tx_total_num)
                    {
                        /* Come out as it transmitted all the user data. */
                        break;
                    }

                    /* send character to thr register. */
                    uart_send_a_char_to_thr(uart, transfer->tx_buf[transfer->tx_curr_cnt]);
                    transfer->tx_curr_cnt++; /* increment the tx current count */
                }

                /* write again to tx fifo if it is not full and
                 * still there is some user data which needs to be send. */
            } while( uart_tx_ready(uart) && (transfer->tx_curr_cnt < transfer->tx_total_num) );

            /* check whether it transmitted all the bytes? */
            if (transfer->tx_curr_cnt >= transfer->tx_total_num)
            {
                /* yes then disable the transmitter interrupt */
                uart_disable_tx_irq(uart);

                /* mark status as Send Complete */
                transfer->status = UART_TRANSFER_STATUS_SEND_COMPLETE;
            }
            break;

        case UART_IIR_CHARACTER_TIMEOUT:        /* character timeout */
        case UART_IIR_RECEIVED_DATA_AVAILABLE:  /* received data available. */
            do
            {
                /* Query how many characters are available in RX fifo. */
                rx_fifo_available_cnt = uart_get_rx_fifo_available_count (uart);

                /* Read maximum number of characters available from the RX fifo or till rx total number. */
                for(i=0; i<rx_fifo_available_cnt; i++)
                {
                    if (transfer->rx_curr_cnt >= transfer->rx_total_num)
                    {
                        /* Come out as it received all the user data. */
                        break;
                    }

                    /* read character from rbr receive buffer register. */
                    transfer->rx_buf[transfer->rx_curr_cnt] = uart_receive_a_char_from_rbr(uart);
                    transfer->rx_curr_cnt++;
                }

                /* read again from rx fifo if it is not empty and user data is still remaining to read. */
            } while( uart_rx_ready(uart) && (transfer->rx_curr_cnt < transfer->rx_total_num) );

            /* check whether it received all the bytes? */
            if (transfer->rx_curr_cnt >= transfer->rx_total_num)
            {
                /* yes than disable the receiver interrupt */
                uart_disable_rx_irq(uart);

                /* mark status as Receive Complete */
                transfer->status = UART_TRANSFER_STATUS_RECEIVE_COMPLETE;
            }
            else /* fifo is empty. */
            {
                /* FIXME After debugging we found that without else case, code is getting hang here as
                 * 1.) fifo is empty here, (as we are reading fifo in while loop),
                 *     and if we have not read total number of user RX data.
                 * 2.) Further RX_CHAR_TIMEOUT interrupt will not come as fifo is empty.
                 *     (As per Datasheet : Interrupt source for RX_CHAR_TIMEOUT
                 *          No characters in or out of the RCVR FIFO during the last 4 character times
                 *          and there is at least 1 character in it during this time)
                 * 3.) Added this to fix timeout issue (work-around)
                 * 4.) scenario: expecting 10 bytes but receiving only 1 byte? needs one more upper layer timeout.
                 * 5.) or Mark as separate event for "FIFO_EMPTY", Don't include in RX_Timeout.
                 */

                /* in RX_Timeout case not clearing rx busy flag
                 * it is up to user to decide whether
                 * to wait for remaining bytes or call the abort rx.
                */

                /* mark status as RX Timeout */
                transfer->status = UART_TRANSFER_STATUS_RX_TIMEOUT;
            }

            /* got character Timeout? mark status as a RX Timeout. */
            if (uart_int_status == UART_IIR_CHARACTER_TIMEOUT)
            {
                /* in RX_Timeout case not clearing rx busy flag
                 * it is up to user to decide whether
                 * to wait for remaining bytes or call the abort rx.
                 */

                /* mark status as RX Timeout */
                transfer->status = UART_TRANSFER_STATUS_RX_TIMEOUT;
            }

            break;

        default:
            /* read the usr uart status register */
            (void)(uart->UART_USR);
            break;
    }

    return;
}
