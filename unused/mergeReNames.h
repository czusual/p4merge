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
#include "lib/cstring.h"

namespace P4 {

class MRenameMap {
    // Internal declaration name
    std::map<const IR::IDeclaration*, cstring> reName;
    std::set<const IR::P4Action*> inTable;  // all actions that appear in tables

 public:
    void seReName(const IR::IDeclaration* decl, cstring name) {
        CHECK_NULL(decl);
        BUG_CHECK(!name.isNullOrEmpty(), "Empty name");
        LOG1("MRenaming " << decl << " to " << name);
        if (reName.find(decl) != reName.end())
            BUG("%1%: already mrenamed", decl);
        reName.emplace(decl, name);
    }
    cstring getName(const IR::IDeclaration* decl) const {
        CHECK_NULL(decl);
        BUG_CHECK(reName.find(decl) != reName.end(), "%1%: no re name", decl);
        auto result = ::get(reName, decl);
        return result;
    }
    bool toRename(const IR::IDeclaration* decl) const {
        CHECK_NULL(decl);
        return reName.find(decl) != reName.end();
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
    //cstring postName = nullptr;
 public:
    //explicit MergeReNames(ReferenceMap* refMap);
    explicit MergeReNames(ReferenceMap* refMap);
};

// Finds and allocates new names for some symbols:
// Declaration_Variable, Declaration_Constant, Declaration_Instance,
// P4Table, P4Action.
#if 0
class MFindSymbols : public Inspector {
    ReferenceMap *refMap;  // used to generate new names
    MRenameMap    *renameMap;

 public:
    bool isTopLevel() const {
        return findContext<IR::P4Parser>() == nullptr &&
                findContext<IR::P4Control>() == nullptr;
    }
    FindSymbols(ReferenceMap *refMap, RenameMap *renameMap) :
            refMap(refMap), renameMap(renameMap)
    { CHECK_NULL(refMap); CHECK_NULL(renameMap); setName("MFindSymbols"); }
    void doDecl(const IR::Declaration* decl) {
        cstring newName = refMap->newName(decl->getName());
        renameMap->setReName(decl, newName);
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
#endif
class MRenameSymbols : public Transform {
    ReferenceMap *refMap;
    char postName[5]="_v1";

    //IR::ID* getName() const;
 public:
    MRenameSymbols(ReferenceMap *refMap) :
            refMap(refMap)
    { CHECK_NULL(refMap); setName("MRenameSymbols"); }
    const IR::Node* postorder(IR::Declaration_Variable* decl) override;
    const IR::Node* postorder(IR::Declaration_Constant* decl) override;
    //const IR::Node* postorder(IR::PathExpression* expression) override;
    const IR::Node* postorder(IR::Declaration_Instance* decl) override;
    const IR::Node* postorder(IR::P4Table* decl) override;
    const IR::Node* postorder(IR::P4Action* decl) override;
    const IR::Node* postorder(IR::Parameter* param) override;
};

#if 0

// Finds parameters for actions that will be given unique names
class FindParameters : public Inspector {
    ReferenceMap* refMap;  // used to generate new names
    RenameMap*    renameMap;

    // If all is true then rename all parameters, else rename only
    // directional parameters
    void doParameters(const IR::ParameterList* pl, bool all) {
        for (auto p : pl->parameters) {
            if (!all && p->direction == IR::Direction::None)
                continue;
            cstring newName = refMap->newName(p->name);
            renameMap->setNewName(p, newName);
        }
    }
 public:
    FindParameters(ReferenceMap* refMap, RenameMap* renameMap) :
            refMap(refMap), renameMap(renameMap)
    { CHECK_NULL(refMap); CHECK_NULL(renameMap); setName("FindParameters"); }
    void postorder(const IR::P4Action* action) override {
        bool inTable = renameMap->isInTable(action);
        doParameters(action->parameters, !inTable);
    }
};

// Give each parameter of a table and action a new unique name
class UniqueParameters : public PassManager {
 private:
    RenameMap    *renameMap;
 public:
    UniqueParameters(ReferenceMap* refMap, TypeMap* typeMap);
};
#endif

}  // namespace P4


#endif /* _FRONTENDS_P4_UNIQUENAMES_H_ */
