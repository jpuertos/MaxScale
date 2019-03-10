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
#define MXS_MODULE_NAME "sentencehistogram"

#include "sentencecounterfilter.hh"

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
        "Counts the number of times a sentence is used in a given window time",
        "V1.0.0",
        RCAP_TYPE_NONE,
        &SentenceCounterFilter::s_object,               // This is defined in the MaxScale filter template
        NULL,                                   /* Process init. */
        NULL,                                   /* Process finish. */
        NULL,                                   /* Thread init. */
        NULL,                                   /* Thread finish. */
        {
            {"logfile",          MXS_MODULE_PARAM_STRING, "/var/log/sentencecount.txt"},
            {"seconds",          MXS_MODULE_PARAM_COUNT, "42"},
            {"collectivelly",    MXS_MODULE_PARAM_BOOL, "false"},
            {MXS_END_MODULE_PARAMS}
        }
    };

    return &info;
}

SentenceCounterFilter::SentenceCounterFilter(std::string logfile, unsigned long seconds, bool collectivelly) :
    m_logfile(logfile), m_time_window(seconds), m_collectivelly(collectivelly), m_counter()
{
    std::stringstream message;
    message << "Sentence counter filter created: Log file: " << m_logfile
            << ", Seconds: " << seconds
            << ", Collectivelly: " << m_collectivelly;

    m_task = std::thread{&SentenceCounterFilter::logger_task, this}; // Using move semantics to start the logger thread
    MXS_NOTICE("%s", message.str().c_str());
}

SentenceCounterFilter::~SentenceCounterFilter()
{
    std::lock_guard<std::mutex> lock(m_counters_mutex); // Need to wait if saving operation is ongoin and prevent starting a new one
    m_stop = true;
    m_stop_cv.notify_one(); // We notify the logger thread that we are finishing up
    m_task.join(); // And we wait for logger thread to finish its job
}

SentenceCounterFilter* SentenceCounterFilter::create(const char* zName, MXS_CONFIG_PARAMETER* ppParams)
{
    auto logfile = config_get_string(ppParams, "logfile");
    auto seconds = config_get_size(ppParams, "seconds");
    auto collectivelly = config_get_bool(ppParams, "collectivelly");

    return new SentenceCounterFilter(logfile, seconds, collectivelly);
}


SentenceCounterSession* SentenceCounterFilter::newSession(MXS_SESSION* pSession)
{
    return SentenceCounterSession::create(pSession, this);
}

void SentenceCounterFilter::diagnostics(DCB* pDcb)
{

}

// static
json_t* SentenceCounterFilter::diagnostics_json() const
{
    return NULL;
}

// static
uint64_t SentenceCounterFilter::getCapabilities()
{
    return RCAP_TYPE_NONE;
}

void SentenceCounterFilter::increment(qc_query_op_t operation)
{
    std::lock_guard<std::mutex> lock(m_counters_mutex); // No saving while we increment

    // TODO: Check if is one of the ones we want to log SELECT, INSERT, DELETE, UPDATE
    m_counter[operation]++;
}


void SentenceCounterFilter::save()
{
    std::lock_guard<std::mutex> lock(m_counters_mutex);

    std::ofstream file(m_logfile);
    if (file.is_open())
    {
        auto now = std::chrono::system_clock::now();
        auto now_time = std::chrono::system_clock::to_time_t(now);
        auto last_time = std::chrono::system_clock::to_time_t(m_last_saving_time);

        file << "\"" << std::ctime(&now_time) << "\" \"" << std::ctime(&last_time) << "\",";
        for (auto & c : m_counter) // TODO, there is no order... how to know what column is what
        {
            file << c.second << ",";
        }
        file <<"\b\n"; // Replace last coma with newline
        file.flush();
    }
    else
    {
        // TODO: Error, unable to open file
    }
    m_last_saving_time = std::chrono::system_clock::now();
    m_counter.clear();
}

/*
 * We need to wait either for the time to expire in which case we would save()
 * or for the condition variable 'stop' (object being destructed) become true
 */
void SentenceCounterFilter::logger_task()
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
