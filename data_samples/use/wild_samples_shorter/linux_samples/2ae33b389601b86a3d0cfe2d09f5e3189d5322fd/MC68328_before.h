/*
 * Here go the bitmasks themselves
 */
#define IMR_MSPIM 	(1 << SPIM _IRQ_NUM)	/* Mask SPI Master interrupt */
#define	IMR_MTMR2	(1 << TMR2_IRQ_NUM)	/* Mask Timer 2 interrupt */
#define IMR_MUART	(1 << UART_IRQ_NUM)	/* Mask UART interrupt */	
#define	IMR_MWDT	(1 << WDT_IRQ_NUM)	/* Mask Watchdog Timer interrupt */
#define IMR_MRTC	(1 << RTC_IRQ_NUM)	/* Mask RTC interrupt */
#define IWR_ADDR	0xfffff308
#define IWR		LONG_REF(IWR_ADDR)

#define IWR_SPIM 	(1 << SPIM _IRQ_NUM)	/* SPI Master interrupt */
#define	IWR_TMR2	(1 << TMR2_IRQ_NUM)	/* Timer 2 interrupt */
#define IWR_UART	(1 << UART_IRQ_NUM)	/* UART interrupt */	
#define	IWR_WDT		(1 << WDT_IRQ_NUM)	/* Watchdog Timer interrupt */
#define IWR_RTC		(1 << RTC_IRQ_NUM)	/* RTC interrupt */
#define ISR_ADDR	0xfffff30c
#define ISR		LONG_REF(ISR_ADDR)

#define ISR_SPIM 	(1 << SPIM _IRQ_NUM)	/* SPI Master interrupt */
#define	ISR_TMR2	(1 << TMR2_IRQ_NUM)	/* Timer 2 interrupt */
#define ISR_UART	(1 << UART_IRQ_NUM)	/* UART interrupt */	
#define	ISR_WDT		(1 << WDT_IRQ_NUM)	/* Watchdog Timer interrupt */
#define ISR_RTC		(1 << RTC_IRQ_NUM)	/* RTC interrupt */
#define IPR_ADDR	0xfffff310
#define IPR		LONG_REF(IPR_ADDR)

#define IPR_SPIM 	(1 << SPIM _IRQ_NUM)	/* SPI Master interrupt */
#define	IPR_TMR2	(1 << TMR2_IRQ_NUM)	/* Timer 2 interrupt */
#define IPR_UART	(1 << UART_IRQ_NUM)	/* UART interrupt */	
#define	IPR_WDT		(1 << WDT_IRQ_NUM)	/* Watchdog Timer interrupt */
#define IPR_RTC		(1 << RTC_IRQ_NUM)	/* RTC interrupt */

/* 'EZ328-compatible definitions */
#define TCN_ADDR	TCN1_ADDR
#define TCN		TCN

/*
 * Timer Unit 1 and 2 Status Registers
 */