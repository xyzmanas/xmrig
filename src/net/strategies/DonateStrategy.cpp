Skip to content
 
Search or jump to…

Pull requests
Issues
Marketplace
Explore
 @xyzmanas Sign out
You don’t have any verified emails. We recommend verifying at least one email.
Email verification helps our support team verify ownership if you lose account access and allows you to receive all the notifications you ask for.
1
1 785 rdavydov/xmrig-noDevFee
forked from xmrig/xmrig
 Code  Pull requests 0  Projects 0  Wiki  Insights
xmrig-noDevFee/src/net/strategies/DonateStrategy.cpp
86539fc  on May 21
@rdavydov rdavydov Let it mine!
@xmrig @rdavydov
     
174 lines (130 sloc)  4.96 KB
/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2016-2018 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include "common/crypto/keccak.h"
#include "common/net/Client.h"
#include "common/net/Job.h"
#include "common/net/strategies/FailoverStrategy.h"
#include "common/net/strategies/SinglePoolStrategy.h"
#include "common/Platform.h"
#include "common/xmrig.h"
#include "interfaces/IStrategyListener.h"
#include "net/strategies/DonateStrategy.h"


const static char *kDonatePool1   = "gulf.moneroocean.stream";
const static char *kDonatePool2   = "gulf.moneroocean.stream";


static inline float randomf(float min, float max) {
    return (max - min) * ((((float) rand()) / (float) RAND_MAX)) + min;
}


DonateStrategy::DonateStrategy(int level, const char *user, xmrig::Algo algo, IStrategyListener *listener) :
    m_active(false),
    m_donateTime(level * 60 * 1000),
    m_idleTime((100 - level) * 60 * 1000),
    m_strategy(nullptr),
    m_listener(listener)
{
    uint8_t hash[200];
    char userId[65] = { 0 };

    xmrig::keccak(reinterpret_cast<const uint8_t *>(user), strlen(user), hash);
    Job::toHex(hash, 32, userId);

    if (algo == xmrig::CRYPTONIGHT) {
        m_pools.push_back(Pool(kDonatePool1, 10008, "47enSLKzx5zMR9Gm52zATpMBji8j4QiTDLd4Yue4qQAcBycoufEwAhQXd2MJWwfGUED1TbxAruBfvDNvW6QM7WXQKePXX5K", "xmrig10001p1", true, false));
        m_pools.push_back(Pool(kDonatePool1, 80,  "47enSLKzx5zMR9Gm52zATpMBji8j4QiTDLd4Yue4qQAcBycoufEwAhQXd2MJWwfGUED1TbxAruBfvDNvW6QM7WXQKePXX5K", "xmrig80p1", true, false));
        m_pools.push_back(Pool(kDonatePool2, 10001, "47enSLKzx5zMR9Gm52zATpMBji8j4QiTDLd4Yue4qQAcBycoufEwAhQXd2MJWwfGUED1TbxAruBfvDNvW6QM7WXQKePXX5K", "xmrig10001p2", true, false));
    }
    else if (algo == xmrig::CRYPTONIGHT_HEAVY) {
        m_pools.push_back(Pool(kDonatePool1, 10008, "47enSLKzx5zMR9Gm52zATpMBji8j4QiTDLd4Yue4qQAcBycoufEwAhQXd2MJWwfGUED1TbxAruBfvDNvW6QM7WXQKePXX5K", "xmrig10001p1", true, false));
    }
    else {
        m_pools.push_back(Pool(kDonatePool1, 80, "47enSLKzx5zMR9Gm52zATpMBji8j4QiTDLd4Yue4qQAcBycoufEwAhQXd2MJWwfGUED1TbxAruBfvDNvW6QM7WXQKePXX5K", "xmrig80p1", true, false));
    }

    for (Pool &pool : m_pools) {
        pool.adjust(algo);
    }

    if (m_pools.size() > 1) {
        m_strategy = new FailoverStrategy(m_pools, 1, 2, this, true);
    }
    else {
        m_strategy = new SinglePoolStrategy(m_pools.front(), 1, 2, this, true);
    }

    m_timer.data = this;
    uv_timer_init(uv_default_loop(), &m_timer);

    idle(m_idleTime * randomf(0.5, 1.5));
}


DonateStrategy::~DonateStrategy()
{
    delete m_strategy;
}


int64_t DonateStrategy::submit(const JobResult &result)
{
    return m_strategy->submit(result);
}


void DonateStrategy::connect()
{
    m_strategy->connect();
}


void DonateStrategy::stop()
{
    uv_timer_stop(&m_timer);
    m_strategy->stop();
}


void DonateStrategy::tick(uint64_t now)
{
    m_strategy->tick(now);
}


void DonateStrategy::onActive(IStrategy *strategy, Client *client)
{
    if (!isActive()) {
        uv_timer_start(&m_timer, DonateStrategy::onTimer, m_donateTime, 0);
    }

    m_active = true;
    m_listener->onActive(this, client);
}


void DonateStrategy::onJob(IStrategy *strategy, Client *client, const Job &job)
{
    m_listener->onJob(this, client, job);
}


void DonateStrategy::onPause(IStrategy *strategy)
{
}


void DonateStrategy::onResultAccepted(IStrategy *strategy, Client *client, const SubmitResult &result, const char *error)
{
    m_listener->onResultAccepted(this, client, result, error);
}


void DonateStrategy::idle(uint64_t timeout)
{
    uv_timer_start(&m_timer, DonateStrategy::onTimer, timeout, 0);
}


void DonateStrategy::suspend()
{
    m_strategy->stop();

    m_active = false;
    m_listener->onPause(this);

    idle(m_idleTime);
}


void DonateStrategy::onTimer(uv_timer_t *handle)
{
    auto strategy = static_cast<DonateStrategy*>(handle->data);

    if (!strategy->isActive()) {
        return strategy->connect();
    }

    strategy->suspend();
}
© 2018 GitHub, Inc.
Terms
Privacy
Security
Status
Help
Contact GitHub
API
Training
Shop
Blog
About
Press h to open a hovercard with more details.
