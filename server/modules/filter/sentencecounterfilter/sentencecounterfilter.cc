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
    m_logfile(logfile), m_seconds(seconds), m_collectivelly(collectivelly)
{
    std::stringstream message;
    message << "Sentence counter filter created: Log file: " << m_logfile
            << ", Seconds: " << m_seconds
            << ", Collectivelly: " << m_collectivelly;

    MXS_NOTICE("%s", message.str().c_str());
}

SentenceCounterFilter::~SentenceCounterFilter()
{
}

SentenceCounterFilter* SentenceCounterFilter::create(const char* zName, MXS_CONFIG_PARAMETER* ppParams)
{
    SentenceCounterFilter* pFilter = nullptr;
    // TODO: Get the params into vars that we can pass to the constructor
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
