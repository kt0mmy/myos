#include "paging.hpp"
#include "asmfunc.h"
#include <array>
#include <cstdint>

namespace
{
    const uint64_t kPageSize4K = 4096; // 4KiB
    const uint64_t kPageSize2M = 512 * kPageSize4K;
    const uint64_t kPageSize1G = 512 * kPageSize2M;

    // 4KiBでアラインすることで、ページをフルに使える(かつ、0-11はフラグでもこれで問題ない)
    // 12-51 ビットが物理フレームのアドレス。残りはフラグ
    alignas(kPageSize4K) std::array<uint64_t, 512> pml4_table;
    alignas(kPageSize4K) std::array<uint64_t, 512> pdp_table;
    alignas(kPageSize4K) std::array<std::array<uint64_t, 512>, kPageDirectoryCount> page_directory;
}

void SetupIdentityPageTable() {
    pml4_table[0] = reinterpret_cast<uint64_t>(&pdp_table[0]) | 0x003; // present, writable
    for (int i_pdpt=0;i_pdpt<page_directory.size();i_pdpt++) {
        pdp_table[i_pdpt] = reinterpret_cast<uint64_t>(&page_directory[i_pdpt]) | 0x003;
        for (int i_pd=0;i_pd<512;i_pd++) {
            page_directory[i_pdpt][i_pd] = i_pdpt * kPageSize1G + i_pd * kPageSize2M | 0x083; // 2MiBページ、identity mapping
        }
    }
    SetCR3(reinterpret_cast<uint64_t>(&pml4_table[0]));
}