/*
 * Copyright 2021 4Paradigm
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sched.h>
#include <unistd.h>

#include <memory>
#include <string>
#include <vector>

#include "base/file_util.h"
#include "base/glog_wapper.h"
#include "catalog/schema_adapter.h"
#include "codec/fe_row_codec.h"
#include "common/timer.h"
#include "gflags/gflags.h"
#include "gtest/gtest.h"
#include "sdk/mini_cluster.h"
#include "sdk/sql_cluster_router.h"
#include "sdk/sql_router.h"
#include "sdk/sql_sdk_test.h"
#include "vm/catalog.h"

namespace openmldb {
namespace sdk {

::openmldb::sdk::MiniCluster* mc_;
std::shared_ptr<SQLRouter> router_;

class SQLClusterTest : public ::testing::Test {
 public:
    SQLClusterTest() {}
    ~SQLClusterTest() {}
    void SetUp() {}
    void TearDown() {}
};

class MockClosure : public ::google::protobuf::Closure {
 public:
    MockClosure() {}
    ~MockClosure() {}
    void Run() {}
};

class SQLClusterDDLTest : public SQLClusterTest {
 public:
    void SetUp() {
        SQLRouterOptions sql_opt;
        sql_opt.zk_cluster = mc_->GetZkCluster();
        sql_opt.zk_path = mc_->GetZkPath();
        router = NewClusterSQLRouter(sql_opt);
        ASSERT_TRUE(router != nullptr);
        db = "db" + GenRand();
        ::hybridse::sdk::Status status;
        ASSERT_TRUE(router->CreateDB(db, &status));
    }

    void TearDown() {
        ::hybridse::sdk::Status status;
        ASSERT_TRUE(router->DropDB(db, &status));
        router.reset();
    }

    void RightDDL(const std::string& name, const std::string& ddl) {
        ::hybridse::sdk::Status status;
        ASSERT_TRUE(router->ExecuteDDL(db, ddl, &status)) << "ddl: " << ddl;
        ASSERT_TRUE(router->ExecuteDDL(db, "drop table " + name + ";", &status));
    }

    void WrongDDL(const std::string& name, const std::string& ddl) {
        ::hybridse::sdk::Status status;
        ASSERT_FALSE(router->ExecuteDDL(db, ddl, &status)) << "ddl: " << ddl;
        ASSERT_FALSE(router->ExecuteDDL(db, "drop table " + name + ";", &status));
    }

    std::shared_ptr<SQLRouter> router;
    std::string db;
};

TEST_F(SQLClusterDDLTest, ColumnDefaultValue) {
    std::string name = "test" + GenRand();
    ::hybridse::sdk::Status status;
    std::string ddl;
    ddl = "create table " + name +
          "("
          "col1 bool default true, col2 bigint not null, col3 string,"
          "index(key=col3, ts=col2));";
    RightDDL(name, ddl);
    ddl = "create table " + name +
          "("
          "col1 bool default 12, col2 bigint not null, col3 string,"
          "index(key=col3, ts=col2));";
    RightDDL(name, ddl);
    ddl = "create table " + name +
          "("
          "col1 int default 12, col2 bigint not null, col3 string,"
          "index(key=col3, ts=col2));";
    RightDDL(name, ddl);
    ddl = "create table " + name +
          "("
          "col1 bigint default 50000000000l, col2 bigint not null, col3 string,"
          "index(key=col3, ts=col2));";
    RightDDL(name, ddl);
    ddl = "create table " + name +
          "("
          "col1 float default 123, col2 bigint not null, col3 string,"
          "index(key=col3, ts=col2));";
    RightDDL(name, ddl);
    ddl = "create table " + name +
          "("
          "col1 float default 1.23, col2 bigint not null, col3 string,"
          "index(key=col3, ts=col2));";
    RightDDL(name, ddl);
    ddl = "create table " + name +
          "("
          "col1 double default 0.1, col2 bigint not null, col3 string,"
          "index(key=col3, ts=col2));";
    RightDDL(name, ddl);
    ddl = "create table " + name +
          "("
          "col1 date default '2021-10-01', col2 bigint not null, col3 string,"
          "index(key=col3, ts=col2));";
    RightDDL(name, ddl);
    ddl = "create table " + name +
          "("
          "col1 timestamp default 1634890378, col2 bigint not null, col3 string,"
          "index(key=col3, ts=col2));";
    RightDDL(name, ddl);
}

TEST_F(SQLClusterDDLTest, ColumnDefaultValueWrongType) {
    std::string name = "test" + GenRand();
    ::hybridse::sdk::Status status;
    std::string ddl;
    ddl = "create table " + name +
          "("
          "col1 bool default test, col2 bigint not null, col3 string,"
          "index(key=col3, ts=col2));";
    WrongDDL(name, ddl);
    ddl = "create table " + name +
          "("
          "col1 int default 'test', col2 bigint not null, col3 string,"
          "index(key=col3, ts=col2));";
    WrongDDL(name, ddl);
    ddl = "create table " + name +
          "("
          "col1 int default 1.0, col2 bigint not null, col3 string,"
          "index(key=col3, ts=col2));";
    WrongDDL(name, ddl);
    ddl = "create table " + name +
          "("
          "col1 int default 1+1, col2 bigint not null, col3 string,"
          "index(key=col3, ts=col2));";
    WrongDDL(name, ddl);
    ddl = "create table " + name +
          "("
          "col1 string default 12, col2 bigint not null, col3 string,"
          "index(key=col3, ts=col2));";
    WrongDDL(name, ddl);
    ddl = "create table " + name +
          "("
          "col1 double default 'test', col2 bigint not null, col3 string,"
          "index(key=col3, ts=col2));";
    WrongDDL(name, ddl);
    ddl = "create table " + name +
          "("
          "col1 date default 'test', col2 bigint not null, col3 string,"
          "index(key=col3, ts=col2));";
    WrongDDL(name, ddl);
    ddl = "create table " + name +
          "("
          "col1 date default 1234, col2 bigint not null, col3 string,"
          "index(key=col3, ts=col2));";
    WrongDDL(name, ddl);
    ddl = "create table " + name +
          "("
          "col1 timestamp default 'test', col2 bigint not null, col3 string,"
          "index(key=col3, ts=col2));";
    WrongDDL(name, ddl);
}

TEST_F(SQLClusterTest, ClusterInsert) {
    SQLRouterOptions sql_opt;
    sql_opt.zk_cluster = mc_->GetZkCluster();
    sql_opt.zk_path = mc_->GetZkPath();
    auto router = NewClusterSQLRouter(sql_opt);
    ASSERT_TRUE(router != nullptr);
    std::string name = "test" + GenRand();
    std::string db = "db" + GenRand();
    ::hybridse::sdk::Status status;
    bool ok = router->CreateDB(db, &status);
    ASSERT_TRUE(ok);
    std::string ddl = "create table " + name +
                      "("
                      "col1 string, col2 bigint,"
                      "index(key=col1, ts=col2)) options(partitionnum=8);";
    ok = router->ExecuteDDL(db, ddl, &status);
    ASSERT_TRUE(ok);
    ASSERT_TRUE(router->RefreshCatalog());
    std::map<uint32_t, std::vector<std::string>> key_map;
    for (int i = 0; i < 100; i++) {
        std::string key = "hello" + std::to_string(i);
        std::string insert = "insert into " + name + " values('" + key + "', 1590);";
        ok = router->ExecuteInsert(db, insert, &status);
        ASSERT_TRUE(ok);
        uint32_t pid = static_cast<uint32_t>(::openmldb::base::hash64(key) % 8);
        key_map[pid].push_back(key);
    }
    auto endpoints = mc_->GetTbEndpoint();
    uint32_t count = 0;
    for (const auto& endpoint : endpoints) {
        ::openmldb::tablet::TabletImpl* tb1 = mc_->GetTablet(endpoint);
        ::openmldb::api::GetTableStatusRequest request;
        ::openmldb::api::GetTableStatusResponse response;
        MockClosure closure;
        tb1->GetTableStatus(NULL, &request, &response, &closure);
        for (const auto& table_status : response.all_table_status()) {
            count += table_status.record_cnt();
            auto iter = key_map.find(table_status.pid());
            ASSERT_EQ(iter->second.size(), table_status.record_cnt());
        }
    }
    ASSERT_EQ(100u, count);
    ok = router->ExecuteDDL(db, "drop table " + name + ";", &status);
    ASSERT_TRUE(ok);
    ok = router->DropDB(db, &status);
    ASSERT_TRUE(ok);
}

TEST_F(SQLClusterTest, ClusterInsertWithColumnDefaultValue) {
    SQLRouterOptions sql_opt;
    sql_opt.zk_cluster = mc_->GetZkCluster();
    sql_opt.zk_path = mc_->GetZkPath();
    auto router = NewClusterSQLRouter(sql_opt);
    ASSERT_TRUE(router != nullptr);
    std::string name = "test" + GenRand();
    std::string db = "db" + GenRand();
    ::hybridse::sdk::Status status;
    bool ok = router->CreateDB(db, &status);
    ASSERT_TRUE(ok);
    std::string ddl = "create table " + name +
                      "("
                      "col1 int not null,"
                      "col2 bigint default 112 not null,"
                      "col4 string default 'test4' not null,"
                      "col5 date default '2000-01-01' not null,"
                      "col6 timestamp default 10000 not null,"
                      "index(key=col1, ts=col2)) options(partitionnum=1);";
    ok = router->ExecuteDDL(db, ddl, &status);
    ASSERT_TRUE(ok);
    ASSERT_TRUE(router->RefreshCatalog());
    std::string sql;
    sql = "insert into " + name + " values(1, 1, '1', '2021-01-01', 1);";
    ASSERT_TRUE(router->ExecuteInsert(db, sql, &status));
    auto res = router->ExecuteSQL(db, "select * from " + name + " where col1=1", &status);
    ASSERT_TRUE(res);
    ASSERT_TRUE(res->Next());
    ASSERT_EQ("1, 1, 1, 2021-01-01, 1", res->GetRowString());
    ASSERT_FALSE(res->Next());

    sql = "insert into " + name + "(col1, col2) values(2, 2);";
    ASSERT_TRUE(router->ExecuteInsert(db, sql, &status));
    res = router->ExecuteSQL(db, "select * from " + name + " where col1=2", &status);
    ASSERT_TRUE(res);
    ASSERT_TRUE(res->Next());
    ASSERT_EQ("2, 2, test4, 2000-01-01, 10000", res->GetRowString());
    ASSERT_FALSE(res->Next());
    sql = "insert into " + name + "(col1) values(3);";
    ASSERT_TRUE(router->ExecuteInsert(db, sql, &status));
    res = router->ExecuteSQL(db, "select * from " + name + " where col1=3", &status);
    ASSERT_TRUE(res);
    ASSERT_TRUE(res->Next());
    ASSERT_EQ("3, 112, test4, 2000-01-01, 10000", res->GetRowString());
    ASSERT_FALSE(res->Next());
    sql = "insert into " + name + "(col2) values(4);";
    ASSERT_FALSE(router->ExecuteInsert(db, sql, &status));

    auto endpoints = mc_->GetTbEndpoint();
    uint32_t count = 0;
    for (const auto& endpoint : endpoints) {
        ::openmldb::tablet::TabletImpl* tb1 = mc_->GetTablet(endpoint);
        ::openmldb::api::GetTableStatusRequest request;
        ::openmldb::api::GetTableStatusResponse response;
        MockClosure closure;
        tb1->GetTableStatus(NULL, &request, &response, &closure);
        for (const auto& table_status : response.all_table_status()) {
            count += table_status.record_cnt();
        }
    }
    ASSERT_EQ(3u, count);
    ok = router->ExecuteDDL(db, "drop table " + name + ";", &status);
    ASSERT_TRUE(ok);
    ok = router->DropDB(db, &status);
    ASSERT_TRUE(ok);
}

TEST_F(SQLSDKQueryTest, GetTabletClient) {
    std::string ddl =
        "create table t1(col0 string,\n"
        "                col1 bigint,\n"
        "                col2 string,\n"
        "                col3 bigint,\n"
        "                index(key=col2, ts=col3)) "
        "options(partitionnum=2);";
    SQLRouterOptions sql_opt;
    sql_opt.session_timeout = 30000;
    sql_opt.zk_cluster = mc_->GetZkCluster();
    sql_opt.zk_path = mc_->GetZkPath();
    sql_opt.enable_debug = hybridse::sqlcase::SqlCase::IsDebug();
    auto router = NewClusterSQLRouter(sql_opt);
    if (!router) {
        FAIL() << "Fail new cluster sql router";
    }
    std::string db = "gettabletclient;";
    hybridse::sdk::Status status;
    ASSERT_TRUE(router->CreateDB(db, &status));
    ASSERT_TRUE(router->ExecuteDDL(db, ddl, &status));
    ASSERT_TRUE(router->RefreshCatalog());
    std::string sql =
        "select col2, sum(col1) over w1 from t1 \n"
        "window w1 as (partition by col2 \n"
        "order by col3 rows between 3 preceding and current row);";
    auto ns_client = mc_->GetNsClient();
    std::vector<::openmldb::nameserver::TableInfo> tables;
    std::string msg;
    ASSERT_TRUE(ns_client->ShowTable("t1", db, false, tables, msg));
    for (int i = 0; i < 10; i++) {
        std::string pk = "pk" + std::to_string(i);
        auto request_row = router->GetRequestRow(db, sql, &status);
        request_row->Init(4 + pk.size());
        request_row->AppendString("col0");
        request_row->AppendInt64(1);
        request_row->AppendString(pk);
        request_row->AppendInt64(3);
        ASSERT_TRUE(request_row->Build());
        auto sql_cluster_router = std::dynamic_pointer_cast<SQLClusterRouter>(router);
        auto client = sql_cluster_router->GetTabletClient(db, sql, hybridse::vm::kRequestMode, request_row);
        int pid = ::openmldb::base::hash64(pk) % 2;
        ASSERT_EQ(client->GetEndpoint(), tables[0].table_partition(pid).partition_meta(0).endpoint());
    }
    ASSERT_TRUE(router->ExecuteDDL(db, "drop table t1;", &status));
    ASSERT_TRUE(router->DropDB(db, &status));
}

static std::shared_ptr<SQLRouter> GetNewSQLRouter() {
    SQLRouterOptions sql_opt;
    sql_opt.zk_cluster = mc_->GetZkCluster();
    sql_opt.zk_path = mc_->GetZkPath();
    sql_opt.session_timeout = 60000;
    sql_opt.enable_debug = hybridse::sqlcase::SqlCase::IsDebug();
    return NewClusterSQLRouter(sql_opt);
}
static bool IsRequestSupportMode(const std::string& mode) {
    if (mode.find("hybridse-only") != std::string::npos ||
        mode.find("rtidb-unsupport") != std::string::npos ||
        mode.find("performance-sensitive-unsupport") != std::string::npos ||
        mode.find("request-unsupport") != std::string::npos
        || mode.find("cluster-unsupport") != std::string::npos) {
        return false;
    }
    return true;
}
static bool IsBatchRequestSupportMode(const std::string& mode) {
    if (mode.find("hybridse-only") != std::string::npos ||
        mode.find("rtidb-unsupport") != std::string::npos ||
        mode.find("performance-sensitive-unsupport") != std::string::npos ||
        mode.find("batch-request-unsupport") != std::string::npos ||
        mode.find("request-unsupport") != std::string::npos
        || mode.find("cluster-unsupport") != std::string::npos) {
        return false;
    }
    return true;
}
static bool IsBatchSupportMode(const std::string& mode) {
    if (mode.find("hybridse-only") != std::string::npos ||
        mode.find("rtidb-unsupport") != std::string::npos ||
        mode.find("performance-sensitive-unsupport") != std::string::npos ||
        mode.find("batch-unsupport") != std::string::npos
        || mode.find("cluster-unsupport") != std::string::npos) {
        return false;
    }
    return true;
}
TEST_P(SQLSDKQueryTest, SqlSdkDistributeBatchRequestTest) {
    auto sql_case = GetParam();
    if (!IsBatchRequestSupportMode(sql_case.mode())) {
        LOG(WARNING) << "Unsupport mode: " << sql_case.mode();
        return;
    }
    if (sql_case.batch_request().columns_.empty()) {
        LOG(WARNING) << "No batch request specified";
        return;
    }
    LOG(INFO) << "ID: " << sql_case.id() << ", DESC: " << sql_case.desc();
    ASSERT_TRUE(router_ != nullptr) << "Fail new cluster sql router";
    DistributeRunBatchRequestModeSDK(sql_case, router_);
    LOG(INFO) << "Finish sql_sdk_distribute_batch_request_test: ID: " << sql_case.id() << ", DESC: " << sql_case.desc();
}
TEST_P(SQLSDKBatchRequestQueryTest, SqlSdkDistributeBatchRequestTest) {
    auto sql_case = GetParam();
    if (!IsBatchRequestSupportMode(sql_case.mode())) {
        LOG(WARNING) << "Unsupport mode: " << sql_case.mode();
        return;
    }
    if (sql_case.batch_request().columns_.empty()) {
        LOG(WARNING) << "No batch request specified";
        return;
    }
    LOG(INFO) << "ID: " << sql_case.id() << ", DESC: " << sql_case.desc();
    ASSERT_TRUE(router_ != nullptr) << "Fail new cluster sql router";
    DistributeRunBatchRequestModeSDK(sql_case, router_);
    LOG(INFO) << "Finish sql_sdk_distribute_batch_request_test: ID: " << sql_case.id() << ", DESC: " << sql_case.desc();
}

TEST_P(SQLSDKQueryTest, SqlSdkDistributeRequestTest) {
    auto sql_case = GetParam();
    LOG(INFO) << "ID: " << sql_case.id() << ", DESC: " << sql_case.desc();
    if (!IsRequestSupportMode(sql_case.mode())) {
        LOG(WARNING) << "Unsupport mode: " << sql_case.mode();
        return;
    }
    ASSERT_TRUE(router_ != nullptr) << "Fail new cluster sql router with multi partitions";
    DistributeRunRequestModeSDK(sql_case, router_);
    LOG(INFO) << "Finish sql_sdk_distribute_request_test: ID: " << sql_case.id() << ", DESC: " << sql_case.desc();
}
TEST_P(SQLSDKQueryTest, SqlSdkDistributeBatchRequestSinglePartitionTest) {
    auto sql_case = GetParam();
    if (!IsBatchRequestSupportMode(sql_case.mode())) {
        LOG(WARNING) << "Unsupport mode: " << sql_case.mode();
        return;
    }
    if (sql_case.batch_request().columns_.empty()) {
        LOG(WARNING) << "No batch request specified";
        return;
    }
    LOG(INFO) << "ID: " << sql_case.id() << ", DESC: " << sql_case.desc();
    ASSERT_TRUE(router_ != nullptr) << "Fail new cluster sql router";
    DistributeRunBatchRequestModeSDK(sql_case, router_, 1);
    LOG(INFO) << "Finish sql_sdk_distribute_batch_request_single_partition_test: ID: " << sql_case.id()
              << ", DESC: " << sql_case.desc();
}
TEST_P(SQLSDKBatchRequestQueryTest, SqlSdkDistributeBatchRequestSinglePartitionTest) {
    auto sql_case = GetParam();
    if (!IsBatchRequestSupportMode(sql_case.mode())) {
        LOG(WARNING) << "Unsupport mode: " << sql_case.mode();
        return;
    }
    if (sql_case.batch_request().columns_.empty()) {
        LOG(WARNING) << "No batch request specified";
        return;
    }
    LOG(INFO) << "ID: " << sql_case.id() << ", DESC: " << sql_case.desc();
    ASSERT_TRUE(router_ != nullptr) << "Fail new cluster sql router";
    DistributeRunBatchRequestModeSDK(sql_case, router_, 1);
    LOG(INFO) << "Finish sql_sdk_distribute_batch_request_single_partition_test: ID: " << sql_case.id()
              << ", DESC: " << sql_case.desc();
}

/* TEST_P(SQLSDKQueryTest, sql_sdk_distribute_request_single_partition_test) {
    auto sql_case = GetParam();
    LOG(INFO) << "ID: " << sql_case.id() << ", DESC: " << sql_case.desc();
    if (boost::contains(sql_case.mode(), "rtidb-unsupport") ||
        boost::contains(sql_case.mode(), "rtidb-request-unsupport") ||
        boost::contains(sql_case.mode(), "request-unsupport") ||
        boost::contains(sql_case.mode(), "cluster-unsupport")) {
        LOG(WARNING) << "Unsupport mode: " << sql_case.mode();
        return;
    }
    ASSERT_TRUE(router_ != nullptr) << "Fail new cluster sql router with multi partitions";
    DistributeRunRequestModeSDK(sql_case, router_, 1);
    LOG(INFO) << "Finish sql_sdk_distribute_request_single_partition_test: ID: " << sql_case.id()
              << ", DESC: " << sql_case.desc();
} */

