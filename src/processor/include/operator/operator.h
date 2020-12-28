#pragma once

#include <exception>
#include <memory>

#include "src/processor/include/operator/tuple/data_chunks.h"
#include "src/processor/include/task_system/morsel.h"
#include "src/storage/include/graph.h"

using namespace std;
using namespace graphflow::storage;

namespace graphflow {
namespace processor {

class Operator {

public:
    Operator() { dataChunks = make_shared<DataChunks>(); }

    virtual bool hasNextMorsel() { return prevOperator->hasNextMorsel(); }
    // Warning: getNextTuples() should only be called if getNextMorsel() returns true.
    virtual void getNextTuples() = 0;
    virtual void initialize(Graph* graph, shared_ptr<MorselDesc>& morsel) = 0;

    shared_ptr<DataChunks> getOutDataChunks() { return dataChunks; }

    void setPrevOperator(Operator* prevOperator) {
        this->prevOperator = unique_ptr<Operator>(prevOperator);
    }

    virtual void cleanup() { prevOperator->cleanup(); }

protected:
    shared_ptr<DataChunks> dataChunks;
    unique_ptr<Operator> prevOperator;
};

} // namespace processor
} // namespace graphflow