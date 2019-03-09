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
#include "sentencecountersession.hh"

#include <unordered_map>

class SentenceCounterFilter : public maxscale::Filter<SentenceCounterFilter, SentenceCounterSession>
{
    SentenceCounterFilter(const SentenceCounterFilter&);
    SentenceCounterFilter& operator=(const SentenceCounterFilter&);

public:
    ~SentenceCounterFilter();

    // Creates a new filter instance
    static SentenceCounterFilter* create(const char* zName, MXS_CONFIG_PARAMETER* ppParams);

    // Creates a new session for this filter
    SentenceCounterSession* newSession(MXS_SESSION* pSession);

    void    diagnostics(DCB* pDcb);
    json_t* diagnostics_json() const;

    uint64_t getCapabilities();

private:
    SentenceCounterFilter(std::string logfile, unsigned long seconds, bool collectivelly);

    std::string m_logfile;
    unsigned long m_seconds = 0;
    bool m_collectivelly = false;
    std::unordered_map<qc_query_op_t, unsigned long> m_counter;
};