TEST_P(SQLSDKBatchRequestQueryTest, SqlSdkDistributeBatchRequestProcedureTest) {
    auto sql_case = GetParam();
    if (!IsBatchRequestSupportMode(sql_case.mode())) {
        LOG(WARNING) << "Unsupport mode: " << sql_case.mode();
        return;
    }
    if (sql_case.batch_request().columns_.empty()) {
        LOG(WARNING) << "No batch request specified";
        return;
    }
    LOG(INFO) << "ID: " << sql_case.id() << ", DESC: " << sql_case.desc();
    ASSERT_TRUE(router_ != nullptr) << "Fail new cluster sql router";
    DistributeRunBatchRequestProcedureModeSDK(sql_case, router_, 8, false);
    LOG(INFO) << "Finish sql_sdk_distribute_batch_request_procedure_test: ID: " << sql_case.id()
              << ", DESC: " << sql_case.desc();
}

TEST_P(SQLSDKQueryTest, SqlSdkDistributeRequestProcedureTest) {
    auto sql_case = GetParam();
    LOG(INFO) << "ID: " << sql_case.id() << ", DESC: " << sql_case.desc();
    if (!IsRequestSupportMode(sql_case.mode())) {
        LOG(WARNING) << "Unsupport mode: " << sql_case.mode();
        return;
    }
    ASSERT_TRUE(router_ != nullptr) << "Fail new cluster sql router with multi partitions";
    DistributeRunRequestProcedureModeSDK(sql_case, router_, 8, false);
    LOG(INFO) << "Finish sql_sdk_distribute_request_procedure_test: ID: " << sql_case.id()
              << ", DESC: " << sql_case.desc();
}
TEST_P(SQLSDKBatchRequestQueryTest, SqlSdkDistributeBatchRequestProcedureAsyncTest) {
    auto sql_case = GetParam();
    if (!IsRequestSupportMode(sql_case.mode())) {
        LOG(WARNING) << "Unsupport mode: " << sql_case.mode();
        return;
    }
    if (sql_case.batch_request().columns_.empty()) {
        LOG(WARNING) << "No batch request specified";
        return;
    }
    LOG(INFO) << "ID: " << sql_case.id() << ", DESC: " << sql_case.desc();
    ASSERT_TRUE(router_ != nullptr) << "Fail new cluster sql router";
    DistributeRunBatchRequestProcedureModeSDK(sql_case, router_, 8, true);
    LOG(INFO) << "Finish sql_sdk_distribute_batch_request_procedure_async_test: ID: " << sql_case.id()
              << ", DESC: " << sql_case.desc();
}

