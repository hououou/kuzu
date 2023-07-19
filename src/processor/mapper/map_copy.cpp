#include "planner/logical_plan/logical_operator/logical_copy.h"
#include "processor/mapper/plan_mapper.h"
#include "processor/operator/copy/copy_node.h"
#include "processor/operator/copy/copy_npy_node.h"
#include "processor/operator/copy/copy_rel.h"
#include "processor/operator/copy/read_csv.h"
#include "processor/operator/copy/read_file.h"
#include "processor/operator/copy/read_npy.h"
#include "processor/operator/copy/read_parquet.h"
#include "processor/operator/table_scan/factorized_table_scan.h"

using namespace kuzu::planner;

namespace kuzu {
namespace processor {

std::unique_ptr<PhysicalOperator> PlanMapper::mapLogicalCopyToPhysical(
    LogicalOperator* logicalOperator) {
    auto copy = (LogicalCopy*)logicalOperator;
    auto tableName = catalog->getReadOnlyVersion()->getTableName(copy->getTableID());
    if (catalog->getReadOnlyVersion()->containNodeTable(tableName)) {
        return mapLogicalCopyNodeToPhysical(copy);
    } else {
        return mapLogicalCopyRelToPhysical(copy);
    }
}

std::unique_ptr<PhysicalOperator> PlanMapper::mapLogicalCopyNodeToPhysical(LogicalCopy* copy) {
    auto fileType = copy->getCopyDescription().fileType;
    if (fileType != common::CopyDescription::FileType::CSV &&
        fileType != common::CopyDescription::FileType::PARQUET &&
        fileType != common::CopyDescription::FileType::NPY) {
        throw common::NotImplementedException{"PlanMapper::mapLogicalCopyToPhysical"};
    }
    std::unique_ptr<ReadFile> readFile;
    std::shared_ptr<ReadFileSharedState> readFileSharedState;
    auto outSchema = copy->getSchema();
    auto arrowColumnExpressions = copy->getArrowColumnExpressions();
    std::vector<DataPos> arrowColumnPoses;
    arrowColumnPoses.reserve(arrowColumnExpressions.size());
    for (auto& arrowColumnPos : arrowColumnExpressions) {
        arrowColumnPoses.emplace_back(outSchema->getExpressionPos(*arrowColumnPos));
    }
    auto rowIdxVectorPos = DataPos(outSchema->getExpressionPos(*copy->getRowIdxExpression()));
    auto filePathVectorPos = DataPos(outSchema->getExpressionPos(*copy->getFilePathExpression()));
    auto columnIdxPos = DataPos(outSchema->getExpressionPos(*copy->getColumnIdxExpression()));
    auto nodeTableSchema = catalog->getReadOnlyVersion()->getNodeTableSchema(copy->getTableID());
    switch (copy->getCopyDescription().fileType) {
    case (common::CopyDescription::FileType::CSV): {
        readFileSharedState =
            std::make_shared<ReadCSVSharedState>(*copy->getCopyDescription().csvReaderConfig,
                copy->getCopyDescription().filePaths, nodeTableSchema);
        readFile = std::make_unique<ReadCSV>(rowIdxVectorPos, filePathVectorPos, arrowColumnPoses,
            readFileSharedState, getOperatorID(), copy->getExpressionsForPrinting());
    } break;
    case (common::CopyDescription::FileType::PARQUET): {
        readFileSharedState = std::make_shared<ReadParquetSharedState>(
            copy->getCopyDescription().filePaths, nodeTableSchema);
        readFile =
            std::make_unique<ReadParquet>(rowIdxVectorPos, filePathVectorPos, arrowColumnPoses,
                readFileSharedState, getOperatorID(), copy->getExpressionsForPrinting());
    } break;
    case (common::CopyDescription::FileType::NPY): {
        readFileSharedState = std::make_shared<ReadNPYSharedState>(
            nodeTableSchema, copy->getCopyDescription().filePaths);
        readFile = std::make_unique<ReadNPY>(rowIdxVectorPos, filePathVectorPos, arrowColumnPoses,
            columnIdxPos, readFileSharedState, getOperatorID(), copy->getExpressionsForPrinting());
    } break;
    default:
        throw common::NotImplementedException("PlanMapper::mapLogicalCopyNodeToPhysical");
    }
    auto copyNodeSharedState =
        std::make_shared<CopyNodeSharedState>(readFileSharedState->numRows, memoryManager);
    auto outputExpression = copy->getOutputExpression();
    auto outputVectorPos = DataPos(outSchema->getExpressionPos(*outputExpression));
    auto ftSharedState = std::make_shared<FTableSharedState>(
        copyNodeSharedState->table, common::DEFAULT_VECTOR_CAPACITY);
    std::unique_ptr<CopyNode> copyNode;
    CopyNodeDataInfo copyNodeDataInfo{rowIdxVectorPos, filePathVectorPos, arrowColumnPoses};
    if (copy->getCopyDescription().fileType == common::CopyDescription::FileType::NPY) {
        copyNode = std::make_unique<CopyNPYNode>(copyNodeSharedState, copyNodeDataInfo,
            columnIdxPos, copy->getCopyDescription(),
            storageManager.getNodesStore().getNodeTable(copy->getTableID()),
            &storageManager.getRelsStore(), catalog, storageManager.getWAL(),
            std::make_unique<ResultSetDescriptor>(copy->getSchema()), std::move(readFile),
            getOperatorID(), copy->getExpressionsForPrinting());
    } else {
        copyNode = std::make_unique<CopyNode>(copyNodeSharedState, copyNodeDataInfo,
            copy->getCopyDescription(),
            storageManager.getNodesStore().getNodeTable(copy->getTableID()),
            &storageManager.getRelsStore(), catalog, storageManager.getWAL(),
            std::make_unique<ResultSetDescriptor>(copy->getSchema()), std::move(readFile),
            getOperatorID(), copy->getExpressionsForPrinting());
    }
    // We need to create another pipeline to return the copy message to the user.
    // The new pipeline only contains a factorizedTableScan and a resultCollector.
    return std::make_unique<FactorizedTableScan>(std::vector<DataPos>{outputVectorPos},
        std::vector<uint32_t>{0} /* colIndicesToScan */, ftSharedState, std::move(copyNode),
        getOperatorID(), copy->getExpressionsForPrinting());
}

std::unique_ptr<PhysicalOperator> PlanMapper::mapLogicalCopyRelToPhysical(LogicalCopy* copy) {
    auto relsStatistics = &storageManager.getRelsStore().getRelsStatistics();
    auto table = storageManager.getRelsStore().getRelTable(copy->getTableID());
    return std::make_unique<CopyRel>(catalog, copy->getCopyDescription(), table,
        storageManager.getWAL(), relsStatistics, storageManager.getNodesStore(), getOperatorID(),
        copy->getExpressionsForPrinting());
}

} // namespace processor
} // namespace kuzu