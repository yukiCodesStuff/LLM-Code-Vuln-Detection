
#define RTC_AL_SEC             0x0018

#define RTC_AL_SEC_MASK        0x003f
#define RTC_AL_MIN_MASK        0x003f
#define RTC_AL_HOU_MASK        0x001f
#define RTC_AL_DOM_MASK        0x001f
#define RTC_AL_DOW_MASK        0x0007
#define RTC_AL_MTH_MASK        0x000f
#define RTC_AL_YEA_MASK        0x007f

#define RTC_PDN2               0x002e
#define RTC_PDN2_PWRON_ALARM   BIT(4)

#define RTC_MIN_YEAR           1968