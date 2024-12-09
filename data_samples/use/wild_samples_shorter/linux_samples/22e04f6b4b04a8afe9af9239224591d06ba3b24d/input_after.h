#define BTN_DPAD_LEFT		0x222
#define BTN_DPAD_RIGHT		0x223

#define BTN_FRET_FAR_UP		0x224
#define BTN_FRET_UP		0x225
#define BTN_FRET_MID		0x226
#define BTN_FRET_LOW		0x227
#define BTN_FRET_FAR_LOW	0x228
#define BTN_STRUM_BAR_UP	0x229
#define BTN_STRUM_BAR_DOWN	0x22a

#define BTN_TRIGGER_HAPPY		0x2c0
#define BTN_TRIGGER_HAPPY1		0x2c0
#define BTN_TRIGGER_HAPPY2		0x2c1
#define BTN_TRIGGER_HAPPY3		0x2c2
#define ABS_MT_TOOL_X		0x3c	/* Center X tool position */
#define ABS_MT_TOOL_Y		0x3d	/* Center Y tool position */

/* Drums and guitars (mostly toys) */
#define ABS_TOM_FAR_LEFT	0x40
#define ABS_TOM_LEFT		0x41
#define ABS_TOM_RIGHT		0x42
#define ABS_TOM_FAR_RIGHT	0x43
#define ABS_CYMBAL_FAR_LEFT	0x44
#define ABS_CYMBAL_LEFT		0x45
#define ABS_CYMBAL_RIGHT	0x46
#define ABS_CYMBAL_FAR_RIGHT	0x47
#define ABS_BASS		0x48
#define ABS_HI_HAT		0x49
#define ABS_FRET_BOARD		0x4a	/* Guitar fret board, vertical pos */
#define ABS_WHAMMY_BAR		0x4b	/* Guitar whammy bar (or vibrato) */

#define ABS_MAX			0x4f
#define ABS_CNT			(ABS_MAX+1)

/*
 * Switch events