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
#include "frontends/p4/mfrontend.h" //Zoe
#include "frontends/p4/frontend.h"
#include "frontends/p4/toP4/toP4.h"
//#include "midend.h"
#include "mergeManager.h" //Zoe
using namespace std;

class P4MergeOptions : public CompilerOptions {
 public:
    bool parseOnly = false;
    bool validateOnly = false;
    vector<string> postnames;
    string postName;
    vector<cstring> mfile;
    int mergenum=0;
    void setInputFileForMerge();
    void setInputFileForOne(int i)
    {
        postName=postnames[i];
        file = remainingOptions.at(i);
        top4.push_back("FrontEndLast");
    }
    void setInputFileForMeta(const char *fileName)
    {     
        file = fileName;
        //top4.push_back("FrontEndLast");
    }

    P4MergeOptions() {
        registerOption("--parse-only", nullptr,
                       [this](const char*) {
                           parseOnly = true;
                           return true; },
                       "only parse the P4 input, without any further processing");
        registerOption("--validate", nullptr,
                       [this](const char*) {
                           validateOnly = true;
                           return true;
                       },
                       "Validate the P4 input, running just the front-end");
     }
};
void P4MergeOptions::setInputFileForMerge() {
    if (remainingOptions.size() == 1) {
        ::error("Only one input file !");
        usage();
    } else if (remainingOptions.size() == 0) {
        ::error("No input files specified");
        usage();
    } else {
        for(unsigned int i=0;i<remainingOptions.size();i++)
        {
            mfile.push_back(remainingOptions.at(i));
            postnames.push_back("_v"+to_string(i+1));
            mergenum++;
        }
    }
}
 #if 0  
class P4MergeOption : public P4MergeOptions  {
 public:
    string rename;

    P4MergeOption() {
       
     }
    
     void setInputFileForOne(int i)
     {
        rename="_v"+to_string(i+1);
        file = remainingOptions.at(i);
     }
};
#endif

using P4MergeContext = P4CContextWithOptions<P4MergeOptions>;

static void log_dump(const IR::Node *node, const char *head) {
    if (node && LOGGING(1)) {
        if (head)
            std::cout << '+' << std::setw(strlen(head)+6) << std::setfill('-') << "+\n| "
                      << head << " |\n" << '+' << std::setw(strlen(head)+3) << "+" <<
                      std::endl << std::setfill(' ');
        if (LOGGING(2))
            dump(node);
        else
            std::cout << *node << std::endl; }
}

int main(int argc, char *const argv[]) {
    setup_gc_logging();
    setup_signals();

    AutoCompileContext autoP4TestContext(new P4MergeContext);

    /* 1.0 parse p4 file meta*/
    auto& options0 = P4MergeContext::get().options();
    options0.langVersion = CompilerOptions::FrontendVersion::P4_16;
    options0.compilerVersion = "0.0.5";

    if (options0.process(argc, argv) != nullptr)
        options0.setInputFileForMerge() ;
    if (::errorCount() > 0)
        return 1;
    
    options0.setInputFileForMeta("meta.p4");
   


    auto program0 = P4::parseP4File(options0);
    auto hook = options0.getDebugHook();

    if (program0 != nullptr && ::errorCount() == 0) {
        P4::P4COptionPragmaParser optionsPragmaParser;
        program0->apply(P4::ApplyOptionsPragmas(optionsPragmaParser));

        if (!options0.parseOnly) {
            try {
                P4::FrontEnd fe;
                fe.addDebugHook(hook);
                program0 = fe.run(options0, program0);
            } catch (const Util::P4CExceptionBase &bug) {
                std::cerr << bug.what() << std::endl;
                return 1;
            }
        }
        log_dump(program0, "Initial program0");
    }

    /* 1.1 parse p4 file 1*/
    auto& options1 = P4MergeContext::get().options();
    options1.langVersion = CompilerOptions::FrontendVersion::P4_16;
    options1.compilerVersion = "0.0.5";

    if (options1.process(argc, argv) != nullptr)
        options1.setInputFileForMerge() ;
    if (::errorCount() > 0)
        return 1;
    //P4MergeOptions moptions(options);
    options1.setInputFileForOne(0);
   


    auto program1 = P4::parseP4File(options1);
    //auto hook = options1.getDebugHook();

    if (program1 != nullptr && ::errorCount() == 0) {
        P4::P4COptionPragmaParser optionsPragmaParser;
        program1->apply(P4::ApplyOptionsPragmas(optionsPragmaParser));

        if (!options1.parseOnly) {
            try {
                P4::MergeFrontEnd fe;
                fe.addDebugHook(hook);
                program1 = fe.run(options1, program1,options1.postName.c_str());
            } catch (const Util::P4CExceptionBase &bug) {
                std::cerr << bug.what() << std::endl;
                return 1;
            }
        }
        log_dump(program1, "Initial program1");
    }

    /*1.2 parse p4 file 2*/
    auto& options2 = P4MergeContext::get().options();
    options2.langVersion = CompilerOptions::FrontendVersion::P4_16;
    options2.compilerVersion = "0.0.5";

    if (options2.process(argc, argv) != nullptr)
        options2.setInputFileForMerge() ;
    if (::errorCount() > 0)
        return 1;
    
    options2.setInputFileForOne(1);
   


    auto program2 = P4::parseP4File(options2);
    //auto hook = options2.getDebugHook();

    if (program2 != nullptr && ::errorCount() == 0) {
        P4::P4COptionPragmaParser optionsPragmaParser;
        program2->apply(P4::ApplyOptionsPragmas(optionsPragmaParser));

        if (!options2.parseOnly) {
            try {
                P4::MergeFrontEnd fe;
                fe.addDebugHook(hook);
                program2 = fe.run(options2, program2,options2.postName.c_str());
            } catch (const Util::P4CExceptionBase &bug) {
                std::cerr << bug.what() << std::endl;
                return 1;
            }
        }
        log_dump(program2, "Initial program2");
    }
 
    /*2.0 Merge*/
    
    for (auto a : program0->declarations) { 
            if (a->is<IR::P4Parser>()) {
                const IR::P4Parser* pmeta =dynamic_cast<const IR::P4Parser*>(a);
                //const IR::Type_Parser* type = nullptr;
                P4::MergeManager merging(program0,program1,program2,pmeta->getName(),pmeta->type);
                merging.run(options0);
                break;
            }
        }
    


 #if 0       


    P4::serializeP4RuntimeIfRequired(program, options);
#endif
    if (Log::verbose())
        std::cerr << "Done." << std::endl;
    return ::errorCount() > 0;
}
