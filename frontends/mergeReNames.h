/*
Copyright 2013-present Barefoot Networks, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef _FRONTENDS_P4_MERGERENAMES_H_
#define _FRONTENDS_P4_MERGERENAMES_H_

#include "ir/ir.h"
#include "frontends/common/resolveReferences/resolveReferences.h"
#include "frontends/p4/typeMap.h"

namespace P4 {

class MRenameMap {
    // Internal declaration name
    std::map<const IR::IDeclaration*, cstring> newName;
    std::set<const IR::P4Action*> inTable;  // all actions that appear in tables

 public:
    void setNewName(const IR::IDeclaration* decl, cstring name) {
        CHECK_NULL(decl);
        BUG_CHECK(!name.isNullOrEmpty(), "Empty name");
        LOG1("Renaming " << decl << " to " << name);
        if (newName.find(decl) != newName.end())
            BUG("%1%: already renamed", decl);
        newName.emplace(decl, name);
    }
    cstring getName(const IR::IDeclaration* decl) const {
        CHECK_NULL(decl);
        BUG_CHECK(newName.find(decl) != newName.end(), "%1%: no new name", decl);
        auto result = ::get(newName, decl);
        return result;
    }
    bool toRename(const IR::IDeclaration* decl) const {
        CHECK_NULL(decl);
        return newName.find(decl) != newName.end();
    }
    void foundInTable(const IR::P4Action* action)
    { inTable.emplace(action); }
    bool isInTable(const IR::P4Action* action)
    { return inTable.find(action) != inTable.end(); }
};

// Give unique names to various declarations to make it easier to
// move declarations around.
class MergeReNames : public PassManager {
 private:
    //cstring postName; not used ,just give it to child class
    MRenameMap    *renameMap;
 public:
    explicit MergeReNames(ReferenceMap* refMap,const char *p); 
};

// Finds and allocates new names for some symbols:
// Declaration_Variable, Declaration_Constant, Declaration_Instance,
// P4Table, P4Action.
class MFindSymbols : public Inspector {
    ReferenceMap *refMap;  // used to generate new names
    MRenameMap    *renameMap;
    cstring postName;  

 public:
    bool isTopLevel() const {
        return findContext<IR::P4Parser>() == nullptr &&
                findContext<IR::P4Control>() == nullptr;
    }
    MFindSymbols(ReferenceMap *refMap, MRenameMap *renameMap,const char * p) :
            refMap(refMap), renameMap(renameMap),postName(p)
    { CHECK_NULL(refMap); CHECK_NULL(renameMap); setName("MFindSymbols"); }
    void doDecl(const IR::Declaration* decl) {
        cstring newName = refMap->newName(decl->getName())+postName;  //now hard code
        renameMap->setNewName(decl, newName);
    }
    void postorder(const IR::Declaration_Variable* decl) override
    { doDecl(decl); }
    void postorder(const IR::Declaration_Constant* decl) override
    { doDecl(decl); }
    void postorder(const IR::Declaration_Instance* decl) override
    { if (!isTopLevel()) doDecl(decl); }
    void postorder(const IR::P4Table* decl) override
    { doDecl(decl); }
    void postorder(const IR::P4Action* decl) override
    { if (!isTopLevel()) doDecl(decl); }
};

class MRenameSymbols : public Transform {
    ReferenceMap *refMap;
    MRenameMap    *renameMap;

    IR::ID* getName() const;
 public:
    MRenameSymbols(ReferenceMap *refMap, MRenameMap *renameMap) :
            refMap(refMap), renameMap(renameMap)
    { CHECK_NULL(refMap); CHECK_NULL(renameMap); setName("MRenameSymbols"); }
    const IR::Node* postorder(IR::Declaration_Variable* decl) override;
    const IR::Node* postorder(IR::Declaration_Constant* decl) override;
    const IR::Node* postorder(IR::PathExpression* expression) override;
    const IR::Node* postorder(IR::Declaration_Instance* decl) override;
    const IR::Node* postorder(IR::P4Table* decl) override;
    const IR::Node* postorder(IR::P4Action* decl) override;
    const IR::Node* postorder(IR::Parameter* param) override;
};

// Finds parameters for actions that will be given unique names
class MFindParameters : public Inspector {
    ReferenceMap* refMap;  // used to generate new names
    MRenameMap*    renameMap;
    cstring postName;

    // If all is true then rename all parameters, else rename only
    // directional parameters
    void doParameters(const IR::ParameterList* pl, bool all) {
        for (auto p : pl->parameters) {
            if (!all && p->direction == IR::Direction::None)
                continue;
            cstring newName = refMap->newName(p->name)+postName;  //now hard code zoe
            renameMap->setNewName(p, newName);
        }
    }
 public:
    MFindParameters(ReferenceMap* refMap, MRenameMap* renameMap,const char *p) :
            refMap(refMap), renameMap(renameMap),postName(p)
    { CHECK_NULL(refMap); CHECK_NULL(renameMap); setName("MFindParameters"); }
    void postorder(const IR::P4Action* action) override {
        bool inTable = renameMap->isInTable(action);
        doParameters(action->parameters, !inTable);
    }
};

// Give each parameter of a table and action a new unique name
class MUniqueParameters : public PassManager {
 private:
    //cstring postName;
    MRenameMap    *renameMap;
 public:
    MUniqueParameters(ReferenceMap* refMap, TypeMap* typeMap,const char *p);
};

}  // namespace P4

#endif /* _FRONTENDS_P4_MERGERENAMES_H_ */
