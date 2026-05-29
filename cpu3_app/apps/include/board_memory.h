#ifndef _BOARD_MEMORY_H
#define _BOARD_MEMORY_H


// mx6dq has 256kB of OCRAM
#define OCRAM_LEN 256K

/*
 * External DDR
 */
// mx6dq and mx6sdl DDR memory starts at this address
#define DDR_ORG 0x78000000

#define DDR_LEN 128M

// Maximum size of the signed image
#define MAX_IMAGE_SIZE 10M

// Maximum size of HAB CSF data
#define MAX_HAB_CSF_DATA_SIZE 8K

// Size of stacks section
#define HEAP_SIZE 16M

#define SYSTEM_STACK_SIZE           2048
#define ABORT_STACK_SIZE            2048
#define SUPERVISOR_STACK_SIZE       0xA00000
#define IRQ_STACK_SIZE              2048
#define FIQ_STACK_SIZE              2048
#define UNDEF_STACK_SIZE            2048

// Size of the L1 page table.
#define MMU_TBL_SIZE 16K

// Size of the RAM vectors table at the top of OCRAM.
//
// The vectors in ROM at address 0 point to these RAM vectors.
#define RAM_VECTORS_SIZE 72

#endif