TEST_P(SQLSDKQueryTest, SqlSdkDistributeRequestProcedureAsyncTest) {
    auto sql_case = GetParam();
    LOG(INFO) << "ID: " << sql_case.id() << ", DESC: " << sql_case.desc();
    if (!IsRequestSupportMode(sql_case.mode())) {
        LOG(WARNING) << "Unsupport mode: " << sql_case.mode();
        return;
    }
    ASSERT_TRUE(router_ != nullptr) << "Fail new cluster sql router with multi partitions";
    DistributeRunRequestProcedureModeSDK(sql_case, router_, 8, true);
    LOG(INFO) << "Finish sql_sdk_distribute_request_procedure_async_test: ID: " << sql_case.id()
              << ", DESC: " << sql_case.desc();
}

TEST_F(SQLClusterTest, CreateTable) {
    SQLRouterOptions sql_opt;
    sql_opt.zk_cluster = mc_->GetZkCluster();
    sql_opt.zk_path = mc_->GetZkPath();
    auto router = NewClusterSQLRouter(sql_opt);
    ASSERT_TRUE(router != nullptr);
    std::string db = "db" + GenRand();
    ::hybridse::sdk::Status status;
    bool ok = router->CreateDB(db, &status);
    ASSERT_TRUE(ok);
    for (int i = 0; i < 2; i++) {
        std::string name = "test" + std::to_string(i);
        std::string ddl = "create table " + name +
                          "("
                          "col1 string, col2 bigint,"
                          "index(key=col1, ts=col2)) "
                          "options(partitionnum=3);";
        ok = router->ExecuteDDL(db, ddl, &status);
        ASSERT_TRUE(ok);
    }
    ASSERT_TRUE(router->RefreshCatalog());
    auto ns_client = mc_->GetNsClient();
    std::vector<::openmldb::nameserver::TableInfo> tables;
    std::string msg;
    ASSERT_TRUE(ns_client->ShowTable("", db, false, tables, msg));
    ASSERT_TRUE(!tables.empty());
    std::map<std::string, int> pid_map;
    for (const auto& table : tables) {
        for (const auto& partition : table.table_partition()) {
            for (const auto& meta : partition.partition_meta()) {
                if (pid_map.find(meta.endpoint()) == pid_map.end()) {
                    pid_map.emplace(meta.endpoint(), 0);
                }
                pid_map[meta.endpoint()]++;
            }
        }
    }
    ASSERT_EQ(pid_map.size(), 3u);
    ASSERT_EQ(pid_map.begin()->second, pid_map.rbegin()->second);
    ASSERT_TRUE(router->ExecuteDDL(db, "drop table test0;", &status));
    ASSERT_TRUE(router->ExecuteDDL(db, "drop table test1;", &status));
    ASSERT_TRUE(router->DropDB(db, &status));
}

