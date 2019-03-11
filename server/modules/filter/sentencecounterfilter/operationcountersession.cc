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

OperationCounterSession::OperationCounterSession(MXS_SESSION* pSession)
    : maxscale::FilterSession(pSession)
{
}

OperationCounterSession::~OperationCounterSession()
{
}

// static
OperationCounterSession* OperationCounterSession::create(MXS_SESSION* pSession, const OperationCounterFilter* pFilter)
{
    return new OperationCounterSession(pSession);
}

void OperationCounterSession::close()
{
    // TODO: Write the counters into the log file?
}

int OperationCounterSession::routeQuery(GWBUF* pPacket)
{
    auto op = qc_get_operation(pPacket);
    return mxs::FilterSession::routeQuery(pPacket);
}

} // maxscale
