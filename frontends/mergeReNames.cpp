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

namespace {

class MFindActionCalls : public Inspector {
    ReferenceMap* refMap;
    TypeMap* typeMap;
    MRenameMap* renameMap;
 public:
    explicit MFindActionCalls(ReferenceMap* refMap, TypeMap* typeMap, MRenameMap* renameMap) :
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

// Add a @name annotation ONLY if it does not already exist.
// Otherwise do nothing.
static const IR::Annotations*
addNameAnnotation(cstring name, const IR::Annotations* annos) {
    if (annos == nullptr)
        annos = IR::Annotations::empty;
    return annos->addAnnotationIfNew(IR::Annotation::nameAnnotation,
                                     new IR::StringLiteral(name));
}

MergeReNames::MergeReNames(ReferenceMap* refMap,const char *p) : renameMap(new MRenameMap) {
    setStopOnError(true);
    setName("MergeReNames");
    CHECK_NULL(refMap);
    passes.emplace_back(new ResolveReferences(refMap));
    passes.emplace_back(new MFindSymbols(refMap, renameMap,p));
    passes.emplace_back(new MRenameSymbols(refMap, renameMap,p));
}

MUniqueParameters::MUniqueParameters(ReferenceMap* refMap, TypeMap* typeMap,const char *p) :
        renameMap(new MRenameMap) {
    setName("MUniqueParameters");
    CHECK_NULL(refMap); CHECK_NULL(typeMap);
    passes.emplace_back(new TypeChecking(refMap, typeMap));
    passes.emplace_back(new MFindActionCalls(refMap, typeMap, renameMap));
    passes.emplace_back(new MFindParameters(refMap, renameMap,p));
    passes.emplace_back(new MRenameSymbols(refMap, renameMap,p));
    passes.emplace_back(new ClearTypeMap(typeMap));
}

/**************************************************************************/

IR::ID* MRenameSymbols::getName() const {
    auto orig = getOriginal<IR::IDeclaration>();
    if (!renameMap->toRename(orig))
        return nullptr;
    auto newName = renameMap->getName(orig);
    auto name = new IR::ID(orig->getName().srcInfo, newName, orig->getName().originalName);
    return name;
}

const IR::Node* MRenameSymbols::postorder(IR::Declaration_Variable* decl) {
    auto name = getName();
    if (name != nullptr && *name != decl->name) {
        if (decl->type->is<IR::Type_ValueSet>()) {
            auto annos = addNameAnnotation(decl->name, decl->annotations);
            decl->annotations = annos;
        }
        decl->name = *name;
    }
    return decl;
}

const IR::Node* MRenameSymbols::postorder(IR::Declaration_Constant* decl) {
    auto name = getName();
    if (name != nullptr && *name != decl->name)
        decl->name = *name;
    return decl;
}

const IR::Node* MRenameSymbols::postorder(IR::Parameter* param) {
    auto name = getName();
    if (name != nullptr && *name != param->name.name)
        param->name = IR::ID(param->name.srcInfo, *name, param->name.originalName);
    return param;
}

const IR::Node* MRenameSymbols::postorder(IR::PathExpression* expression) {
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

const IR::Node* MRenameSymbols::postorder(IR::Declaration_Instance* decl) {
    auto name = getName();
    if (name != nullptr && *name != decl->name) {
        auto annos = addNameAnnotation(decl->name, decl->annotations);
        decl->name = *name;
        decl->annotations = annos;
    }
    return decl;
}

const IR::Node* MRenameSymbols::postorder(IR::P4Table* decl) {
    auto name = getName();
    if (name != nullptr && *name != decl->name) {
        auto annos = addNameAnnotation(decl->name, decl->annotations);
        decl->name = *name;
        decl->annotations = annos;
    }
    return decl;
}

const IR::Node* MRenameSymbols::postorder(IR::P4Action* decl) {
    auto name = getName();
    if (name != nullptr && *name != decl->name) {
        auto annos = addNameAnnotation(decl->name, decl->annotations);
        decl->name = *name;
        decl->annotations = annos;
    }
    return decl;
}

const IR::Node* MRenameSymbols::postorder(IR::ParserState* s) {
    auto name = getName();
    if (name != nullptr && *name != s->name) {
        auto annos = addNameAnnotation(s->name, s->annotations);
        s->name = *name;
        s->annotations = annos;
    }
    return s;
}

/*
const IR::Node* MRenameSymbols::postorder(IR::Type_Header* h) {
    auto name = getName();
    if (name != nullptr && *name != h->name) {
        auto annos = addNameAnnotation(h->name, h->annotations);
        h->name = *name;
        h->annotations = annos;
    }
    return h;
}

const IR::Node* MRenameSymbols::postorder(IR::Type_Struct* h) {
    auto name = getName();
    if (name != nullptr && *name != h->name) {
        auto annos = addNameAnnotation(h->name, h->annotations);
        h->name = *name;
        h->annotations = annos;
    }
    return h;
}

*/


}  // namespace P4
