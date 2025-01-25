    .arm
    .global     _tx_thread_system_stack_ptr
    .global     _tx_initialize_unused_memory
    .global     _end
    .global     _system_stack
    .global      _system_stack_end

/* Define the 16-bit Thumb mode veneer for _tx_initialize_low_level for
   applications calling this function from to 16-bit Thumb mode.  */

    .text
    .align 2
    .thumb
    .global $_tx_initialize_low_level
    .type   $_tx_initialize_low_level,function
$_tx_initialize_low_level:
     BX        pc                               // Switch to 32-bit mode
     NOP                                        //
    .arm
     STMFD     sp!, {lr}                        // Save return address
     BL        _tx_initialize_low_level         // Call _tx_initialize_low_level function
     LDMFD     sp!, {lr}                        // Recover saved return address
     BX        lr                               // Return to 16-bit caller

    .text
    .align 2

    .global _tx_initialize_low_level
    .type   _tx_initialize_low_level,function
    .cfi_startproc
_tx_initialize_low_level:
    LDR     r1, =_system_stack                  // Get pointer to stack area
    LDR     r2, =_tx_thread_system_stack_ptr    // Pickup stack pointer
    STR     r1, [r2]                            // Save the system stack
    
    LDR     r1, =_end                           // Get end of non-initialized RAM area
    LDR     r2, =_tx_initialize_unused_memory   // Pickup unused memory ptr address
    ADD     r1, r1, #8                          // Increment to next free word
    STR     r1, [r2]                            // Save first free memory address

    bx lr
    .cfi_endproc
