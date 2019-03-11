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

namespace maxscale
{

class OperationCounterFilter;

class OperationCounterSession : public maxscale::FilterSession
{
    OperationCounterSession(const OperationCounterSession&);
    OperationCounterSession& operator=(const OperationCounterSession&);

public:
    virtual ~OperationCounterSession();

    void close();

    static OperationCounterSession* create(MXS_SESSION* pSession, OperationCounterFilter* p);

    int routeQuery(GWBUF* pPacket);

private:
    OperationCounterSession(MXS_SESSION* pSession, OperationCounterFilter* pFilter);
    OperationCounterFilter& m_filter;
};

} // maxscale
