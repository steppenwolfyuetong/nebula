/* Copyright (c) 2018 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#ifndef STORAGE_QUERYBOUNDPROCESSOR_H_
#define STORAGE_QUERYBOUNDPROCESSOR_H_

#include "base/Base.h"
#include "storage/QueryBaseProcessor.h"

namespace nebula {
namespace storage {


class QueryBoundProcessor
    : public QueryBaseProcessor<cpp2::GetNeighborsRequest, cpp2::QueryResponse> {
public:
    static QueryBoundProcessor* instance(kvstore::KVStore* kvstore,
                                         meta::SchemaManager* schemaMan,
                                         BoundType type = BoundType::OUT_BOUND) {
        return new QueryBoundProcessor(kvstore, schemaMan, type);
    }

protected:
    explicit QueryBoundProcessor(kvstore::KVStore* kvstore,
                                 meta::SchemaManager* schemaMan,
                                 BoundType type)
        : QueryBaseProcessor<cpp2::GetNeighborsRequest,
                             cpp2::QueryResponse>(kvstore, schemaMan, type) {}

    kvstore::ResultCode processVertex(PartitionID partID,
                                      VertexID vId,
                                      std::vector<TagContext>& tagContexts,
                                      EdgeContext& edgeContext) override;

    void onProcessed(std::vector<TagContext>& tagContexts,
                     EdgeContext& edgeContext,
                     int32_t retNum) override;

private:
    std::vector<cpp2::VertexData> vertices_;
};

}  // namespace storage
}  // namespace nebula
#endif  // STORAGE_QUERYBOUNDPROCESSOR_H_
