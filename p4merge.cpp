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
#include "frontends/p4/mfrontend.h"
#include "frontends/p4/toP4/toP4.h"
#include "midend.h"
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

    /* 1.0 parse p4 file 1*/
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
    auto hook = options1.getDebugHook();

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

    /*1.1 parse p4 file 2*/
    auto& options2 = P4MergeContext::get().options();
    options2.langVersion = CompilerOptions::FrontendVersion::P4_16;
    options2.compilerVersion = "0.0.5";

    if (options2.process(argc, argv) != nullptr)
        options2.setInputFileForMerge() ;
    if (::errorCount() > 0)
        return 1;
    //P4MergeOptions moptions(options);
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
 #if 0       
        if (program != nullptr && ::errorCount() == 0) {
            if (!options.parseOnly && !options.validateOnly) {
                P4Test::MidEnd midEnd(options);
                midEnd.addDebugHook(hook);
#if 0 
                /* doing this breaks the output until we get dump/undump of srcInfo */
                if (options.debugJson) {
                    std::stringstream tmp;
                    JSONGenerator gen(tmp);
                    gen << program;
                    JSONLoader loader(tmp);
                    loader >> program;
                } //liyong hong de yizhong gaoji zhushi jiqiao
#endif
                const IR::ToplevelBlock *top = nullptr;
                try {
                    top = midEnd.process(program);
                } catch (const Util::P4CExceptionBase &bug) {
                    std::cerr << bug.what() << std::endl;
                    return 1;
                }
                log_dump(program, "After midend");
                log_dump(top, "Top level block");
            }
            if (options.dumpJsonFile)
                JSONGenerator(*openFile(options.dumpJsonFile, true), true) << program << std::endl;
            if (options.debugJson) {
                std::stringstream ss1, ss2;
                JSONGenerator gen1(ss1), gen2(ss2);
                gen1 << program;

                const IR::Node* node = nullptr;
                JSONLoader loader(ss1);
                loader >> node;

                gen2 << node;
                if (ss1.str() != ss2.str()) {
                    error("json mismatch");
                    std::ofstream t1("t1.json"), t2("t2.json");
                    t1 << ss1.str() << std::flush;
                    t2 << ss2.str() << std::flush;
                    auto rv = system("json_diff t1.json t2.json");
                    if (rv != 0) ::warning("json_diff failed with code %1%", rv);
                }
            }
        }
    }

    P4::serializeP4RuntimeIfRequired(program, options);
#endif
    if (Log::verbose())
        std::cerr << "Done." << std::endl;
    return ::errorCount() > 0;
}
