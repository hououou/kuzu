#pragma once

#include "binder/bound_statement.h"
#include "common/explain_type.h"

namespace kuzu {
namespace binder {

class BoundExplain : public BoundStatement {
public:
    explicit BoundExplain(
        std::unique_ptr<BoundStatement> statementToExplain, common::ExplainType explainType)
        : BoundStatement{common::StatementType::EXPLAIN,
              BoundStatementResult::createSingleStringColumnResult()},
          statementToExplain{std::move(statementToExplain)}, explainType{explainType} {}

    inline BoundStatement* getStatementToExplain() const { return statementToExplain.get(); }

    inline common::ExplainType getExplainType() const { return explainType; }

private:
    std::unique_ptr<BoundStatement> statementToExplain;
    common::ExplainType explainType;
};

} // namespace binder
} // namespace kuzu
