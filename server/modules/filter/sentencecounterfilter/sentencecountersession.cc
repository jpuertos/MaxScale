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

#define MXS_MODULE_NAME "sentencecounterfilter"

#include "sentencecountersession.hh"
#include "sentencecounterfilter.hh"

SentenceCounterSession::SentenceCounterSession(MXS_SESSION* pSession)
    : mxs::FilterSession(pSession)
{
}

SentenceCounterSession::~SentenceCounterSession()
{
}

// static
SentenceCounterSession* SentenceCounterSession::create(MXS_SESSION* pSession, const SentenceCounterFilter* pFilter)
{
    return new SentenceCounterSession(pSession);
}

void SentenceCounterSession::close()
{
    // TODO: Write the counters into te log file?
}

int SentenceCounterSession::routeQuery(GWBUF* pPacket)
{
    return mxs::FilterSession::routeQuery(pPacket);
}
