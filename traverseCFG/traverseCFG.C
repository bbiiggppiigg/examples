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
#include <iomanip>
#include "CodeObject.h"
#include "InstructionDecoder.h"
#include "CFG.h"

using namespace std;
using namespace Dyninst;
using namespace ParseAPI;
using namespace InstructionAPI;

int main(int argc, char * argv[])
{
   map<Address, bool> seen;
   vector<Function *> funcs;
   SymtabCodeSource *sts;
   CodeObject *co;
   
   // Create a new binary code object from the filename argument
   sts = new SymtabCodeSource( argv[1] );
   co = new CodeObject( sts );
   
   // Parse the binary
   co->parse();
   cout << "digraph G {" << endl;
   
   // Print the control flow graph
   const CodeObject::funclist& all = co->funcs();
     auto fit = all.begin();
   for(int i = 0; fit != all.end(); ++fit, i++) { // i is index for clusters
      Function *f = *fit;
      InstructionDecoder decoder(f->isrc()->getPtrToInstruction(f->addr()),
            InstructionDecoder::maxInstructionLength,
            f->region()->getArch());

   
      // Make a cluster for nodes of this function
      cout << "\t subgraph cluster_" << i 
           << " { \n\t\t label=\""
           << f->name()
           << "\"; \n\t\t color=blue;" << endl;
      
      cout << "\t\t\"" << hex << f->addr() << dec
           << "\" [shape=box";
      if (f->retstatus() == NORETURN)
         cout << ",color=red";
      cout << "]" << endl;
      
      // Label functions by name
      cout << "\t\t\"" << hex << f->addr() << dec
           << "\" [label = \""
           << f->name() << "\\n" << hex << f->addr() << dec
           << "\"];" << endl;

      stringstream edgeoutput;
      
      auto bit = f->blocks().begin();
      for( ; bit != f->blocks().end(); ++bit) {
         Block *b = *bit;
         // Don't revisit blocks in shared code
         if(seen.find(b->start()) != seen.end())
            continue;
         
         seen[b->start()] = true;
         
         cout << "\t\t\"" << hex << b->start() << dec << 
            "\";" << endl;
         
         auto edges = b->targets().begin();
         for( ; edges != b->targets().end(); ++edges) {
            if(!*edges) continue;
            std::string s = "";
            if((*edges)->type() == CALL)
               s = " [color=blue]";
            else if((*edges)->type() == RET)
               s = " [color=green]";
            
            Address branch_addr = (*edges)->src()->lastInsnAddr();

            Instruction instr = 
                decoder.decode(
                    (unsigned char *) f->isrc()->getPtrToInstruction(branch_addr)
                    );

           // Store the edges somewhere to be printed outside of the cluster
            edgeoutput  << "\t\"" 
                       << hex << branch_addr << ":" << left <<setw(40) <<instr.format()
                       << "\" -> \""
                       << (*edges)->trg()->start()
                       << "\"" << s << endl;
         }
      }
      // End cluster
      cout << "\t}" << endl;

      // Print edges
      cout << edgeoutput.str() << endl;
   }
   cout << "}" << endl;
}
