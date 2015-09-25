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

#include "DpdkCfg.hpp"

#include <cassert>

DpdkCfg::DpdkCfg(XMLElement* elem)
    : Cfg(elem),
        coremask(),
        memoryChannels()
{
    if (!elem)
        return;

    msg(MSG_INFO, "DpdkCfg: Start reading dpdk section");
    XMLNode::XMLSet<XMLElement*> set = elem->getElementChildren();

    for (XMLNode::XMLSet<XMLElement*>::iterator it = set.begin();
         it != set.end();
         it++) {
            XMLElement* e = *it;
            
            if (e->matches("coremask")) {
                if(!coremask.empty())
                    THROWEXCEPTION("device already set.");
                coremask = e->getContent();
            } else if (e->matches("memoryChannels")) {
                if(!memoryChannels.empty())
                    THROWEXCEPTION("memoryChannels already set.");
                memoryChannels = e->getContent();
            } else if (e->matches("next")) {
            } else {
                msg(MSG_FATAL, "Unknown dpdk config statement %s", e->getName().c_str());
                continue;
            }
    }

    if(coremask.empty()) THROWEXCEPTION("DpdkCfg: coremask not set in configuration!");
    if(memoryChannels.empty()) THROWEXCEPTION("DpdkCfg: memoryChannels not set in configuration!");

    createInstance();
}

DpdkCfg::~DpdkCfg()
{
    // TODO: Gracefully release resources assigned to dpdk
}

DpdkCfg* DpdkCfg::create(XMLElement* elem) {
    assert(elem);
    assert(elem->getName() == getName());
    return new DpdkCfg(elem);
}

Module* DpdkCfg::getInstance()
{
    if (instance == NULL)
        createInstance();

    return getInstanceAsModule();
}

Dpdk* DpdkCfg::createInstance()
{
    instance = new Dpdk(coremask, memoryChannels);

    return instance;
}

bool DpdkCfg::deriveFrom(Cfg* old)
{
    DpdkCfg* cfg = dynamic_cast<DpdkCfg*> (old);

    if(cfg)
        return deriveFrom(cfg);

    THROWEXCEPTION("Can't derive from DpdkCfg");
    return false;
}

bool DpdkCfg::deriveFrom(DpdkCfg* old)
{
    return ((coremask.compare(old->coremask) == 0) &&
            (memoryChannels.compare(old->memoryChannels) == 0));
}

Module* DpdkCfg::getInstanceAsModule()
{
    Module* m = dynamic_cast<Module*>(instance);
    return m;
}

void DpdkCfg::start(bool fail_if_already_running)
{
    if(instance == NULL)
        createInstance();

    if(instance)
        instance->start(fail_if_already_running);
}