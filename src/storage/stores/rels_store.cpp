#include "src/storage/include/stores/rels_store.h"

#include "src/common/include/compression_scheme.h"

namespace graphflow {
namespace storage {

RelsStore::RelsStore(const Catalog& catalog, const vector<uint64_t>& numNodesPerLabel,
    const string& directory, BufferManager& bufferManager) {
    initAdjListsAndColumns(catalog, numNodesPerLabel, directory, bufferManager);
    initPropertyListsAndColumns(catalog, numNodesPerLabel, directory, bufferManager);
}

void RelsStore::initPropertyListsAndColumns(const Catalog& catalog,
    const vector<uint64_t>& numNodesPerLabel, const string& directory,
    BufferManager& bufferManager) {
    propertyColumns.resize(catalog.getNodeLabelsCount());
    propertyLists[FWD].resize(catalog.getNodeLabelsCount());
    propertyLists[BWD].resize(catalog.getNodeLabelsCount());
    for (auto nodeLabel = 0u; nodeLabel < catalog.getNodeLabelsCount(); nodeLabel++) {
        propertyColumns[nodeLabel].resize(catalog.getRelLabelsCount());
        propertyLists[FWD][nodeLabel].resize(catalog.getRelLabelsCount());
        propertyLists[BWD][nodeLabel].resize(catalog.getRelLabelsCount());
    }
    for (auto relLabel = 0u; catalog.getRelLabelsCount(); relLabel++) {
        if (catalog.isSingleCaridinalityInDir(relLabel, FWD)) {
            initPropertyColumnsForRelLabel(
                catalog, numNodesPerLabel, directory, bufferManager, relLabel, FWD);
        } else if (catalog.isSingleCaridinalityInDir(relLabel, BWD)) {
            initPropertyColumnsForRelLabel(
                catalog, numNodesPerLabel, directory, bufferManager, relLabel, BWD);
        } else {
            initPropertyListsForRelLabel(
                catalog, numNodesPerLabel, directory, bufferManager, relLabel);
        }
    }
}

void RelsStore::initPropertyColumnsForRelLabel(const Catalog& catalog,
    const vector<uint64_t>& numNodesPerLabel, const string& directory, BufferManager& bufferManager,
    const label_t& relLabel, const Direction& dir) {
    for (auto& nodeLabel : catalog.getNodeLabelsForRelLabelDir(relLabel, dir)) {
        auto& propertyMap = catalog.getPropertyMapForRelLabel(relLabel);
        propertyColumns[nodeLabel][relLabel].resize(propertyMap.size());
        for (auto property = propertyMap.begin(); property != propertyMap.end(); property++) {
            auto idx = property->second.idx;
            auto fname = getRelPropertyColumnFname(directory, relLabel, nodeLabel, property->first);
            switch (property->second.dataType) {
            case INT:
                propertyColumns[nodeLabel][relLabel][idx] = make_unique<PropertyColumnInt>(
                    fname, numNodesPerLabel[nodeLabel], bufferManager);
                break;
            case DOUBLE:
                propertyColumns[nodeLabel][relLabel][idx] = make_unique<PropertyColumnDouble>(
                    fname, numNodesPerLabel[nodeLabel], bufferManager);
                break;
            case BOOL:
                propertyColumns[nodeLabel][relLabel][idx] = make_unique<PropertyColumnBool>(
                    fname, numNodesPerLabel[nodeLabel], bufferManager);
                break;
            case STRING:
                propertyColumns[nodeLabel][relLabel][idx] = make_unique<PropertyColumnString>(
                    fname, numNodesPerLabel[nodeLabel], bufferManager);
                break;
            default:
                throw invalid_argument("invalid type for property list creation.");
            }
        }
    }
}

void RelsStore::initPropertyListsForRelLabel(const Catalog& catalog,
    const vector<uint64_t>& numNodesPerLabel, const string& directory, BufferManager& bufferManager,
    const label_t& relLabel) {
    for (auto& dir : DIRS) {
        for (auto& nodeLabel : catalog.getNodeLabelsForRelLabelDir(relLabel, dir)) {
            auto& propertyMap = catalog.getPropertyMapForRelLabel(relLabel);
            propertyLists[dir][nodeLabel][relLabel].resize(propertyMap.size());
            auto adjListsHeaders = adjLists[dir][nodeLabel][relLabel]->getHeaders();
            for (auto property = propertyMap.begin(); property != propertyMap.end(); property++) {
                auto fname =
                    getRelPropertyListsFname(directory, relLabel, nodeLabel, dir, property->first);
                auto idx = property->second.idx;
                switch (property->second.dataType) {
                case INT:
                    propertyLists[dir][nodeLabel][relLabel][idx] =
                        make_unique<RelPropertyListsInt>(fname, adjListsHeaders, bufferManager);
                    break;
                case DOUBLE:
                    propertyLists[dir][nodeLabel][relLabel][idx] =
                        make_unique<RelPropertyListsDouble>(fname, adjListsHeaders, bufferManager);
                    break;
                case BOOL:
                    propertyLists[dir][nodeLabel][relLabel][idx] =
                        make_unique<RelPropertyListsBool>(fname, adjListsHeaders, bufferManager);
                    break;
                case STRING:
                    propertyLists[dir][nodeLabel][relLabel][idx] =
                        make_unique<RelPropertyListsString>(fname, adjListsHeaders, bufferManager);
                default:
                    throw invalid_argument("invalid type for property list creation.");
                }
            }
        }
    }
}

void RelsStore::initAdjListsAndColumns(const Catalog& catalog,
    const vector<uint64_t>& numNodesPerLabel, const string& directory,
    BufferManager& bufferManager) {
    for (auto direction : DIRS) {
        adjColumns[direction].resize(catalog.getNodeLabelsCount());
        adjLists[direction].resize(catalog.getNodeLabelsCount());
        for (auto nodeLabel = 0u; nodeLabel < catalog.getNodeLabelsCount(); nodeLabel++) {
            adjColumns[direction][nodeLabel].resize(catalog.getRelLabelsCount());
            adjLists[direction][nodeLabel].resize(catalog.getRelLabelsCount());
            for (auto relLabel : catalog.getRelLabelsForNodeLabelDirection(nodeLabel, direction)) {
                NodeIDCompressionScheme nodeIDCompression{
                    catalog.getNodeLabelsForRelLabelDir(relLabel, !direction), numNodesPerLabel,
                    catalog.getNodeLabelsCount()};
                if (catalog.isSingleCaridinalityInDir(relLabel, direction)) {
                    auto fname = getAdjColumnIndexFname(directory, nodeLabel, relLabel, direction);
                    adjColumns[direction][nodeLabel][relLabel] = make_unique<AdjColumn>(
                        fname, numNodesPerLabel[nodeLabel], bufferManager, nodeIDCompression);
                } else {
                    auto fname = getAdjListsIndexFname(directory, nodeLabel, relLabel, direction);
                    adjLists[direction][nodeLabel][relLabel] =
                        make_unique<AdjLists>(fname, bufferManager, nodeIDCompression);
                }
            }
        }
    }
}

} // namespace storage
} // namespace graphflow
