/* Copyright (c) 2018 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#include "base/Base.h"
#include "graph/SequentialExecutor.h"
#include "graph/GoExecutor.h"
#include "graph/PipeExecutor.h"
#include "graph/UseExecutor.h"

namespace nebula {
namespace graph {

SequentialExecutor::SequentialExecutor(SequentialSentences *sentences,
                                       ExecutionContext *ectx) : Executor(ectx) {
    sentences_ = sentences;
}


Status SequentialExecutor::prepare() {
    for (auto i = 0U; i < sentences_->sentences_.size(); i++) {
        auto *sentence = sentences_->sentences_[i].get();
        auto executor = makeExecutor(sentence);
        DCHECK(executor != nullptr);
        auto status = executor->prepare();
        if (!status.ok()) {
            FLOG_ERROR("Prepare executor `%s' failed: %s",
                        executor->name(), status.toString().c_str());
            return status;
        }
        executors_.emplace_back(std::move(executor));
    }
    /**
     * For the time being, we execute sentences one by one. We may allow concurrent
     * or out of order execution in the future.
     */
    // For an executor except the last one, it executes the next one on finished.
    // If any fails, the whole execution would abort.
    auto onError = [this] (Status status) {
        DCHECK(onError_);
        onError_(std::move(status));
    };
    for (auto i = 0U; i < executors_.size() - 1; i++) {
        auto onFinish = [this, next = i + 1] () {
            executors_[next]->execute();
        };
        executors_[i]->setOnFinish(onFinish);
        executors_[i]->setOnError(onError);
    }
    // The whole execution is done upon the last executor finishes.
    auto onFinish = [this] () {
        DCHECK(onFinish_);
        onFinish_();
    };
    executors_.back()->setOnFinish(onFinish);
    executors_.back()->setOnError(onError);

    return Status::OK();
}


void SequentialExecutor::execute() {
    executors_.front()->execute();
}


void SequentialExecutor::setupResponse(cpp2::ExecutionResponse &resp) {
    executors_.back()->setupResponse(resp);
}

}   // namespace graph
}   // namespace nebula
