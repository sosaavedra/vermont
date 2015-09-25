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

#include "Dpdk.hpp"

#include <rte_eal.h>
#include <rte_ethdev.h>
#include <rte_config.h>

Dpdk::Dpdk(std::string coremask, std::string memoryChannels)
        : coremask(coremask), memoryChannels(memoryChannels)
{
    prepare();
}

Dpdk::~Dpdk()
{
// TODO
}

void Dpdk::prepare()
{
    msg(MSG_DEBUG, "Dpdk: Initializing dpdk");
    int result;
    // mandatory eal's parameters
    char c[] = "-c"; // hexadecimal bit mask of the cores to run
    char n[] = "-n"; // number of memory channels per processor socket

    msg(MSG_DEBUG, "Dpdk: Checking for hugepages");
    result = rte_eal_has_hugepages();

    if(result == 0){
        THROWEXCEPTION("Hugepages are disabled");
    }

    char *ealargs[] = {(char *) "", c, (char *) coremask.c_str(),
                       n, (char *) memoryChannels.c_str()};

    msg(MSG_DEBUG, "Dpdk: EAL init");
    result = rte_eal_init(5, ealargs);

    if(result < 0){
        THROWEXCEPTION("Cannot init EAL");
    }

    nb_ports = rte_eth_dev_count();
    if (nb_ports < 2 || (nb_ports & 1))
    {
        THROWEXCEPTION("Error: number of ports must be even. Number of ports: %d", nb_ports);
    }

    mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_BUFS * nb_ports,
                                        MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

    if (mbuf_pool == NULL)
    {
        THROWEXCEPTION("Error: cannot create mbuf pool");
    }

    for (uint8_t port_id = 0; port_id < nb_ports; port_id++)
    {
        if (port_init(port_id) != 0)
        {
            THROWEXCEPTION("Error: cannot init port %" PRIu8, port_id);
        }
    }

    if (rte_lcore_count() > 1)
    {
        msg(MSG_DEBUG, "Too many lcores enabled. Only 1 used");
    }
}

void Dpdk::performStart()
{
    msg(MSG_DEBUG, "Dpdk::performStart()");
    pthread_create(&thread, NULL,
                   lcore_main, (void *)NULL);
}

void Dpdk::performShutdown()
{
    msg(MSG_DEBUG, "Dpdk::performShutdown()");
}

int Dpdk::port_init(uint8_t port)
{
    struct rte_eth_conf port_conf = {};
    struct rte_eth_rxmode rxmode = {};
    rxmode.max_rx_pkt_len = ETHER_MAX_LEN;
    port_conf.rxmode = rxmode;
    const uint16_t rx_rings = 1, tx_rings = 1;
    int retval;
    uint16_t q;

    if (port >= rte_eth_dev_count())
    {
        return -1;
    }

    retval = rte_eth_dev_configure(port, rx_rings, tx_rings, &port_conf);

    if (retval != 0)
    {
        return retval;
    }

    for (q = 0; q < rx_rings; q++)
    {
        retval = rte_eth_rx_queue_setup(port, q, RX_RING_SIZE,
                        rte_eth_dev_socket_id(port), NULL, mbuf_pool);

        if (retval < 0)
        {
            return retval;
        }
    }

    for (q = 0; q < tx_rings; q++)
    {
        retval = rte_eth_tx_queue_setup(port, q, TX_RING_SIZE,
                        rte_eth_dev_socket_id(port), NULL);

        if (retval < 0)
        {
            return retval;
        }
    }

    retval = rte_eth_dev_start(port);

    if (retval < 0)
    {
        return retval;
    }

    struct ether_addr addr;

    rte_eth_macaddr_get(port, &addr);

    printf("\nPort %u MAC: %02" PRIx8 " %02" PRIx8 " %02" PRIx8
                       " %02" PRIx8 " %02" PRIx8 " %02" PRIx8 "\n",
                   (unsigned)port,
                    addr.addr_bytes[0], addr.addr_bytes[1],
                    addr.addr_bytes[2], addr.addr_bytes[3],
                    addr.addr_bytes[4], addr.addr_bytes[5]);

    rte_eth_promiscuous_enable(port);

    return 0;
};

void* Dpdk::lcore_main(void *)
{
    const uint8_t nb_ports = rte_eth_dev_count();
    uint8_t port;

    for (port = 0; port < nb_ports; port++) {
        int socket_id = rte_eth_dev_socket_id(port);

        if (socket_id > 0 && socket_id != (int) rte_socket_id()) {
            printf("WARNING, port %u is on remote NUMA node to polling thread.\n\tPerformance will not be optimal.\n",
                   port);
        }
    }

    printf("\nCore %u forwarding packets. [Ctrl+C to quit]\n", rte_lcore_id());

    for (;;)
    {
        for (port = 0; port < nb_ports; port++)
        {
            struct rte_mbuf* bufs[BURST_SIZE];
            const uint16_t nb_rx = rte_eth_rx_burst(port, 0, bufs, BURST_SIZE);
            printf("\nPackets received: %d\n", nb_rx);

            if (unlikely(nb_rx == 0))
            {
                continue;
            }


            const uint16_t nb_tx = rte_eth_tx_burst(port ^ 1, 0, bufs, nb_rx);
            printf("\nPacket sent: %d\n", nb_rx);

            if (unlikely(nb_tx < nb_rx))
            {
                uint16_t buf;
                for (buf = nb_tx; buf < nb_rx; buf++)
                {
                    rte_pktmbuf_free(bufs[buf]);
                }
            }

        }
    }
};
