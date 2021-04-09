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

const IR::P4Control * MergeManager::getControl(const IR::P4Program* p,int order){
    int i=0;
    for (auto a : p->declarations) {
               
        if (a->is<IR::P4Control>()) {
            
            if(i==order){
                return dynamic_cast<const IR::P4Control*>(a);    
            }
            i++;
        }               
    }
    return nullptr;
}
const IR::P4Parser * MergeManager::getParser(const IR::P4Program* p){
    for (auto a : p->declarations) { 
        if (a->is<IR::P4Parser>()) {
            return dynamic_cast<const IR::P4Parser*>(a); 
        }
    }
    return nullptr;
}


Visitor::profile_t MergeManager::init_apply(const IR::Node* node) {
    
    return Inspector::init_apply(node);
}

void MergeManager::end_apply(const IR::Node*) {
    
}

bool MergeManager::preorder(const IR::Type_Struct* t) {
    cstring name=t->getName();
    if(headerName.find(name) == headerName.end()){
                
        headerName.emplace(name);
        result.declarations.push_back(t);
    }
    return false;
}
bool MergeManager::preorder(const IR::Type_Header* t) {
    cstring name=t->getName();
    if(headerName.find(name) == headerName.end()){
                
        headerName.emplace(name);
        result.declarations.push_back(t);
    }
    return false;
}

/*
bool MergeManager::preorder(IR::P4Parser* c){
    if(isP1parser){
        p1parser=c;
    }
    else{
        if(isP2parser){
            p2parser=c;
        }
        else
        {
            for (auto p1 : p1parser->states) 
            {
                c->states.push_back(p1);
            }
            for (auto p2 : p2parser->states) 
            {
                c->states.push_back(p2);
            }
        } 
    }
    return false;
}
*/

IR::P4Program* MergeManager::run(const CompilerOptions& options,bool secondRound){   //!!now this is rough merge!!
    //bool first=true;
    //IR::Vector<IR::Node>::iterator itest=result.declarations.begin();
    std::set<cstring> includesEmitted;
    

    /* 1 include , typedef*/
    for (auto a : program1->declarations) {
        
        cstring sourceFile = ifSystemFile(a);
        std::cout<<"1"<<a->node_type_name()<<std::endl;
        if (!a->is<IR::Type_Error>() && sourceFile != nullptr) {
            
            if (includesEmitted.find(sourceFile) == includesEmitted.end()) {
                result.declarations.push_back(a);
                includesEmitted.emplace(sourceFile);
            }
            
            continue;
        }         
    }
    for (auto a : program2->declarations) {
        
        cstring sourceFile = ifSystemFile(a);
        std::cout<<"2"<<a->node_type_name()<<std::endl;
        if (!a->is<IR::Type_Error>() &&  
            sourceFile != nullptr) {
            
            if (includesEmitted.find(sourceFile) == includesEmitted.end()) {
                result.declarations.push_back(a);
                includesEmitted.emplace(sourceFile);
            }
            
            continue;
        }
                 
    }
    for (auto a : program2->declarations) {
        
        
        if (a->is<IR::Type_Typedef>()) {
            
            result.declarations.push_back(a);           
            continue;
        }
                 
    }
    /* 2 header and struct*/ /*For "header" and "typedef" now is hardcode, because I don't do ''remame'' and vars who have the same name become one*/
    for (auto a : program2->declarations) {
        
        if (a->is<IR::Type_Struct>()||a->is<IR::Type_Header>()) {
            
            init_apply(a);
            visit(a);
            
            continue;
        }         
    }
    for (auto a : program1->declarations) {
        
        if (a->is<IR::Type_Struct>()||a->is<IR::Type_Header>()) {
            
            init_apply(a);
            visit(a);  
            
            continue;
        }         
    }
    /* 3 parser*/
    const IR::P4Parser* metaParser=getParser(meta);
    const IR::P4Parser* p1Parser=getParser(program1);
    const IR::P4Parser* p2Parser=getParser(program2);
    IR::P4Parser mergeParser(metaParser->getName(),metaParser->type);
    int i=0;
    for(auto p0:metaParser->states)
    {
        mergeParser.states.push_back(p0);
        break;

    }
    int startstate=1;
    if(secondRound){
         startstate=1;
    }
    i=0;  
    for(auto p1:p1Parser->states)
    {
        ++i;
        
        if(i>startstate){
            mergeParser.states.push_back(p1);
        }
        
    }
    i=0;
    for(auto p2:p2Parser->states)
    {
         ++i;
        
        if(i>startstate){
            mergeParser.states.push_back(p2);
        }
    }   
    result.declarations.push_back(&mergeParser);

    
    /* 4.0 control checksum*/
    result.declarations.push_back(getControl(program1,0));
    /* 4.1 control ingress*/
    const IR::P4Control* metaControl=getControl(meta,1);
    const IR::P4Control* p1Control=getControl(program1,1);
    const IR::P4Control* p2Control=getControl(program2,1);
    IR::BlockStatement mergeControlBody;
    IR::BlockStatement mergeControlBody1;
    IR::BlockStatement mergeControlBody2;
    for(auto p1:p1Control->body->components)
    {
        mergeControlBody1.components.push_back(p1);
    }
    for(auto p2:p2Control->body->components)
    {
        mergeControlBody2.components.push_back(p2);
    }
    IR::IfStatement mergeControlIf1(dynamic_cast<const IR::IfStatement*>(metaControl->body->components[0])->condition,
        &mergeControlBody1,dynamic_cast<const IR::IfStatement*>(metaControl->body->components[0])->ifFalse);
    IR::IfStatement mergeControlIf2(dynamic_cast<const IR::IfStatement*>(metaControl->body->components[1])->condition,
        &mergeControlBody2,dynamic_cast<const IR::IfStatement*>(metaControl->body->components[1])->ifFalse);
    mergeControlBody.components.push_back(&mergeControlIf1);
    mergeControlBody.components.push_back(&mergeControlIf2);
    
    IR::P4Control mergeControl(metaControl->getName(),metaControl->type,metaControl->getConstructorParameters(),&mergeControlBody);
    for(auto p1:p1Control->controlLocals)
    {
        mergeControl.controlLocals.push_back(p1);
    }
    for(auto p2:p2Control->controlLocals)
    {
        mergeControl.controlLocals.push_back(p2);
    }
   
    result.declarations.push_back(&mergeControl);

    /* 4.2 control egress*/
    result.declarations.push_back(getControl(program2,2));
    /* 4.3 control outchecksum*/
    result.declarations.push_back(getControl(program2,3));
    /* 4.4 control deparser*/
    result.declarations.push_back(getControl(program2,4));
    /* 5 instance*/
    for (auto a : program2->declarations) {
        
        
        if (a->is<IR::Declaration_Instance>()) {
            
            result.declarations.push_back(a);           
            break;
        }
                 
    }
    /* out:toP4*/
    for (auto a : result.declarations) {
        std::cout<<a->node_type_name()<<std::endl;              
    }
    cstring fileName="merge_result.p4";

    if(secondRound){                                  ////////////////////////////////
        fileName="merge_result_2.p4";
    }
    
    auto stream = openFile(fileName, true);
    if (stream != nullptr) {
        if (Log::verbose())
            std::cerr << "Writing program to " << fileName << std::endl;
        P4::ToP4 toP4(stream,false,options.file); 
        result.apply(toP4);
    }
    return &result;
}

} // namespace P4
