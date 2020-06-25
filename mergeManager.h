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

#ifndef _EXTENSIONS_P4MERGE_MERGEMANAGER_H_
#define _EXTENSIONS_P4MERGE_MERGEMANAGER_H_

#include "ir/ir.h"
#include "ir/visitor.h"
#include "lib/sourceCodeBuilder.h"

namespace P4 {


class MergeManager : public Inspector{
    const IR::P4Program* meta;
    const IR::P4Program* program1;
    const IR::P4Program* program2;
    IR::P4Program result;
    //std::map<const IR::Node*, cstring> headerName;
    std::set<cstring> headerName;
    //IR::P4Parser* p1parser=nullptr;
    //IR::P4Parser* p2parser=nullptr;
    IR::P4Parser mergeParser;
    bool isP1parser=false;
    bool isP2parser=false;

public:
    MergeManager()=delete;

    MergeManager(const IR::P4Program* m,const IR::P4Program* p1,const IR::P4Program* p2,IR::ID name,const IR::Type_Parser* type):
    mergeParser(name,type)
    {
        meta=m;
        program1=p1;
        program2=p2;
        
        
        visitDagOnce = false; 
        setName("MergeManager");
        //visited = new visited_t();// because I don't goto  visitor.cpp:142
    }
    using Inspector::preorder;
    IR::P4Program* run(const CompilerOptions& options);
    bool isSystemFile(cstring file);
    cstring ifSystemFile(const IR::Node* node);

    Visitor::profile_t init_apply(const IR::Node* node) override;
    void end_apply(const IR::Node* node) override;

    bool preorder(const IR::Type_Struct* t) override;
    bool preorder(const IR::Type_Header* t) override;
    //bool preorder(const IR::P4Parser* c) override;
};

}  // namespace P4

#endif /* _EXTENSIONS_P4MERGE_MERGEMANAGER_H_ */
