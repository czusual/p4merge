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

#include <fstream>
#include <iostream>
#include <vector>
#include <string.h>
#include <string>

#include "control-plane/p4RuntimeSerializer.h"
#include "ir/ir.h"
#include "ir/json_loader.h"
#include "lib/log.h"
#include "lib/error.h"
#include "lib/exceptions.h"
#include "lib/gc.h"
#include "lib/crash.h"
#include "lib/nullstream.h"
#include "lib/cstring.h"
#include "frontends/common/applyOptionsPragmas.h"
#include "frontends/common/parseInput.h"
#include "frontends/p4/evaluator/evaluator.h"
#include "frontends/p4/toP4/toP4.h"
#include "ir/indexed_vector.h"
#include "mergeManager.h"
using namespace std;

namespace P4 {

bool MergeManager::isSystemFile(cstring file) {
    if (file.startsWith(p4includePath)) return true;
    return false;
}

cstring MergeManager::ifSystemFile(const IR::Node* node) {
    if (!node->srcInfo.isValid()) return nullptr;
    auto sourceFile = node->srcInfo.getSourceFile();
    if (isSystemFile(sourceFile))
        return sourceFile;
    return nullptr;
}

IR::P4Program* MergeManager::run(const CompilerOptions& options){   //!!now this is rough merge!!
    //bool first=true;
    for (auto a : program1->declarations) {
        // Check where this declaration originates
        cstring sourceFile = ifSystemFile(a);
        if (!a->is<IR::Type_Error>() &&  // errors can come from multiple files
            sourceFile != nullptr) {
            
            //#include<a>     
            
            continue;
        }
        result.declarations.push_back(a);
        
    }
    for (auto a : program2->declarations) {
        // Check where this declaration originates
        cstring sourceFile = ifSystemFile(a);
        if (!a->is<IR::Type_Error>() &&  // errors can come from multiple files
            sourceFile != nullptr) {
            
            //#include<a>     
            
            continue;
        }
        result.declarations.push_back(a);
        
    }
    cstring fileName="merge_result.p4";
    auto stream = openFile(fileName, true);
    if (stream != nullptr) {
        if (Log::verbose())
            std::cerr << "Writing program to " << fileName << std::endl;
        P4::ToP4 toP4(stream,false,options.file); //now postFix hardcode,easy to improve by using global var
        result.apply(toP4);
    }
    return &result;
}

} // namespace P4
