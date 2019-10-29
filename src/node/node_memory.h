/*-------------------------------------------------------------------------
 * Copyright (C) 2019, 4paradigm
 * memory_manager.h
 *      负责FeSQL的基础元件（SQLNode, PlanNode)的创建和销毁
 *      SQL的语法解析树、查询计划里面维护的只是这些节点的指针或者引用
 * Author: chenjing
 * Date: 2019/10/28 
 *--------------------------------------------------------------------------
**/

#ifndef FESQL_NODE_MEMORY_H
#define FESQL_NODE_MEMORY_H

#include "sql_node.h"
#include "plan_node.h"
#include <list>

namespace fesql {
namespace node {
class NodeManager {
public:

    NodeManager() {

    }

    ~NodeManager() {

        std::list<SQLNode *>::iterator sql_node_ite;
        for (sql_node_ite = parser_node_list_.begin(); sql_node_ite != parser_node_list_.end(); ++sql_node_ite) {
            delete (*sql_node_ite);
            sql_node_ite = parser_node_list_.erase(sql_node_ite);
        }

        std::list<node::PlanNode *>::iterator plan_node_ite;
        for (plan_node_ite = plan_node_list_.begin(); plan_node_ite != plan_node_list_.end(); ++plan_node_ite) {
            delete (*plan_node_ite);
            plan_node_ite = plan_node_list_.erase(plan_node_ite);
        }

        std::list<node::SQLLinkedNode *>::iterator linked_node_ite;
        for (linked_node_ite = linked_node_list_.begin(); linked_node_ite != linked_node_list_.end(); ++linked_node_ite) {
            delete (*linked_node_ite);
            linked_node_ite = linked_node_list_.erase(linked_node_ite);
        }

        std::list<node::SQLNodeList *>::iterator sql_node_list_iter;
        for (sql_node_list_iter = sql_node_list_list_.begin(); sql_node_list_iter != sql_node_list_list_.end(); ++sql_node_list_iter) {
            delete (*sql_node_list_iter);
            sql_node_list_iter = sql_node_list_list_.erase(sql_node_list_iter);
        }
    }

    int GetParserNodeListSize() {
        return parser_node_list_.size();
    }

    int GetPlanNodeListSize() {
        return plan_node_list_.size();
    }

    // Make xxxPlanNode
    PlanNode *MakePlanNode(const PlanType &type);
    PlanNode *MakeLeafPlanNode(const PlanType &type);
    PlanNode *MakeUnaryPlanNode(const PlanType &type);
    PlanNode *MakeBinaryPlanNode(const PlanType &type);
    PlanNode *MakeMultiPlanNode(const PlanType &type);
    ProjectListPlanNode *MakeProjectListPlanNode(const std::string &table, const std::string &w);
    ProjectPlanNode *MakeProjectPlanNode(node::SQLNode * expression, const std::string &name, const std::string &table, const std::string &w);
    // Make SQLxxx Node
    SQLNode *MakeSQLNode(const SQLNodeType &type);
    SQLNode *MakeSelectStmtNode(SQLNodeList *select_list_ptr_,
                                SQLNodeList *tableref_list_ptr,
                                SQLNodeList *window_clause_ptr,
                                SQLNode *limit_clause_ptr);
    SQLNode *MakeTableNode(const std::string &name, const std::string &alias);
    SQLNode *MakeFuncNode(const std::string &name, SQLNodeList *args, SQLNode *over);
    SQLNode *MakeWindowDefNode(const std::string &name);
    SQLNode *MakeWindowDefNode(SQLNodeList *partitions, SQLNodeList *orders, SQLNode *frame);
    SQLNode *MakeOrderByNode(SQLNode *node_ptr);
    SQLNode *MakeFrameNode(SQLNode *start, SQLNode *end);
    SQLNode *MakeFrameBound(SQLNodeType bound_type);
    SQLNode *MakeFrameBound(SQLNodeType bound_type, SQLNode *offset);
    SQLNode *MakeRangeFrameNode(SQLNode *node_ptr);
    SQLNode *MakeRowsFrameNode(SQLNode *node_ptr);
    SQLNode *MakeLimitNode(int count);
    SQLNode *MakeConstNode(int value);
    SQLNode *MakeConstNode(long value);
    SQLNode *MakeConstNode(float value);
    SQLNode *MakeConstNode(double value);
    SQLNode *MakeConstNode(const std::string &value);
    SQLNode *MakeConstNode();

    SQLNode *MakeColumnRefNode(const std::string &column_name, const std::string &relation_name);
    SQLNode *MakeResTargetNode(SQLNode *node_ptr, const std::string &name);

    // Make NodeList
    SQLNodeList *MakeNodeList(SQLNode *node_ptr);
    SQLNodeList *MakeNodeList();
    // Make Linked Node
    SQLLinkedNode *MakeLinkedNode(SQLNode *node_ptr);
private:

    void RegisterNode(SQLNode *node_ptr) {
        parser_node_list_.push_back(node_ptr);
    }

    void RegisterNode(node::PlanNode *node_ptr) {
        plan_node_list_.push_back(node_ptr);
    }

    void RegisterNode(SQLNodeList *node_ptr) {
        sql_node_list_list_.push_back(node_ptr);
    }

    void RegisterNode(SQLLinkedNode *node_ptr) {
        linked_node_list_.push_back(node_ptr);
    }

    std::list<SQLNode *> parser_node_list_;
    std::list<SQLNodeList *> sql_node_list_list_;
    std::list<SQLLinkedNode *> linked_node_list_;
    std::list<node::PlanNode *> plan_node_list_;
};
}
}
#endif //FESQL_NODE_MEMORY_H
