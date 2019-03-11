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

#define MXS_MODULE_NAME "operationcountersession"

#include "operationcountersession.hh"
#include "operationcounterfilter.hh"

namespace maxscale
{

OperationCounterSession::OperationCounterSession(MXS_SESSION* pSession, OperationCounterFilter* pFilter)
    : maxscale::FilterSession(pSession), m_filter(*pFilter)
{
}

OperationCounterSession::~OperationCounterSession()
{
}

// static
OperationCounterSession* OperationCounterSession::create(MXS_SESSION* pSession, OperationCounterFilter* pFilter)
{
    return new OperationCounterSession(pSession, pFilter);
}

void OperationCounterSession::close()
{
    m_filter.stop(); // Stop the logging thread.
}

int OperationCounterSession::routeQuery(GWBUF* pPacket)
{
    auto op = qc_get_operation(pPacket);
    m_filter.increment(op);
    return mxs::FilterSession::routeQuery(pPacket);
}

} // maxscale
