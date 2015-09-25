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

#ifndef DPDKCFG_HPP
#define	DPDKCFG_HPP

#include "Dpdk.hpp"

#include <string>
#include <core/XMLElement.h>
#include <core/Cfg.h>

/**
 * This class holds the <dpdk> ... </dpdk> information from the config
 */

class Dpdk;

class DpdkCfg
    : public Cfg
{
public:
    friend class ConfigManager;

    virtual void start(bool fail_if_already_running = true); // Implemented (?))
    virtual void shutdown(bool fail_if_not_running = true, bool shutdownProperly = false) { instance->performShutdown(); }
    virtual void onReconfiguration1() { THROWEXCEPTION("Not supported"); }
    virtual void onReconfiguration2() { THROWEXCEPTION("Not supported"); }
    virtual Module* getQueueInstance()  { THROWEXCEPTION("Not supported"); return NULL; }
    virtual void freeInstance() { THROWEXCEPTION("Not supported"); }
    virtual void setupWithoutSuccessors() { THROWEXCEPTION("Not supported"); }
    virtual void connectInstances(Cfg* other) { /* TODO */ }
    virtual void disconnectInstances()  { /* TODO */ }
    virtual void transferInstance(Cfg* other) { THROWEXCEPTION("Not supported"); }
    virtual Module* getInstanceAsModule();

    virtual DpdkCfg* create(XMLElement* elem);
    virtual ~DpdkCfg();

    virtual string getName() { return "dpdk"; }

    virtual Module* getInstance();
    virtual Dpdk* createInstance();

    virtual bool deriveFrom(DpdkCfg* old);
    virtual bool deriveFrom(Cfg* old);

protected:
    DpdkCfg(XMLElement* elem);

private:
    Dpdk* instance;
    std::string coremask;
    std::string memoryChannels;
};


#endif	/* DPDKCFG_HPP */
