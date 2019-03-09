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

class SentenceCounterFilter;

class SentenceCounterSession : public maxscale::FilterSession
{
    SentenceCounterSession(const SentenceCounterSession&);
    SentenceCounterSession& operator=(const SentenceCounterSession&);

public:
    ~SentenceCounterSession();

    void close();

    static SentenceCounterSession* create(MXS_SESSION* pSession, const SentenceCounterFilter* p);

    int routeQuery(GWBUF* pPacket);

private:
    SentenceCounterSession(MXS_SESSION* pSession);
};
