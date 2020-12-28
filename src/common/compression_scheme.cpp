#include "src/common/include/compression_scheme.h"

#include <stdexcept>

namespace graphflow {
namespace common {

NodeIDCompressionScheme::NodeIDCompressionScheme(const vector<label_t>& nbrNodeLabels,
    const vector<uint64_t>& numNodesPerLabel, const uint32_t& numNodeLabels) {
    auto maxNodeOffsetToFit = 0ull;
    for (auto nodeLabel : nbrNodeLabels) {
        if (numNodesPerLabel[nodeLabel] > maxNodeOffsetToFit) {
            maxNodeOffsetToFit = numNodesPerLabel[nodeLabel];
        }
    }
    auto maxLabelToFit = 1 == nbrNodeLabels.size() ? 0 : numNodeLabels - 1;
    numBytesForLabel =
        maxLabelToFit == 0 ? 0 : getNumBytesForEncoding(maxLabelToFit, 1 /*min num bytes*/);
    numBytesForOffset = getNumBytesForEncoding(maxNodeOffsetToFit, 2 /*min num bytes*/);
    numTotalBytes = numBytesForLabel + numBytesForOffset;
}

uint32_t NodeIDCompressionScheme::getNumBytesForEncoding(
    const uint64_t& maxValToEncode, const uint8_t& minNumBytes) {
    auto numBytes = minNumBytes;
    while (maxValToEncode > (1ull << (8 * numBytes)) - 2) {
        numBytes <<= 1;
    }
    return numBytes;
}

} // namespace common
} // namespace graphflow