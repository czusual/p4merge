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

#include "mergeReNames.h"
#include "frontends/p4/methodInstance.h"
#include "frontends/p4/typeChecking/typeChecker.h"

namespace P4 {

#if 0
namespace {

class FindActionCalls : public Inspector {
    ReferenceMap* refMap;
    TypeMap* typeMap;
    RenameMap* renameMap;
 public:
    explicit FindActionCalls(ReferenceMap* refMap, TypeMap* typeMap, RenameMap* renameMap) :
            refMap(refMap), typeMap(typeMap), renameMap(renameMap)
    { CHECK_NULL(refMap); CHECK_NULL(typeMap); CHECK_NULL(renameMap); }

    void postorder(const IR::MethodCallExpression* expression) {
        auto mi = MethodInstance::resolve(expression, refMap, typeMap);
        if (!mi->is<P4::ActionCall>())
            return;
        auto ac = mi->to<P4::ActionCall>();

        auto table = findContext<IR::P4Table>();
        if (table != nullptr)
            renameMap->foundInTable(ac->action);
    }
};

}  // namespace
#endif 
// Add a @name annotation ONLY if it does not already exist.
// Otherwise do nothing.


MergeReNames::MergeReNames(ReferenceMap* refMap) {
    setStopOnError(true);
    setName("MergeReNames");
    CHECK_NULL(refMap);
    passes.emplace_back(new ResolveReferences(refMap));   
    passes.emplace_back(new MRenameSymbols(refMap));
}

#if 0
UniqueParameters::UniqueParameters(ReferenceMap* refMap, TypeMap* typeMap) :
        renameMap(new RenameMap) {
    setName("UniqueParameters");
    CHECK_NULL(refMap); CHECK_NULL(typeMap);
    passes.emplace_back(new TypeChecking(refMap, typeMap));
    passes.emplace_back(new FindActionCalls(refMap, typeMap, renameMap));
    passes.emplace_back(new FindParameters(refMap, renameMap));
    passes.emplace_back(new RenameSymbols(refMap, renameMap));
    passes.emplace_back(new ClearTypeMap(typeMap));
}
#endif
/**************************************************************************/

/*
IR::ID* MRenameSymbols::getName() const {
    auto orig = getOriginal<IR::IDeclaration>();
    
    cstring tore=
    auto name = new IR::ID(orig->getName().srcInfo, orig->getName().name, orig->getName().originalName);
    return name;
}
*/
const IR::Node* MRenameSymbols::postorder(IR::Declaration_Variable* decl) {
    decl->name.setReName(postName);
    return decl;
}

const IR::Node* MRenameSymbols::postorder(IR::Declaration_Constant* decl) {
    decl->name.setReName(postName);
    return decl;
}

const IR::Node* MRenameSymbols::postorder(IR::Parameter* param) {
    param->name.setReName(postName);
    return param;
}
#if 0
const IR::Node* RenameSymbols::postorder(IR::PathExpression* expression) {
    auto decl = refMap->getDeclaration(expression->path, true);
    if (!renameMap->toRename(decl))
        return expression;
    // This should be a local name.
    BUG_CHECK(!expression->path->absolute,
              "%1%: renaming absolute path", expression);
    auto newName = renameMap->getName(decl);
    auto name = IR::ID(expression->path->name.srcInfo, newName,
                       expression->path->name.originalName);
    auto result = new IR::PathExpression(name);
    return result;
}
#endif
const IR::Node* MRenameSymbols::postorder(IR::Declaration_Instance* decl) {
    decl->name.setReName(postName);
    return decl;
}

const IR::Node* MRenameSymbols::postorder(IR::P4Table* decl) {
    decl->name.setReName(postName);
    return decl;
}

const IR::Node* MRenameSymbols::postorder(IR::P4Action* decl) {
    decl->name.setReName(postName);
    return decl;
}

}  // namespace P4
