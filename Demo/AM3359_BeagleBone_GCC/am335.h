#ifndef am335_h
#define am335_h
/*******************************************************************************
am335h - Register definitions for TI BeagleBone Rev A4
           
THE SOFTWARE IS DELIVERED "AS IS" WITHOUT WARRANTY OR CONDITION OF ANY KIND, 
EITHER EXPRESS, IMPLIED OR STATUTORY. THIS INCLUDES WITHOUT LIMITATION ANY 
WARRANTY OR CONDITION WITH RESPECT TO MERCHANTABILITY OR FITNESS FOR ANY 
PARTICULAR PURPOSE, OR AGAINST THE INFRINGEMENTS OF INTELLECTUAL PROPERTY RIGHTS 
OF OTHERS.
           
This file may be freely used for commercial and non-commercial applications, 
including being redistributed with any tools.

If you find a problem with the file, please report it so that it can be fixed.

Created by Markos Chandras <chandram@cs.man.ac.uk>
Edited by Jerrin Shaji George <jerrinsg@gmail.com>

*******************************************************************************/

#define REG32 (volatile unsigned int*)
#define FALSE 0
#define TRUE 1

/*##############################################################################
## MISC
##############################################################################*/


#define LINE_FEED              0xA
#define CARRIAGE_RETURN        0xD
#define PRINTABLE_START        0x20
#define PRINTABLE_END          0x7E

void serial_putchar(char c);
void serial_putstring(char * c);
void serial_newline(void);
void serial_putlong(unsigned long nr);
void serial_putint(unsigned int nr);
void serial_putint_nozeros(unsigned int nr);
void serial_putbyte(unsigned char nr);
int printableChar(char c);

/* Default RAM Exception handlers */
/* Changed for bone */
#define E_UNDEFINED				(*(REG32 (0x4030CE24)))
#define E_SWI					(*(REG32 (0x4030CE28)))
#define E_PREFETCH				(*(REG32 (0x4030CE2C)))
#define E_DATA_ABRT				(*(REG32 (0x4030CE30)))
#define E_UNUSED				(*(REG32 (0x4030CE34)))
#define E_IRQ					(*(REG32 (0x4030CE38)))
#define E_FIQ					(*(REG32 (0x4030CE3C)))

/* GPTIMER REGISTERS */
/* Changing to DMTIMER */
#define DMTIMER0                0x44E05000
#define GPTI1				0x48318000
// #define GPTI2				0x49032000

/* Timer Offsets */
#define GPTI_TIOCP_CFG 			0x10
#define GPTI_TISTAT				0x14
#define GPTI_TISR				0x18
#define GPTI_TIER				0x1C
#define GPTI_TWER				0x20
#define GPTI_TCLR				0x24
#define GPTI_TCRR				0x28
#define GPTI_TLDR				0x2C
#define GPTI_TTGR				0x30
#define GPTI_TWPS				0x34
#define GPTI_TMAR				0x38
#define GPTI_TCAR1				0x3C
#define GPTI_TSICR				0x40
#define GPTI_TCAR2				0x44
#define GPTI_TPIR				0x48
#define GPTI_TNIR				0x4C
#define GPTI_TCVR				0x50
#define GPTI_TOCR				0x54
#define GPTI_TOWR				0x58


#define DMTIMER_TIDR            0x00
#define DMTIMER_TIOCP_CFG       0x10
#define DMTIMER_IRQSTATUS_RAW   0x24
#define DMTIMER_IRQSTATUS       0x28
#define DMTIMER_IRQENABLE_SET   0x2C
#define DMTIMER_IRQENABLE_CLR   0x30
#define DMTIMER_IRQWAKEEN       0x34
#define DMTIMER_TCLR            0x38
#define DMTIMER_TCRR            0x3C
#define DMTIMER_TLDR            0x40
#define DMTIMER_TTGR            0x44
#define DMTIMER_TWPS            0x48
#define DMTIMER_TMAR            0x4C
#define DMTIMER_TCAR1           0x50
#define DMTIMER_TSICR           0x54
#define DMTIMER_TCAR2           0x58

