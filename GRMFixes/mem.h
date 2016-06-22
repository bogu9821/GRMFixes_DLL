#pragma once

#define THISPTR_OFFSET(x) (((char *)this) + (x))

// -- call macro from GothicX (thx, Zerxes!)
#define XCALL(uAddr)                    \
        __asm { mov esp, ebp    }       \
        __asm { pop ebp                 }       \
        __asm { mov eax, uAddr  }       \
        __asm { jmp eax                 }