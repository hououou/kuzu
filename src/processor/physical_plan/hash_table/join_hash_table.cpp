#include "src/processor/include/physical_plan/hash_table/join_hash_table.h"

namespace graphflow {
namespace processor {

JoinHashTable::JoinHashTable(
    MemoryManager& memoryManager, uint64_t numTuples, TableSchema& tableSchema)
    : BaseHashTable{memoryManager} {
    maxNumHashSlots = HashTableUtils::nextPowerOfTwo(numTuples * 2);
    bitMask = maxNumHashSlots - 1;
    numHashSlotsPerBlock = LARGE_PAGE_SIZE / sizeof(uint8_t*);
    auto numBlocks =
        maxNumHashSlots / numHashSlotsPerBlock + (maxNumHashSlots % numHashSlotsPerBlock != 0);
    for (auto i = 0u; i < numBlocks; i++) {
        hashSlotsBlocks.emplace_back(make_unique<DataBlock>(&memoryManager));
    }
    factorizedTable = make_unique<FactorizedTable>(&memoryManager, tableSchema);
}

void JoinHashTable::allocateHashSlots(uint64_t numTuples) {
    maxNumHashSlots = HashTableUtils::nextPowerOfTwo(numTuples * 2);
    bitMask = maxNumHashSlots - 1;
    auto numBlocksNeeded =
        maxNumHashSlots / numHashSlotsPerBlock + (maxNumHashSlots % numHashSlotsPerBlock != 0);
    while (hashSlotsBlocks.size() < numBlocksNeeded) {
        hashSlotsBlocks.emplace_back(make_unique<DataBlock>(&memoryManager));
    }
}

} // namespace processor
} // namespace graphflow
