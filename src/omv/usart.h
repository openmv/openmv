#ifndef __USART_H__
#define __USART_H__
void usart_init(int baudrate);
void usart_set_baudrate(int baudrate);
void usart_send(uint8_t byte);
uint8_t usart_recv();
#endif //__USART_H__
