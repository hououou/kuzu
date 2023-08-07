#include "binder/query/updating_clause/bound_delete_clause.h"
#include "planner/logical_plan/persistent/logical_delete.h"
#include "planner/query_planner.h"

namespace kuzu {
namespace planner {

void QueryPlanner::appendDeleteNode(
    const std::vector<binder::BoundDeleteInfo*>& boundInfos, LogicalPlan& plan) {
    std::vector<std::unique_ptr<LogicalDeleteNodeInfo>> logicalInfos;
    for (auto& boundInfo : boundInfos) {
        auto node = std::static_pointer_cast<NodeExpression>(boundInfo->nodeOrRel);
        logicalInfos.push_back(std::make_unique<LogicalDeleteNodeInfo>(node));
    }
    auto deleteNode =
        std::make_shared<LogicalDeleteNode>(std::move(logicalInfos), plan.getLastOperator());
    deleteNode->computeFactorizedSchema();
    plan.setLastOperator(std::move(deleteNode));
}

void QueryPlanner::appendDeleteRel(
    const std::vector<binder::BoundDeleteInfo*>& boundInfos, LogicalPlan& plan) {
    std::vector<std::shared_ptr<RelExpression>> rels;
    for (auto& info : boundInfos) {
        auto rel = std::static_pointer_cast<RelExpression>(info->nodeOrRel);
        rels.push_back(rel);
    }
    auto deleteRel = std::make_shared<LogicalDeleteRel>(rels, plan.getLastOperator());
    for (auto i = 0u; i < boundInfos.size(); ++i) {
        appendFlattens(deleteRel->getGroupsPosToFlatten(i), plan);
        deleteRel->setChild(0, plan.getLastOperator());
    }
    deleteRel->computeFactorizedSchema();
    plan.setLastOperator(std::move(deleteRel));
}

} // namespace planner
} // namespace kuzu