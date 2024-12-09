#ifndef ISCSI_PARAMETERS_H
#define ISCSI_PARAMETERS_H

struct iscsi_extra_response {
	char key[64];
	char value[32];
	struct list_head er_list;
} ____cacheline_aligned;