/* Define MPU_INTC */
/* Same for Bone */
#define MPU_INTC				0x48200000

/* Interrupt Controller Offsets */
/* Changed for bone */
#define INTC_REVISION           0x00
#define INTC_SYSCONFIG          0x10
#define INTC_SYSSTATUS          0x14
#define INTC_SIR_IRQ            0x40
#define INTC_SIR_FIQ            0x44
#define INTC_CONTROL            0x48
#define INTC_PROTECTION         0x4C
#define INTC_IDLE               0x50
#define INTC_IRQ_PRIORITY       0x60
#define INTC_FIQ_PRIORITY       0x64
#define INTC_THRESHOLD          0x68
#define INTC_ITR0               0x80
#define INTC_MIR0               0x84
#define INTC_MIR_CLEAR0         0x88
#define INTC_MIR_SET0           0x8C
#define INTC_ISR_SET0           0x90
#define INTC_ISR_CLEAR0         0x94
#define INTC_PENDING_IRQ0       0x98
#define INTC_PENDING_FIQ0       0x9C
#define INTC_ITR1               0xA0
#define INTC_MIR1               0xA4
#define INTC_MIR_CLEAR1         0xA8
#define INTC_MIR_SET1           0xAC
#define INTC_ISR_SET1           0xB0
#define INTC_ISR_CLEAR1         0xB4
#define INTC_PENDING_IRQ1       0xB8
#define INTC_PENDING_FIQ1       0xBC
#define INTC_ITR2               0xC0
#define INTC_MIR2               0xC4
#define INTC_MIR_CLEAR2         0xC8
#define INTC_MIR_SET2           0xCC
#define INTC_ISR_SET2           0xD0
#define INTC_ISR_CLEAR2         0xD4
#define INTC_PENDING_IRQ2       0xD8
#define INTC_PENDING_FIQ2       0xDC
#define INTC_ITR3               0xE0
#define INTC_MIR3               0xE4
#define INTC_MIR_CLEAR3         0xE8
#define INTC_MIR_SET3           0xEC
#define INTC_ISR_SET3           0xF0
#define INTC_ISR_CLEAR3         0xF4
#define INTC_PENDING_IRQ3       0xF8
#define INTC_PENDING_FIQ3       0xFC
#define INTC_ILR0               0x100
#define INTC_ILR1               0x104
#define INTC_ILR2               0x108
#define INTC_ILR3               0x10C
#define INTC_ILR4               0x110
#define INTC_ILR5               0x114
#define INTC_ILR6               0x118
#define INTC_ILR7               0x11C
#define INTC_ILR8               0x120
#define INTC_ILR9               0x124
#define INTC_ILR10              0x128
#define INTC_ILR11              0x12C
#define INTC_ILR12              0x130
#define INTC_ILR13              0x134
#define INTC_ILR14              0x138
#define INTC_ILR15              0x13C
#define INTC_ILR16              0x140
#define INTC_ILR17              0x144
#define INTC_ILR18              0x148
#define INTC_ILR19              0x14C
#define INTC_ILR20              0x150
#define INTC_ILR21              0x154
#define INTC_ILR22              0x158
#define INTC_ILR23              0x15C
#define INTC_ILR24              0x160
#define INTC_ILR25              0x164
#define INTC_ILR26              0x168
#define INTC_ILR27              0x16C
#define INTC_ILR28              0x170
#define INTC_ILR29              0x174
#define INTC_ILR30              0x178
#define INTC_ILR31              0x17C
#define INTC_ILR32              0x180
#define INTC_ILR33              0x184
#define INTC_ILR34              0x188
#define INTC_ILR35              0x18C
#define INTC_ILR36              0x190
#define INTC_ILR37              0x194
#define INTC_ILR38              0x198
#define INTC_ILR39              0x19C
#define INTC_ILR40              0x1A0
#define INTC_ILR41              0x1A4
#define INTC_ILR42              0x1A8
#define INTC_ILR43              0x1AC
#define INTC_ILR44              0x1B0
#define INTC_ILR45              0x1B4
#define INTC_ILR46              0x1B8
#define INTC_ILR47              0x1BC       
#define INTC_ILR48              0x1C0
#define INTC_ILR49              0x1C4
#define INTC_ILR50              0x1C8
#define INTC_ILR51              0x1CC
#define INTC_ILR52              0x1D0
#define INTC_ILR53              0x1D4
#define INTC_ILR54              0x1D8
#define INTC_ILR55              0x1DC
#define INTC_ILR56              0x1E0
#define INTC_ILR57              0x1E4
#define INTC_ILR58              0x1E8
#define INTC_ILR59              0x1EC
#define INTC_ILR60              0x1F0
#define INTC_ILR61              0x1F4
#define INTC_ILR62              0x1F8
#define INTC_ILR63              0x1FC
#define INTC_ILR64              0x200
#define INTC_ILR65              0x204
#define INTC_ILR66              0x208
#define INTC_ILR67              0x20C
#define INTC_ILR68              0x210
#define INTC_ILR69              0x214
#define INTC_ILR70              0x218
#define INTC_ILR71              0x21C
#define INTC_ILR72              0x220
#define INTC_ILR73              0x224
#define INTC_ILR74              0x228
#define INTC_ILR75              0x22C
#define INTC_ILR76              0x230
#define INTC_ILR77              0x234
#define INTC_ILR78              0x238
#define INTC_ILR79              0x23C
#define INTC_ILR80              0x240
#define INTC_ILR81              0x244
#define INTC_ILR82              0x248
#define INTC_ILR83              0x24C
#define INTC_ILR84              0x250
#define INTC_ILR85              0x254
#define INTC_ILR86              0x258
#define INTC_ILR87              0x25C
#define INTC_ILR88              0x260
#define INTC_ILR89              0x264
#define INTC_ILR90              0x268
#define INTC_ILR91              0x26C
#define INTC_ILR92              0x270
#define INTC_ILR93              0x274
#define INTC_ILR94              0x278
#define INTC_ILR95              0x27C
#define INTC_ILR96              0x280
#define INTC_ILR97              0x284
#define INTC_ILR98              0x288
#define INTC_ILR99              0x28C
#define INTC_ILR100             0x290
#define INTC_ILR101             0x294
#define INTC_ILR102             0x298
#define INTC_ILR103             0x29C
#define INTC_ILR104             0x2A0
#define INTC_ILR105             0x2A4
#define INTC_ILR106             0x2A8
#define INTC_ILR107             0x2AC
#define INTC_ILR108             0x2B0
#define INTC_ILR109             0x2B4
#define INTC_ILR110             0x2B8
#define INTC_ILR111             0x2BC
#define INTC_ILR112             0x2C0
#define INTC_ILR113             0x2C4
#define INTC_ILR114             0x2C8
#define INTC_ILR115             0x2CC
#define INTC_ILR116             0x2D0
#define INTC_ILR117             0x2D4
#define INTC_ILR118             0x2D8
#define INTC_ILR119             0x2DC
#define INTC_ILR120             0x2E0
#define INTC_ILR121             0x2E4
#define INTC_ILR122             0x2E8
#define INTC_ILR123             0x2EC
#define INTC_ILR124             0x2F0
#define INTC_ILR125             0x2F4
#define INTC_ILR126             0x2F8
#define INTC_ILR127             0x2FC


