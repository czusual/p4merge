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


class MergeManager {
    const IR::P4Program* meta;
    const IR::P4Program* program1;
    const IR::P4Program* program2;
    IR::P4Program result;
public:
    MergeManager() = default;
    MergeManager(const IR::P4Program* m,const IR::P4Program* p1,const IR::P4Program* p2)
    {
        meta=m;
        program1=p1;
        program2=p2;
    }
    IR::P4Program* run(const CompilerOptions& options);
    bool isSystemFile(cstring file);
    cstring ifSystemFile(const IR::Node* node);
};

}  // namespace P4

#endif /* _P4_TOP4_TORENAMEP4_H_ */
