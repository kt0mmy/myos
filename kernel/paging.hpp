#pragma once 
#include <cstddef>

// 2MiB x 512 * 64 = 64GiBの仮想アドレスがマッピングされる
const size_t kPageDirectoryCount = 64;

void SetupIdentityPageTable();