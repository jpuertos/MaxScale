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
            {"collectivelly",    MXS_MODULE_PARAM_BOOL, "true"},
            {MXS_END_MODULE_PARAMS}
        }
    };

    return &info;
}

SentenceCounterFilter::SentenceCounterFilter()
{
}

SentenceCounterFilter::~SentenceCounterFilter()
{
}

SentenceCounterFilter* SentenceCounterFilter::create(const char* zName, MXS_CONFIG_PARAMETER* ppParams)
{
    return new SentenceCounterFilter();
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
