/*
 * Vermont Configuration Subsystem
 * Copyright (C) 2009 Vermont Project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef DPDK_HPP
#define DPDK_HPP

#include "core/Module.h"
#include <string>
#include <pthread.h>

#define RX_RING_SIZE 128
#define TX_RING_SIZE 512

#define NUM_BUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

class Dpdk
    : public Module
{
public:
    friend class DpdkCfg;

    virtual ~Dpdk();

    virtual void prepare();
    virtual void performStart();
    virtual void performShutdown();
protected:
    Dpdk(std::string coremask, std::string memoryChannels);

private:
    virtual int port_init(uint8_t port);
    static void* lcore_main(void *);

    std::string coremask;
    std::string memoryChannels;
    struct rte_mempool* mbuf_pool;
    unsigned nb_ports;
    pthread_t thread;
};

#endif /* DPDK_HPP */

