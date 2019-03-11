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

// All log messages from this module are prefixed with this
#define MXS_MODULE_NAME "operationcounterfiler"

#include "operationcounterfilter.hh"

#include <algorithm> // std::find
#include <fstream>
#include <sstream>

// This declares a module in MaxScale
extern "C" MXS_MODULE* MXS_CREATE_MODULE()
{
    static MXS_MODULE info =
    {
        MXS_MODULE_API_FILTER,
        MXS_MODULE_IN_DEVELOPMENT,
        MXS_FILTER_VERSION,
        "Counts the number of times an operation is used in a given window time",
        "V1.0.0",
        RCAP_TYPE_STMT_INPUT,
        &maxscale::OperationCounterFilter::s_object,               // This is defined in the MaxScale filter template
        NULL,                                   /* Process init. */
        NULL,                                   /* Process finish. */
        NULL,                                   /* Thread init. */
        NULL,                                   /* Thread finish. */
        {
            {"logfile",          MXS_MODULE_PARAM_STRING, "/var/log/operationcount.txt"},
            {"seconds",          MXS_MODULE_PARAM_COUNT, "42"},
            {"collectivelly",    MXS_MODULE_PARAM_BOOL, "false"},
            {MXS_END_MODULE_PARAMS}
        }
    };

    return &info;
}

namespace maxscale
{

OperationCounterFilter::OperationCounterFilter(std::string logfile, unsigned long seconds, bool collectivelly,
    std::vector<qc_query_op_t> ops_counted_for = {QUERY_OP_SELECT, QUERY_OP_INSERT, QUERY_OP_UPDATE, QUERY_OP_DELETE}) :
    m_logfile(logfile), m_time_window(seconds), m_collectivelly(collectivelly), m_ops_counted_for(ops_counted_for), m_counter()
{
    std::stringstream message;
    message << "Operation counter filter created: Log file: " << m_logfile
            << ", Seconds: " << seconds
            << ", Collectivelly: " << m_collectivelly;

    // We init counters to zero
    for (auto op : m_ops_counted_for)
    {
        m_counter[op] = 0;
    }

    m_task = std::thread{&OperationCounterFilter::logger_task, this}; // Using move semantics to start the logger thread
    MXS_NOTICE("%s", message.str().c_str());
}

OperationCounterFilter::~OperationCounterFilter()
{
    stop();
}

OperationCounterFilter* OperationCounterFilter::create(const char* zName, MXS_CONFIG_PARAMETER* ppParams)
{
    auto logfile = config_get_string(ppParams, "logfile");
    auto seconds = config_get_size(ppParams, "seconds");
    auto collectivelly = config_get_bool(ppParams, "collectivelly");

    return new OperationCounterFilter(logfile, seconds, collectivelly);
}


OperationCounterSession* OperationCounterFilter::newSession(MXS_SESSION* pSession)
{
    return OperationCounterSession::create(pSession, this);
}

void OperationCounterFilter::diagnostics(DCB* pDcb)
{

}

// static
json_t* OperationCounterFilter::diagnostics_json() const
{
    return NULL;
}

// static
uint64_t OperationCounterFilter::getCapabilities()
{
    return RCAP_TYPE_NONE;
}

void OperationCounterFilter::increment(qc_query_op_t operation)
{
    std::lock_guard<std::mutex> lock(m_counters_mutex); // No saving while we increment

    // This has linear cost, but we need keep an ordered sequence of the operations in a vector
    if (std::find(m_ops_counted_for.begin(), m_ops_counted_for.end(), operation) != m_ops_counted_for.end())
    {
        m_counter[operation]++;
    }
}


void OperationCounterFilter::save()
{
    std::lock_guard<std::mutex> lock(m_counters_mutex);

    std::ofstream file(m_logfile);
    if (file.is_open())
    {
        auto now = std::chrono::system_clock::now();
        auto now_time = std::chrono::system_clock::to_time_t(now);
        auto last_time = std::chrono::system_clock::to_time_t(m_last_saving_time);

        file << "\"" << std::ctime(&now_time) << "\" \"" << std::ctime(&last_time) << "\",";
        for (auto & op : m_ops_counted_for)
        {
            file << m_counter[op] << ",";
        }
        file <<"\b\n"; // Replace last coma with newline

        m_last_saving_time = now;
        m_counter.clear();
        file.close();
    }
    else
    {
        // TODO: Error, unable to open file
    }
}

void OperationCounterFilter::stop()
{
    std::lock_guard<std::mutex> lock(m_counters_mutex); // Need to wait if saving operation is ongoin and prevent starting a new one
    m_stop = true;
    m_stop_cv.notify_one(); // We notify the logger thread that we are finishing up
    m_task.join(); // And we wait for logger thread to finish its job
}
/*
 * We need to wait either for the time to expire in which case we would save()
 * or for the condition variable 'stop' (object being destructed) become true
 */
void OperationCounterFilter::logger_task()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(m_stop_mutex);
        if(m_stop_cv.wait_for(lock, m_time_window, [this](){return m_stop == true;}))
        {
            return;
        }
        else // Period expired
        {
            save();
        }

    }
}

} // maxscale
