
#define MAX_TCP_HEADER	(128 + MAX_HEADER)
#define MAX_TCP_OPTION_SPACE 40

/*
 * Never offer a window over 32767 without using window scaling. Some
 * poor stacks do signed 16bit maths!