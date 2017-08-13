#
# Relocatable exception vector; jumps to kernel initialisation
#
    .global kernel_trampoline
    .global kernel_trampoline_end
    .ent kernel_trampoline
kernel_trampoline:
    j           __kernel_entry_point
    nop
kernel_trampoline_end:
