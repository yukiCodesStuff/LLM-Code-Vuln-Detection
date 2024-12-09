#ifndef ISCSI_PARAMETERS_H
#define ISCSI_PARAMETERS_H

#include <scsi/iscsi_proto.h>

struct iscsi_extra_response {
	char key[KEY_MAXLEN];
	char value[32];
	struct list_head er_list;
} ____cacheline_aligned;