/* GPIO */
/* Changed for Bone */
#define GPIO0_BASE              0x44E07000
#define GPIO1_BASE              0x4804C000
#define GPIO2_BASE              0x481AC000
#define GPIO3_BASE              0x481AE000

/* GPIO Offsets */
/* Changed for Bone */
#define GPIO_REVISION           0x0
#define GPIO_SYSCONFIG          0x10
#define GPIO_IRQSTATUS_RAW_0    0x24
#define GPIO_IRQSTATUS_RAW_1    0x28
#define GPIO_IRQSTATUS_0        0x2C
#define GPIO_IRQSTATUS_1        0x30
#define GPIO_IRQSTATUS_SET_0    0x34
#define GPIO_IRQSTATUS_SET_1    0x38
#define GPIO_IRQSTATUS_CLR_0    0x3C
#define GPIO_IRQSTATUS_CLR_1    0x40
#define GPIO_SYSSTATUS          0x114
#define GPIO_CTRL               0x130
#define GPIO_OE                 0x134
#define GPIO_DATAIN             0x138
#define GPIO_DATAOUT            0x13C
#define GPIO_LEVELDETECT0       0x140
#define GPIO_LEVELDETECT1       0x144
#define GPIO_RISINGDETECT       0x148
#define GPIO_FALLINGDETECT      0x14C
#define GPIO_DEBOUNCENABLE      0x150
#define GPIO_DEBOUNCINGTIME     0x154
#define GPIO_CLEARDATAOUT       0x190
#define GPIO_SETDATAOUT         0x194
/* GPIO Changes End */


