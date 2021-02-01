// Example ParseAPI program; produces a graph (in DOT format) of the
// control flow graph of the provided binary. 
//
// Improvements by E. Robbins (er209 at kent dot ac dot uk)
//

#include <stdio.h>
#include <map>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <iostream>

#include "Instruction.h"
#include "CodeObject.h"
#include "CFG.h"

using namespace std;
using namespace Dyninst;
using namespace ParseAPI;
using namespace InstructionAPI;


 
void updateSet(set<RegisterAST> &valueSet ,set<RegisterAST::Ptr> &ptrSet ){
    for (auto ptr : ptrSet){
        valueSet.insert(ptr->getID());
    }
}

int main(int argc, char * argv[])
{
    map<Address, bool> seen;
    vector<Function *> funcs;
    SymtabCodeSource *sts;
    CodeObject *co;

    map<Address, unsigned > registerUsage;


    // Create a new binary code object from the filename argument
    sts = new SymtabCodeSource( argv[1] );
    co = new CodeObject( sts );

    // Parse the binary
    co->parse();

    // Print the control flow graph
    const CodeObject::funclist& all = co->funcs();
    auto fit = all.begin();
    for(int i = 0; fit != all.end(); ++fit, i++) { // i is index for clusters
        Function *f = *fit;
        //if(f->addr() != 0x605320)
        //    continue;


        set<RegisterAST> regsUsed;
        regsUsed.clear();


        stringstream edgeoutput;


        auto bit = f->blocks().begin();
        ParseAPI::Block::Insns insns;
        unsigned record  = 0 ; 
        for( ; bit != f->blocks().end(); ++bit) {

            Block *b = *bit;

            // Don't revisit blocks in shared code
            if(seen.find(b->start()) != seen.end())
                continue;

            seen[b->start()] = true;
            record  = regsUsed.size();

            insns.clear();
            b->getInsns(insns);

            auto iit = insns.begin();
            for ( ; iit != insns.end(); iit ++){
                Instruction &instr = iit->second;
                cout << "\t\t instr :: " << instr.format()  <<  endl << std::flush; 
                set<RegisterAST::Ptr> readSet;
                set<RegisterAST::Ptr> writeSet;
                readSet.clear();
                writeSet.clear();


                instr.getReadSet(readSet);
                instr.getWriteSet(writeSet);
                updateSet(regsUsed,readSet);
                updateSet(regsUsed,writeSet);

                std::cout << "Read Set : ";  
                for (auto regs : readSet){
                    cout << regs->format() << " ";
                }
                cout <<endl;
                std::cout << "Write Set : ";  

                for (auto regs : writeSet){
                    cout << regs->format() << " ";
                }
                cout <<endl;
                std:: cout << "Combined : " ; 
                for (auto regs : regsUsed){
                    cout << regs.format() <<  " " ;
                }
                cout << endl;
               /* 
                
               if ( (readSet.begin() != readSet.end()) &&  ( writeSet.end() != writeSet.end())){

                    RegisterAST::Ptr r1 = *readSet.begin();
                    RegisterAST::Ptr w1 = *writeSet.begin();
 
                    cout << "readSet.begin = " << r1->format() << "writeSet.begin = " << w1->format() << "equal < "<< (r1 < w1) << " > ? " << (w1 < r1 ) << endl;
                }
                */


            }
            cout << "In basic block starting at " << hex << b->start() << " a total of " << regsUsed.size() - record<< " registers are used " << endl;

        }
        cout << "In function " << f->name() << " starting at " 
            << hex << f->addr() << " a total of " << dec << regsUsed.size() << " registers are used" << std::endl;
        for (auto regs : regsUsed){
            cout << regs.format() << " ";
        }
        cout <<endl;
        registerUsage[f->addr()] = regsUsed.size();
    }
    ofstream ofile;
    ofile.open("usage.txt");
    auto uit = registerUsage.begin();
    for (; uit != registerUsage.end() ; uit ++){
        ofile << std::hex << uit->first << " "  << std::dec << uit->second << endl;
    }
    ofile.close();
}
