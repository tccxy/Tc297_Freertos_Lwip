#ifndef TLF35584_H_
#define TLF35584_H_
/******************************************************************************/

/******************************************************************************/
/* Types                                                              */
/******************************************************************************/
typedef enum
{
	Tlf_Status_Ok = 0,
	Tlf_Status_Not_Ok = 1,
	Tlf_Status_Err_Spi = 2,
	Tlf_Status_Err_State = 3,
	Tlf_Status_Err_Config = 4

} Tlf_Status;


/******************************************************************************/
/* Macro defines                                                              */
/******************************************************************************/
/* TLF35584 Register definitions */


#define DEVCFG0            ((uint8)0x00U)    /* Device configuration 0 */
#define DEVCFG1            ((uint8)0x01U)    /* Device configuration 1 */
#define DEVCFG2            ((uint8)0x02U)    /* Device configuration 2 */
#define PROTCFG            ((uint8)0x03U)    /* Protection register */
#define SYSPCFG0           ((uint8)0x04U)    /* System protected configuration 0 */
#define SYSPCFG1           ((uint8)0x05U)    /* System protected configuration 1 */
#define WDCFG0             ((uint8)0x06U)    /* Watchdog configuration 0 */
#define WDCFG1             ((uint8)0x07U)    /* Watchdog configuration 1 */
#define FWDCFG             ((uint8)0x08U)    /* Functional Watchdog configuration */
#define WWDCFG0            ((uint8)0x09U)    /* Window Watchdog configuration 0 */
#define WWDCFG1            ((uint8)0x0AU)    /* Window Watchdog configuration 1 */
#define GTM                ((uint8)0x3FU)    /* Global test mode */


#define RSYSPCFG0           ((uint8)0x0BU)   /* System configuration 0 status */
#define RSYSPCFG1           ((uint8)0x0CU)   /* System configuration 1 status */
#define RWDCFG0             ((uint8)0x0DU)   /* Watchdog configuration 0 status */
#define RWDCFG1             ((uint8)0x0EU)   /* Watchdog configuration 1 status */
#define RFWDCFG             ((uint8)0x0FU)   /* Functional watchdog configuration status */
#define RWWDCFG0            ((uint8)0x10U)   /* Window watchdog configuration 0 status */
#define RWWDCFG1            ((uint8)0x11U)   /* Window watchdog configuration 1 status */
#define	WKTIMCFG0      		((uint8)0x12U)   /* Wake timer configuration 0 */
#define WKTIMCFG1           ((uint8)0x13U)   /* Wake timer configuration 1 */
#define WKTIMCFG2           ((uint8)0x14U)   /* Wake timer configuration 2 */

#define DEVCTRL             ((uint8)0x15U)   /* Device control */
#define DEVCTRLN            ((uint8)0x16U)   /* Device control inverted request */
#define WWDSCMD             ((uint8)0x17U)   /* Window watchdog service command */
#define FWDRSP              ((uint8)0x18U)   /* Functional watchdog response command */
#define FWDRSPSYNC          ((uint8)0x19U)   /* Functional watchdog response command with synchronization */
#define SYSFAIL             ((uint8)0x1AU)   /* Failure status flags */
#define INITERR             ((uint8)0x1BU)   /* Init error status flags */
#define IF                  ((uint8)0x1CU)   /* Interrupt flags */
#define SYSSF               ((uint8)0x1DU)   /* System status flags */
#define WKSF                ((uint8)0x1EU)   /* Wake up status flags */
#define SPISF               ((uint8)0x1FU)   /* SPI status flags */
#define MONSF0              ((uint8)0x20U)   /* Monitor status flags 0 */
#define MONSF1              ((uint8)0x21U)   /* Monitor status flags 1 */
#define MONSF2              ((uint8)0x22U)   /* Monitor status flags 2 */
#define MONSF3              ((uint8)0x23U)   /* Monitor status flags 3 */
#define OTFAIL              ((uint8)0x24U)   /* Over temperature failure status flags */
#define OTWRNSF             ((uint8)0x25U)   /* Over temperature warning status flags */
#define VMONSTAT            ((uint8)0x26U)   /* Voltage monitor status */
#define DEVSTAT             ((uint8)0x27U)   /* Device status */
#define PROTSTAT            ((uint8)0x28U)   /* Protection status */
#define WWDSTAT             ((uint8)0x29U)   /* Window watchdog status */
#define FWDSTAT0            ((uint8)0x2AU)   /* Functional watchdog status 0 */
#define FWDSTAT1            ((uint8)0x2BU)   /* Functional watchdog status 1 */

#define ABIST_CTRL0         ((uint8)0x2CU)   /* ABIST control0 */
#define ABIST_CTRL1         ((uint8)0x2DU)   /* ABIST control1 */
#define ABIST_SELECT0       ((uint8)0x2EU)   /* ABIST select 0 */
#define ABIST_SELECT1       ((uint8)0x2FU)   /* ABIST select 1 */
#define ABIST_SELECT2       ((uint8)0x30U)   /* ABIST select 2 */

                     				         /* Buck trim */
#define BCK_FREQ_CHANGE     ((uint8)0x31U)   /* Buck switching frequency change */
#define BCK_FRE_SPREAD      ((uint8)0x32U)   /* Buck Frequency spread */
#define BCK_MAIN_CTRL       ((uint8)0x33U)   /* Buck main control */

/* Register SYSPCFG0, System protected configuration 0 */
#define STBYEN_BIT_POS      (3U)
#define ERRSLPEN_BIT_POS    (2U)
#define ERREN_BIT_POS       (1U)
#define RECEN_BIT_POS       (0U)

/* Register WDCFG0, Watchdog configuration 0 */
#define WWDETHR_BIT_POS     (4U)
#define WWDEN_BIT_POS       (3U)
#define FWDEN_BIT_POS       (2U)
#define WWDTSEL_BIT_POS     (1U)
#define WDCYC_BIT_POS       (0U)

/* Register WDCFG1, Watchdog configuration 1 */
#define WDSLPEN_BIT_POS     (4U)
#define FWDETHR_BIT_POS     (0U)

/* Register FWDCFG, Functional Watchdog configuration */
#define WDHBTP_BIT_POS      (0U)

/* Register WWDCFG0, Window Watchdog configuration 0 */
#define CW_BIT_POS          (0U)

/* Register WWDCFG1, Window Watchdog configuration 1 */
#define OW_BIT_POS          (0U)


/******************************************************************************/
/* Function declarations                                                      */
/******************************************************************************/
void TLF35584Demo_ConfigSysWdCfg(void);
void TLF35584Demo_ErrorCallBack(Tlf_Status RetStatus);
void TLF35584_Init(void);
void TLF35584Demo(void);


/******************************************************************************/
#endif /* TLF35584DEMO_H_ */
