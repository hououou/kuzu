#pragma once

#include "src/processor/include/physical_plan/operator/filtering_operator.h"
#include "src/processor/include/physical_plan/operator/physical_operator.h"

namespace graphflow {
namespace processor {

class Skip : public PhysicalOperator, public FilteringOperator {

public:
    Skip(uint64_t skipNumber, shared_ptr<atomic_uint64_t> counter, uint32_t dataChunkToSelectPos,
        vector<uint32_t> dataChunksToSkipPos, unique_ptr<PhysicalOperator> prevOperator,
        ExecutionContext& context, uint32_t id)
        : PhysicalOperator{move(prevOperator), SKIP, context, id},
          FilteringOperator(), skipNumber{skipNumber}, counter{move(counter)},
          dataChunkToSelectPos{dataChunkToSelectPos}, dataChunksToSkipPos{
                                                          move(dataChunksToSkipPos)} {}

    shared_ptr<ResultSet> initResultSet() override;

    bool getNextTuples() override;

    unique_ptr<PhysicalOperator> clone() override {
        return make_unique<Skip>(skipNumber, counter, dataChunkToSelectPos, dataChunksToSkipPos,
            prevOperator->clone(), context, id);
    }

private:
    uint64_t skipNumber;
    shared_ptr<atomic_uint64_t> counter;
    uint32_t dataChunkToSelectPos;
    vector<uint32_t> dataChunksToSkipPos;
};

} // namespace processor
} // namespace graphflow