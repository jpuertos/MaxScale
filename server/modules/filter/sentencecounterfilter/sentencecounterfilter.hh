/*
 * Copyright (c) 2019 MariaDB Corporation Ab
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE.TXT file and at www.mariadb.com/bsl11.
 *
 * Change Date: 2022-01-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */
#pragma once

#include <maxscale/ccdefs.hh>
#include <maxscale/filter.hh>
#include <maxbase/stopwatch.hh>
#include "sentencecountersession.hh"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace maxscale
{

class OperationCounterFilter : public maxscale::Filter<OperationCounterFilter, OperationCounterSession>
{
    OperationCounterFilter(const OperationCounterFilter&);
    OperationCounterFilter& operator=(const OperationCounterFilter&);

public:
    ~OperationCounterFilter();

    static OperationCounterFilter* create(const char* zName, MXS_CONFIG_PARAMETER* ppParams);
    OperationCounterSession* newSession(MXS_SESSION* pSession);
    void    diagnostics(DCB* pDcb);
    json_t* diagnostics_json() const;
    uint64_t getCapabilities();

    void increment(qc_query_op_t operation);
    void save();

private:
    OperationCounterFilter(std::string logfile, unsigned long seconds, bool collectivelly);
    void logger_task(); // Waits for the time window and then writes into logfile


    std::string m_logfile;
    std::chrono::seconds m_time_window;
    bool m_collectivelly = false;
    std::unordered_map<qc_query_op_t, unsigned long> m_counter;
    std::chrono::time_point<std::chrono::system_clock> m_last_saving_time;

    std::thread m_task;
    std::mutex m_counters_mutex;
    std::condition_variable m_stop_cv;
    std::mutex m_stop_mutex; // To signal that launcher thread is destroying this object
    bool m_stop = false;
};

} // maxscale
