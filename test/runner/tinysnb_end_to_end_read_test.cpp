#include <filesystem>

#include "test/test_utility/include/test_helper.h"

using ::testing::Test;
using namespace graphflow::testing;

class TinySnbReadTest : public BaseGraphLoadingTest {

public:
    string getInputCSVDir() override { return "dataset/tinysnb/"; }

    void SetUp() override {
        BaseGraphLoadingTest::SetUp();
        systemConfig->largePageBufferPoolSize = (1ull << 23);
        createDBAndConn();
    }
};

TEST_F(TinySnbReadTest, StructuralQueries) {
    vector<TestQueryConfig> queryConfigs;
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/structural/nodes.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/structural/paths.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/structural/stars.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/structural/multi_query.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
}

TEST_F(TinySnbReadTest, FilteredQueries) {
    vector<TestQueryConfig> queryConfigs;
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/filtered/id_comparison.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/filtered/nodes.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/filtered/null.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/filtered/paths.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/filtered/stars.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/filtered/str_operations.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
    queryConfigs =
        TestHelper::parseTestFile("test/runner/queries/filtered/unstructured_properties.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/filtered/multi_query.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
}

TEST_F(TinySnbReadTest, DateDataTypeTests) {
    vector<TestQueryConfig> queryConfigs;
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/data_types/date_data_type.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
}

TEST_F(TinySnbReadTest, TimestampDataTypeTests) {
    vector<TestQueryConfig> queryConfigs;
    queryConfigs =
        TestHelper::parseTestFile("test/runner/queries/data_types/timestamp_data_type.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
}

TEST_F(TinySnbReadTest, IntervalDataTypeTests) {
    vector<TestQueryConfig> queryConfigs;
    queryConfigs =
        TestHelper::parseTestFile("test/runner/queries/data_types/interval_data_type.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
}

TEST_F(TinySnbReadTest, ListDataTypeTests) {
    vector<TestQueryConfig> queryConfigs;
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/data_types/list_data_type.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
}

TEST_F(TinySnbReadTest, AggregateTests) {
    vector<TestQueryConfig> queryConfigs;
    queryConfigs =
        TestHelper::parseTestFile("test/runner/queries/aggregate/distinct_aggregate.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/aggregate/distinct.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/aggregate/hash_aggregate.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/aggregate/simple_aggregate.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/aggregate/multi_query.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
}

TEST_F(TinySnbReadTest, FunctionTests) {
    vector<TestQueryConfig> queryConfigs;
    queryConfigs =
        TestHelper::parseTestFile("test/runner/queries/functions/boolean_functions.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/functions/list_functions.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/functions/string_functions.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
    queryConfigs =
        TestHelper::parseTestFile("test/runner/queries/functions/arithmetic_functions.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/functions/date_functions.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
    queryConfigs =
        TestHelper::parseTestFile("test/runner/queries/functions/timestamp_functions.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
    queryConfigs =
        TestHelper::parseTestFile("test/runner/queries/functions/interval_functions.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/functions/cast_functions.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
}

TEST_F(TinySnbReadTest, ProjectionTests) {
    vector<TestQueryConfig> queryConfigs;
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/projection/projection.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/projection/with.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/projection/skip_limit.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
}

TEST_F(TinySnbReadTest, SubqueryTests) {
    vector<TestQueryConfig> queryConfigs;
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/subquery/subquery.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
}

TEST_F(TinySnbReadTest, OptionalMatchTests) {
    vector<TestQueryConfig> queryConfigs;
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/optional/optional_match.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
}

TEST_F(TinySnbReadTest, OrderByTests) {
    vector<TestQueryConfig> queryConfigs;
    queryConfigs = TestHelper::parseTestFile(
        "test/runner/queries/order_by/order_by_tiny_snb.test", true /* checkOutputOrder */);
    for (auto& queryConfig : queryConfigs) {
        queryConfig.checkOutputOrder = true;
    }
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
}

TEST_F(TinySnbReadTest, UnionAllTests) {
    vector<TestQueryConfig> queryConfigs;
    queryConfigs = TestHelper::parseTestFile("test/runner/queries/union/union_tiny_snb.test");
    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
}

// TODO(Ziyi): variable length join consumes way too much memory than it should.
// TEST_F(TinySnbReadTest, VarLengthAdjListExtendTests) {
//    vector<TestQueryConfig> queryConfigs;
//    queryConfigs = TestHelper::parseTestFile(
//        "test/runner/queries/var_length_extend/var_length_adj_list_extend.test");
//    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
//    queryConfigs = TestHelper::parseTestFile(
//        "test/runner/queries/var_length_extend/var_length_column_extend.test");
//    ASSERT_TRUE(TestHelper::runTest(queryConfigs, *conn));
//}