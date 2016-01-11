// University of Illinois/NCSA
// Open Source License
//
// Copyright (c) 2013-2015, Advanced Micro Devices, Inc.
// All rights reserved.
//
// Developed by:
//
//     HSA Team
//
//     Advanced Micro Devices, Inc
//
//     www.amd.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal with
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
//
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimers.
//
//     * Redistributions in binary form must reproduce the above copyright notice,
//       this list of conditions and the following disclaimers in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the names of the LLVM Team, University of Illinois at
//       Urbana-Champaign, nor the names of its contributors may be used to
//       endorse or promote products derived from this Software without specific
//       prior written permission.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
// SOFTWARE.

#include "HSAILValidatorBase.h"
#include "HSAILExtManager.h"
#include "HSAILImageExt.h"
#include "HSAILParser.h"

using namespace HSAIL_PROPS;

namespace HSAIL_ASM {

const ExtManager& noExtensions()
{
    static const ExtManager noExtensions;
    return noExtensions;
}

ExtManager& registeredExtensionsNC()
{
    static ExtManager registeredExtensions;
    
    // Static registration of HSAIL IMAGE extension does not work on Windows.
    // The following line of code explicitly uses IMAGE extension code and 
    // this forces static registration on Windows.
    hsail::image::getExtension();

    return registeredExtensions;
}

const ExtManager& registeredExtensions()
{
    return registeredExtensionsNC();
}

bool registerExtension(const Extension* e)
{
    return registeredExtensionsNC().registerExtension(e);
}

//============================================================================
//============================================================================
//============================================================================
// Validator of 'CORE' instructions (autogenerated)

#include "HSAILInstValidation_core_gen.hpp"

//============================================================================
//============================================================================
//============================================================================
// Utilities for 'CORE' Instructions (used by ExtManager)

static unsigned getCoreOperandType(Inst inst, unsigned operandIdx, unsigned machineModel, unsigned profile)
{
    assert(inst);
    assert(operandIdx < MAX_OPERANDS_NUM);
    assert(machineModel == BRIG_MACHINE_SMALL || machineModel == BRIG_MACHINE_LARGE);
    assert(profile == BRIG_PROFILE_BASE  || profile == BRIG_PROFILE_FULL);

    return InstValidator(machineModel, profile).getOperandType(inst, operandIdx);
}

static const char* preValidateCoreInst(Inst inst, unsigned machineModel, unsigned profile)
{
    assert(inst);
    assert(machineModel == BRIG_MACHINE_SMALL || machineModel == BRIG_MACHINE_LARGE);
    assert(profile == BRIG_PROFILE_BASE  || profile == BRIG_PROFILE_FULL);

    return InstValidator(machineModel, profile).preValidateInst(inst);
}

static unsigned getCoreDefWidth(Inst inst, unsigned machineModel, unsigned profile)
{
    assert(inst);
    assert(machineModel == BRIG_MACHINE_SMALL || machineModel == BRIG_MACHINE_LARGE);
    assert(profile == BRIG_PROFILE_BASE  || profile == BRIG_PROFILE_FULL);

    return InstValidator(machineModel, profile).getDefWidth(inst);
}

static unsigned getCoreDefRounding(Inst inst, unsigned machineModel, unsigned profile)
{
    assert(inst);
    assert(machineModel == BRIG_MACHINE_SMALL || machineModel == BRIG_MACHINE_LARGE);
    assert(profile == BRIG_PROFILE_BASE  || profile == BRIG_PROFILE_FULL);

    return InstValidator(machineModel, profile).getDefRounding(inst);
}

//============================================================================
//============================================================================
//============================================================================
// ExtManager implementation 

ExtManager::ExtManager() : registrationComplete(false) {}

ExtManager::ExtManager(const ExtManager& mgr) : extension(mgr.extension), isEnabled(mgr.isEnabled), registrationComplete(true)
{ 
    assert(extension.size() == isEnabled.size());
}

const ExtManager& ExtManager::operator=(const ExtManager& mgr)
{
    if (this != &mgr)
    {
        assert(mgr.extension.size() == mgr.isEnabled.size());

        extension = mgr.extension;
        isEnabled = mgr.isEnabled;
        registrationComplete = true;
    }

    return *this;
}

bool ExtManager::registerExtension(const Extension* e)
{
    assert(e);
    assert(!registrationComplete);

    if (registrationComplete) return false;

    for (unsigned i = 0; i < size(); ++i)
    {
        if (strcmp(e->getName(), extension[i]->getName()) == 0) return false;
    }

    extension.push_back(e);
    isEnabled.push_back(true);

    return true;
}

bool ExtManager::registerExtensions(const Extension** e)
{
    assert(e);
    assert(!registrationComplete);

    if (registrationComplete) return false;

    bool ok = true;
    for (const Extension** p = e; *p != 0; ++p)
    {
        ok &= registerExtension(*p);
    }

    return ok;
}

int ExtManager::getIdx(const char* name) const
{
    assert(name);

    for (unsigned i = 0; i < size(); ++i)
    {
        if (strcmp(name, extension[i]->getName()) == 0) return i;
    }
    return -1;
}

int ExtManager::getIdx(const string& name) const { return getIdx(name.c_str()); }

// Search for an extension which can handle opcode starting with a prefix in the form <vendor>_<extension>
const Extension* ExtManager::getByPrefix(const string& prefix) const
{
    for (unsigned i = 0; i < size(); ++i)
    {
        if (isEnabled[i] && extension[i]->isMnemoPrefix(prefix)) return extension[i];
    }
    return 0;
}

// Search for an extension which can handle non-standard value 'val' of property 'prop'
const Extension* ExtManager::getByProp(unsigned prop, unsigned val) const
{
    assert(PROP_MINID < prop && prop < PROP_MAXID);

    for (unsigned i = 0; i < size(); ++i)
    {
        if (isEnabled[i] && extension[i]->propVal2mnemo(prop, val) != 0) return extension[i];
    }
    return 0;
}

const Extension* ExtManager::get(const char* name)   const { int idx = getIdx(name); return (idx >= 0)? extension[idx] : 0; }
const Extension* ExtManager::get(const string& name) const { int idx = getIdx(name); return (idx >= 0)? extension[idx] : 0; }

const Extension* ExtManager::getEnabled(const char* name) const     { int idx = getIdx(name); return (idx >= 0 && isEnabled[idx])? extension[idx] : 0; }
const Extension* ExtManager::getEnabled(const string& name) const   { int idx = getIdx(name); return (idx >= 0 && isEnabled[idx])? extension[idx] : 0; }
const Extension* ExtManager::getEnabled(unsigned opcode) const      { return getByProp(PROP_OPCODE, opcode); }

bool ExtManager::enabled(const char* name)   const { return getEnabled(name) != 0; }
bool ExtManager::enabled(const string& name) const { return getEnabled(name) != 0; }

void ExtManager::getEnabled(vector<string>& name) const
{
    assert(name.size() == 0);

    for (unsigned i = 0; i < size(); ++i) if (isEnabled[i]) name.push_back(extension[i]->getName());
}

bool ExtManager::hasEnabled() const
{
    for (unsigned i = 0; i < size(); ++i) if (isEnabled[i]) return true;
    return false;
}

bool ExtManager::enable(const char* name, bool flag /* =true */)
{
    assert(name);

    int idx = getIdx(name);
    if (idx >= 0) isEnabled[idx] = flag;
    return idx >= 0;
}

bool ExtManager::enable(const string& name, bool flag /* =true */)
{
    return enable(name.c_str(), flag);
}

bool ExtManager::disable(const char*   name) { return enable(name, false); }
bool ExtManager::disable(const string& name) { return enable(name, false); }

void ExtManager::enableAll()  { for (unsigned i = 0; i < size(); ++i) isEnabled[i] = true; }
void ExtManager::disableAll() { for (unsigned i = 0; i < size(); ++i) isEnabled[i] = false; }

bool ExtManager::isMnemoPrefix(const string& prefix) const { return getByPrefix(prefix) != 0; }

Inst ExtManager::parseExtInstMnemo(Scanner& scanner, Brigantine& bw, int* vx) const
{
    assert(scanner.peek().kind() == EExtInstName);

    // Parse mnemo prefix adding suffices until there is an extension which can handle it.
    // This prefix has the form <vendor>_<extension>

    string prefix = scanner.scan().text();
    while (!isMnemoPrefix(prefix) && scanner.peek().kind() == EExtInstSuff) 
    {
        prefix += scanner.scan().text();
    }

    // Parse remaining part of instruction mnemo (typically in the form <opcode>_<suff>)

    if (const Extension* e = getByPrefix(prefix)) return e->parseInstMnemo(prefix, scanner, bw, vx);

    // Enabled extensions failed to parse this mnemo.
    // Search for a disabled extension which might have handled it
    const char* s = 0;
    for (unsigned i = 0; i < size(); ++i)
    {
        if (!isEnabled[i] && (s = extension[i]->matchInstMnemo(prefix)) != 0)
        {
            scanner.syntaxError(string("Instruction \"") + s + "\" cannot be used (extension \"" + extension[i]->getName() + "\" is not enabled)");
            return Inst();
        }
    }

    // Extensions (including disabled) cannot handle this mnemo

    if (!hasEnabled()) scanner.scan(); // show offending token
    scanner.syntaxError("Undefined instruction");

    return Inst();
}

const char* ExtManager::propVal2mnemo(unsigned prop, unsigned val) const
{
    assert(PROP_MINID < prop && prop < PROP_MAXID);

    if (const char *res = HSAIL_ASM::stdPropVal2mnemo(prop, val)) return res;
    else if (const Extension* e = getByProp(prop, val)) return e->propVal2mnemo(prop, val);
    return 0;
}

const char* ExtManager::propVal2str(unsigned prop, unsigned val) const
{
    assert(PROP_MINID < prop && prop < PROP_MAXID);

    if (const char *res = HSAIL_ASM::stdPropVal2str(prop, val)) return res;
    else if (const Extension* e = getByProp(prop, val)) return e->propVal2mnemo(prop, val);
    return 0;
}

const string ExtManager::extEnum2str(unsigned prop, unsigned val) const
{
    assert(PROP_MINID < prop && prop < PROP_MAXID);

    if (const Extension* e = getByProp(prop, val)) return e->propVal2enum(prop, val);
    return "";
}

unsigned ExtManager::getOperandType(Inst inst, unsigned operandIdx, unsigned machineModel, unsigned profile) const
{
    assert(inst);
    assert(operandIdx < MAX_OPERANDS_NUM);
    assert(machineModel == BRIG_MACHINE_SMALL || machineModel == BRIG_MACHINE_LARGE);
    assert(profile == BRIG_PROFILE_BASE  || profile == BRIG_PROFILE_FULL);

    if (isCoreInst(inst))
    {
        return getCoreOperandType(inst, operandIdx, machineModel, profile);
    }
    else if (const Extension* e = getByProp(PROP_OPCODE, inst.opcode())) 
    {
        return e->getOperandType(inst, operandIdx, machineModel, profile);
    }

    return BRIG_TYPE_INVALID;
}

const char* ExtManager::preValidateInst(Inst inst, unsigned machineModel, unsigned profile) const
{
    assert(inst);
    assert(machineModel == BRIG_MACHINE_SMALL || machineModel == BRIG_MACHINE_LARGE);
    assert(profile == BRIG_PROFILE_BASE  || profile == BRIG_PROFILE_FULL);

    if (isCoreInst(inst))
    {
        return preValidateCoreInst(inst, machineModel, profile);
    }
    else if (const Extension* e = getByProp(PROP_OPCODE, inst.opcode())) 
    {
        return e->preValidateInst(inst, machineModel, profile);
    }

    return 0;
}

unsigned ExtManager::getDefWidth(Inst inst, unsigned machineModel, unsigned profile) const
{
    assert(inst);
    assert(machineModel == BRIG_MACHINE_SMALL || machineModel == BRIG_MACHINE_LARGE);
    assert(profile == BRIG_PROFILE_BASE  || profile == BRIG_PROFILE_FULL);

    if (isCoreInst(inst))
    {
        return getCoreDefWidth(inst, machineModel, profile);
    }
    else if (const Extension* e = getByProp(PROP_OPCODE, inst.opcode())) 
    {
        return e->getDefWidth(inst, machineModel, profile);
    }
    
    return BRIG_WIDTH_NONE;
}

unsigned ExtManager::getDefRounding(Inst inst, unsigned machineModel, unsigned profile) const
{
    assert(inst);
    assert(machineModel == BRIG_MACHINE_SMALL || machineModel == BRIG_MACHINE_LARGE);
    assert(profile == BRIG_PROFILE_BASE  || profile == BRIG_PROFILE_FULL);

    if (isCoreInst(inst))
    {
        return getCoreDefRounding(inst, machineModel, profile);
    }
    else if (const Extension* e = getByProp(PROP_OPCODE, inst.opcode()))
    {
        return e->getDefRounding(inst, machineModel, profile);
    }

    return BRIG_ROUND_NONE;
}

unsigned ExtManager::getDstOperandsNum(unsigned opcode) const
{
    if (isCoreOpcode(opcode))
    {
        return getCoreDstOperandsNum(opcode);
    }
    else if (const Extension* e = getByProp(PROP_OPCODE, opcode))
    {
        return e->getDstOperandsNum(opcode);
    }

    return 1;
}

int ExtManager::getVXIndex(unsigned opcode) const
{
    int vx = -1;

    if (isCoreOpcode(opcode))
    {
        vx = getCoreVXIndex(opcode);
    }
    else if (const Extension* e = getByProp(PROP_OPCODE, opcode))
    {
        vx = e->getVXIndex(opcode);
    }

    return vx;
}

string ExtManager::getExtInstMnemo(Inst inst) const
{
    assert(inst);

    if (const Extension* e = getByProp(PROP_OPCODE, inst.opcode())) return e->getMnemo(inst);
    return "";
}

bool ExtManager::isDisabledProp(unsigned prop, unsigned val, string& valName, string& extName) const
{
    assert(PROP_MINID < prop && prop < PROP_MAXID);

    for (unsigned i = 0; i < size(); ++i)
    {
        if (!isEnabled[i] && extension[i]->propVal2mnemo(prop, val) != 0)
        {
            valName = extension[i]->propVal2mnemo(prop, val);
            extName = extension[i]->getName();
            return true;
        }
    }
    return false;
}

bool ExtManager::validateInst(Inst inst, unsigned model, unsigned profile) const
{
    assert(inst);
    assert(model == BRIG_MACHINE_SMALL || model == BRIG_MACHINE_LARGE);
    assert(profile == BRIG_PROFILE_BASE  || profile == BRIG_PROFILE_FULL);

    if (isCoreInst(inst))
    {
        InstValidator(model, profile).validateInst(inst);
        return true;
    }
    else if (const Extension* e = getByProp(PROP_OPCODE, inst.opcode()))
    {
        return e->validateInst(inst, model, profile);
    }
     
    return false;
}

} // end namespace