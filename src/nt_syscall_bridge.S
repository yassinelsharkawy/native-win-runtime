/*
 * native-win-runtime
 * Windows x64 syscall bridge
 */

.intel_syntax noprefix
.text

.equ SHADOW_SPACE, 32

.global NtSyscallBridge
NtSyscallBridge:
        mov     r11, rdx            # r11 = syscallRetGadget

        mov     eax, ecx            # eax = syscallNumber

        mov     r10, r8             # arg1 -> r10
        mov     rcx, r8             # arg1 -> rcx
        mov     rdx, r9             # arg2 -> rdx
        mov     r8,  [rsp+0x28]     # arg3 -> r8
        mov     r9,  [rsp+0x30]     # arg4 -> r9

        add     rsp, 0x18
        call    r11
        sub     rsp, 0x18

        ret
