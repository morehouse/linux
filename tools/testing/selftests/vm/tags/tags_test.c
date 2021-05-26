// SPDX-License-Identifier: GPL-2.0

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/prctl.h>
#include <sys/utsname.h>

static int tag_bits;
static int tag_offset;

#define SHIFT_TAG(tag)		((uint64_t)(tag) << tag_offset)
#define SET_TAG(ptr, tag)	(((uint64_t)(ptr) & ~SHIFT_TAG((1 << tag_bits) - 1)) | SHIFT_TAG(tag))

static int max_tag_bits(void)
{
	int nr;

	if (prctl(PR_GET_TAGGED_ADDR_CTRL, 0, 0, 0) < 0)
		return 0;

	if (prctl(PR_GET_TAGGED_ADDR_CTRL, &nr, 0, 0) < 0)
		return 8; /* Assume ARM TBI */

	return nr;
}

int main(void)
{
	static int tags_enabled = 0;
	unsigned long tag = 0;
	struct utsname *ptr;
	int err;

	tag_bits = max_tag_bits();

	if (tag_bits && !prctl(PR_SET_TAGGED_ADDR_CTRL, PR_TAGGED_ADDR_ENABLE,
			       &tag_bits, &tag_offset, 0)) {
		tags_enabled = 1;
	} else if (tag_bits == 8 && !prctl(PR_SET_TAGGED_ADDR_CTRL,
					   PR_TAGGED_ADDR_ENABLE, 0, 0)) {
		/* ARM TBI with legacy interface*/
		tags_enabled = 1;
		tag_offset = 56;
	}

	ptr = (struct utsname *)malloc(sizeof(*ptr));
	if (tags_enabled)
		tag = (1UL << tag_bits) - 1;
	ptr = (struct utsname *)SET_TAG(ptr, tag);
	err = uname(ptr);
	printf("Sysname: %s\n", ptr->sysname);
	free(ptr);

	return err;
}