TEST_F(SQLClusterTest, GetTableSchema) {
    SQLRouterOptions sql_opt;
    sql_opt.zk_cluster = mc_->GetZkCluster();
    sql_opt.zk_path = mc_->GetZkPath();
    auto router = NewClusterSQLRouter(sql_opt);
    ASSERT_TRUE(router != nullptr);
    std::string db = "db" + GenRand();
    std::string table = "test0";
    ::hybridse::sdk::Status status;
    bool ok = router->CreateDB(db, &status);
    ASSERT_TRUE(ok);

    std::string ddl = "create table " + table +
                      "("
                      "col1 string, col2 bigint,"
                      "index(key=col1, ts=col2)) "
                      "options(partitionnum=3);";
    ok = router->ExecuteDDL(db, ddl, &status);
    ASSERT_TRUE(ok);
    ASSERT_TRUE(router->RefreshCatalog());

    auto schema = router->GetTableSchema(db, table);
    ASSERT_EQ(schema->GetColumnCnt(), 2);
    ASSERT_EQ(schema->GetColumnName(0), "col1");
    ASSERT_EQ(schema->GetColumnType(0), hybridse::sdk::DataType::kTypeString);
    ASSERT_EQ(schema->GetColumnName(1), "col2");
    ASSERT_EQ(schema->GetColumnType(1), hybridse::sdk::DataType::kTypeInt64);

    ASSERT_TRUE(router->ExecuteDDL(db, "drop table test0;", &status));
    ASSERT_TRUE(router->DropDB(db, &status));
}

}  // namespace sdk
}  // namespace openmldb

int main(int argc, char** argv) {
    ::hybridse::vm::Engine::InitializeGlobalLLVM();
    FLAGS_zk_session_timeout = 100000;
    ::openmldb::sdk::MiniCluster mc(6181);
    ::openmldb::sdk::mc_ = &mc;
    FLAGS_enable_distsql = true;
    int ok = ::openmldb::sdk::mc_->SetUp(3);
    sleep(1);
    ::testing::InitGoogleTest(&argc, argv);
    srand(time(NULL));
    ::google::ParseCommandLineFlags(&argc, &argv, true);
    ::openmldb::sdk::router_ = ::openmldb::sdk::GetNewSQLRouter();
    if (nullptr == ::openmldb::sdk::router_) {
        LOG(ERROR) << "Fail Test with NULL SQL router";
        return -1;
    }
    ok = RUN_ALL_TESTS();
    ::openmldb::sdk::mc_->Close();
    return ok;
}
