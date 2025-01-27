#pragma once

#include "common/query_rel_type.h"
#include "node_pattern.h"

namespace kuzu {
namespace parser {

enum class ArrowDirection : uint8_t { LEFT = 0, RIGHT = 1, BOTH = 2 };

struct RecursiveRelPatternInfo {
    std::string lowerBound;
    std::string upperBound;
    std::string relName;
    std::unique_ptr<ParsedExpression> whereExpression;

    RecursiveRelPatternInfo(std::string lowerBound, std::string upperBound, std::string relName,
        std::unique_ptr<ParsedExpression> whereExpression)
        : lowerBound{std::move(lowerBound)}, upperBound{std::move(upperBound)},
          relName{std::move(relName)}, whereExpression{std::move(whereExpression)} {}
};

/**
 * RelationshipPattern represents "-[relName:RelTableName+]-"
 */
class RelPattern : public NodePattern {
public:
    RelPattern(std::string name, std::vector<std::string> tableNames, common::QueryRelType relType,
        ArrowDirection arrowDirection,
        std::vector<std::pair<std::string, std::unique_ptr<ParsedExpression>>> propertyKeyValPairs,
        std::unique_ptr<RecursiveRelPatternInfo> recursiveInfo)
        : NodePattern{std::move(name), std::move(tableNames), std::move(propertyKeyValPairs)},
          relType{relType}, arrowDirection{arrowDirection}, recursiveInfo{
                                                                std::move(recursiveInfo)} {}

    ~RelPattern() override = default;

    inline common::QueryRelType getRelType() const { return relType; }

    inline ArrowDirection getDirection() const { return arrowDirection; }

    inline RecursiveRelPatternInfo* getRecursiveInfo() const { return recursiveInfo.get(); }

private:
    common::QueryRelType relType;
    ArrowDirection arrowDirection;
    std::unique_ptr<RecursiveRelPatternInfo> recursiveInfo;
};

} // namespace parser
} // namespace kuzu
