#ifndef ark_vm_types
#define ark_vm_types

#include <cinttypes>

namespace Ark
{
    namespace VM
    {
        enum class NFT { Nil, False, True };
        using PageAddr_t = uint16_t;
    }
}

#endif