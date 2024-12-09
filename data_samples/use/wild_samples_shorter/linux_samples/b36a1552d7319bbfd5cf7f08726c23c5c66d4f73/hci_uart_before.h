int hci_uart_init_ready(struct hci_uart *hu);
void hci_uart_init_work(struct work_struct *work);
void hci_uart_set_baudrate(struct hci_uart *hu, unsigned int speed);
void hci_uart_set_flow_control(struct hci_uart *hu, bool enable);
void hci_uart_set_speeds(struct hci_uart *hu, unsigned int init_speed,
			 unsigned int oper_speed);
