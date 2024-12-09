#include <asm/traps.h>
#include <asm/proto.h>
#include <asm/desc.h>

struct idt_data {
	unsigned int	vector;
	unsigned int	segment;