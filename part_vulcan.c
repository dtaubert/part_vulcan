/*
 * Copyright (c) 2017 Derek Taubert
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <linux/fs.h>

static struct stat sb;

#define BLCK_SIZE 512
#define NUM_PARTITIONS 16

static struct {
    uint8_t magic[2];
    uint8_t cksum[2];
    uint8_t foo1[25];
    uint8_t empty1[3 + (9*16) + 7];
    uint8_t foo2[2];
    uint8_t empty2[7 + (4*16)];
    struct {
	uint8_t start[3];
	uint8_t size[2];
	uint8_t foo[1];
	uint8_t name[10];
    } part[NUM_PARTITIONS];
} ptable = {
    { 0xae, 0xae },
    { 0, 0 },
    { 0x00, 0x80, 0x3d, 0x00, 0x00, 0xfb, 0x00, 0x02, 0x20, 0x01, 0x00,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
    { 0 },
    { 0x00, 0x01 },
    { 0 },
    { {
	{ 0, 0, 0 },
	{ 0, 0 },
	{ 0 },
	{ 0xc1, 0xc5, 0xb1, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0 }
    },
    {
	{ 0, 0, 0 },
	{ 0, 0 },
	{ 0 },
	{ 0xc1, 0xc5, 0xb2, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0 }
    },
    {
	{ 0, 0, 0 },
	{ 0, 0 },
	{ 0 },
	{ 0xc1, 0xc5, 0xb3, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0 }
    },
    {
	{ 0, 0, 0 },
	{ 0, 0 },
	{ 0 },
	{ 0xc1, 0xc5, 0xb4, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0 }
    },
    {
	{ 0, 0, 0 },
	{ 0, 0 },
	{ 0 },
	{ 0xc1, 0xc5, 0xb5, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0 }
    },
    {
	{ 0, 0, 0 },
	{ 0, 0 },
	{ 0 },
	{ 0xc1, 0xc5, 0xb6, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0 }
    },
    {
	{ 0, 0, 0 },
	{ 0, 0 },
	{ 0 },
	{ 0xc1, 0xc5, 0xb7, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0 }
    },
    {
	{ 0, 0, 0 },
	{ 0, 0 },
	{ 0 },
	{ 0xc1, 0xc5, 0xb8, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0 }
    },
    {
	{ 0, 0, 0 },
	{ 0, 0 },
	{ 0 },
	{ 0xc1, 0xc5, 0xb9, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0 }
    },
    {
	{ 0, 0, 0 },
	{ 0, 0 },
	{ 0 },
	{ 0xc1, 0xc5, 0xb1, 0xb0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0 }
    },
    {
	{ 0, 0, 0 },
	{ 0, 0 },
	{ 0 },
	{ 0xc1, 0xc5, 0xb1, 0xb1, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0 }
    },
    {
	{ 0, 0, 0 },
	{ 0, 0 },
	{ 0 },
	{ 0xc1, 0xc5, 0xb1, 0xb2, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0 }
    },
    {
	{ 0, 0, 0 },
	{ 0, 0 },
	{ 0 },
	{ 0xc1, 0xc5, 0xb1, 0xb3, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0 }
    },
    {
	{ 0, 0, 0 },
	{ 0, 0 },
	{ 0 },
	{ 0xc1, 0xc5, 0xb1, 0xb4, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0 }
    },
    {
	{ 0, 0, 0 },
	{ 0, 0 },
	{ 0 },
	{ 0xc1, 0xc5, 0xb1, 0xb5, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0 }
    },
    {
	{ 0, 0, 0 },
	{ 0, 0 },
	{ 0 },
	{ 0xc1, 0xc5, 0xb1, 0xb6, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0 }
    } }
};

#define ASSIGN(_arr, _val) { \
    typeof(_val) _tmp = (_val); \
    int _b; \
    for (_b = 0; _b<sizeof(_arr); _b++) { \
	(_arr)[_b] = _tmp & 0xff; \
	_tmp >>= 8; \
    } \
}

static void
write_partition_table (int fd)
{
    uint8_t *buffer = (uint8_t *)&ptable;

    /* Compute and update checksum */
    uint8_t cksum[2] = { 0, 0 };
    int i;
    for (i=0; i<BLCK_SIZE; i+=2) {
	cksum[0] ^= buffer[i];
	cksum[1] ^= buffer[i+1];
    }
    ptable.cksum[0] = cksum[0];
    ptable.cksum[1] = cksum[1];

    /* Write out with interleave */
    for (i=0; i<256; i+=2) {
	int j;
	for (j=i; j<BLCK_SIZE; j+=256) {
	    if (write(fd, buffer + j, 2) != 2) {
		fprintf(stderr, "write failed at %d\n", j);
		exit(2);
	    }
	}
    }
}

static void
usage (void)
{
    printf("Usage: part_vulcan <device>\n");
}

int
main (int argc, char *argv[])
{
    if (argc != 2) {
	usage();
	return (1);
    }

    const char *fname = argv[1];
    int fd = open(fname, O_RDWR);
    if (fd < 0) {
	fprintf(stderr, "open(%s) failed: %s\n", fname, strerror(errno));
	return (2);
    }

    if (fstat(fd, &sb)) {
	fprintf(stderr, "stat failed: %s\n", strerror(errno));
	return (3);
    }

    size_t fs_bytes;
    if (S_ISBLK(sb.st_mode)) {
	if (ioctl(fd, BLKGETSIZE64, &fs_bytes)) {
	    fprintf(stderr, "ioctl failed: %s\n", strerror(errno));
	    return (4);
	}
    } else {
	fs_bytes = sb.st_size;
    }
    size_t fs_blocks = fs_bytes / BLCK_SIZE;
    printf("%s: Blocks = %lld\n", fname, (long long)fs_blocks);

    uint32_t start = 1;
    int i;
    for (i=0; i<NUM_PARTITIONS; i++) {
	ASSIGN(ptable.part[i].start, start);
	uint16_t size = 0xffff;
	if (start + size > fs_blocks) {
	    size = fs_blocks - start;
	}
	ASSIGN(ptable.part[i].size, size);
	start += size;
    }

    write_partition_table(fd);
    return (0);
}