/* Pin definitions */
#define PIN0					(0x1 << 0)
#define PIN1					(0x1 << 1)
#define PIN2					(0x1 << 2)
#define	PIN3					(0x1 << 3)
#define PIN4					(0x1 << 4)
#define PIN5					(0x1 << 5)
#define PIN6					(0x1 << 6)
#define PIN7					(0x1 << 7)
#define PIN8					(0x1 << 8)
#define PIN9					(0x1 << 9)
#define PIN10					(0x1 << 10)
#define PIN11					(0x1 << 11)
#define PIN12					(0x1 << 12)
#define PIN13					(0x1 << 13)
#define PIN14					(0x1 << 14)
#define PIN15					(0x1 << 15)
#define PIN16					(0x1 << 16)
#define PIN17					(0x1 << 17)
#define PIN18					(0x1 << 18)
#define PIN19					(0x1 << 19)
#define PIN20					(0x1 << 20)
#define PIN21					(0x1 << 21)
#define PIN22					(0x1 << 22)
#define PIN23					(0x1 << 23)
#define PIN24					(0x1 << 24)
#define PIN25					(0x1 << 25)
#define PIN26					(0x1 << 26)
#define PIN27					(0x1 << 27)
#define PIN28					(0x1 << 28)
#define PIN29					(0x1 << 29)
#define PIN30					(0x1 << 30)
#define PIN31					(0x1 << 31)

/* Serial Configuration (UART 3)*/
#define SERIAL_BASE 		    0x49020000

/* Serial Offsets */
#define DLL_REG					0x000
#define RHR_REG					0x000
#define THR_REG					0x000
#define DLH_REG					0x004
#define IER_REG					0x004
#define IIR_REG					0x008
#define FCR_REG					0x008
#define EFR_REG					0x008
#define LCR_REG					0x00C
#define MCR_REG					0x010
#define XON1_ADDR1_REG			0x010
#define LSR_REG					0x014
#define XON2_ADDR2_REG			0x014
#define MSR_REG					0x018
#define TCR_REG					0x018
#define XOFF1_REG				0x018
#define SPR_REG					0x01C
#define TLR_REG					0x01C
#define XOFF2_REG				0x01C
#define MDR1_REG				0x020
#define MDR2_REG				0x024
#define SFLSR_REG				0x028
#define TXFLL_REG				0x028
#define RESUME_REG				0x02C
#define TXFLH_REG				0x02C
#define SFREGL_REG				0x030
#define RXFLL_REG				0x030
#define SFREGH_REG				0x034
#define RXFLH_REG				0x034
#define UASR_REG				0x038
#define BLR_REG					0x038
#define ACREG_REG				0x03C
#define SCR_REG					0x040
#define SSR_REG					0x044
#define EBLR_REG				0x048
#define MVR_REG					0x050
#define SYSC_REG				0x054
#define SYSS_REG				0x058
#define WER_REG					0x05C
#define CFPS_REG				0x060

#define PRCM_REG                0x44e00000
#define CM_PER_GPIO1_CLKCTRL    0xAC
#define CM_PER_LCDC_CLKCTRL     0x18

#endif /* omap3_h */
