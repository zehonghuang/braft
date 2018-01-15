// Copyright (c) 2017 Baidu.com, Inc. All Rights Reserved
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Authors: Qin,Duohao(qinduohao@baidu.com)

#ifndef PUBLIC_RAFT_MEMORY_LOG_H
#define PUBLIC_RAFT_MEMORY_LOG_H

#include <vector>
#include <deque>
#include <base/atomicops.h>
#include <base/iobuf.h>
#include <base/logging.h>
#include "raft/log_entry.h"
#include "raft/storage.h"
#include "raft/util.h"

namespace raft {

class BAIDU_CACHELINE_ALIGNMENT MemoryLogStorage : public LogStorage {
public:
    typedef std::deque<LogEntry*> MemoryData;

    MemoryLogStorage(const std::string& path)
            : _path(path),
            _first_log_index(1),
            _last_log_index(0) {}
    MemoryLogStorage()
            : _first_log_index(1),
            _last_log_index(0) {}

    virtual ~MemoryLogStorage() {}

    virtual int init(ConfigurationManager* configuration_manager);

    // first log index in log
    virtual int64_t first_log_index() {
        return _first_log_index.load(base::memory_order_acquire);
    }

    // last log index in log
    virtual int64_t last_log_index() {
        return _last_log_index.load(base::memory_order_acquire);
    }

    // get logentry by index
    virtual LogEntry* get_entry(const int64_t index);

    // get logentry's term by index
    virtual int64_t get_term(const int64_t index);

    // append entries to log
    virtual int append_entry(const LogEntry* entry);

    // append entries to log, return append success number
    virtual int append_entries(const std::vector<LogEntry*>& entries);

    // delete logs from storage's head, [first_log_index, first_index_kept) will be discarded
    virtual int truncate_prefix(const int64_t first_index_kept);

    // delete uncommitted logs from storage's tail, (last_index_kept, last_log_index] will be discarded
    virtual int truncate_suffix(const int64_t last_index_kept);

    // Drop all the existing logs and reset next log index to |next_log_index|.
    // This function is called after installing snapshot from leader
    virtual int reset(const int64_t next_log_index);

    // Create an instance of this kind of LogStorage with the parameters encoded
    // in |uri|
    // Return the address referenced to the instance on success, NULL otherwise.
    virtual LogStorage* new_instance(const std::string& uri) const;

private:
    std::string _path;
    base::atomic<int64_t> _first_log_index;
    base::atomic<int64_t> _last_log_index;
    MemoryData _log_entry_data;
    raft_mutex_t _mutex;
};

} // namespace raft

#endif //~PUBLIC_RAFT_MEMORY_LOG_H
