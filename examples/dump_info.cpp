#include <iostream>
#include <memory>
#include <jka/jka_demo_parser.hpp>
#include <jka/demo.h>
#include <jka/message.h>
#include <jka/instruction.h>

using namespace DemoJKA;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <demo.dm_26>\n";
        return 1;
    }

    Demo demo;
    if (!demo.open(argv[1], true)) {
        std::cerr << "Failed to open demo file: " << argv[1] << "\n";
        return 1;
    }

    std::cout << "Loaded: " << argv[1] << "\n";
    std::cout << "Messages: " << demo.getMessageCount() << "\n";
    std::cout << "Maps: " << demo.getMapsCount() << "\n";

    // Parcours des messages
    for (int i = 0; i < demo.getMessageCount(); i++) {
        Message* msg = demo.getMessage(i);

        if (!msg || !msg->isLoad())
            continue;

        std::cout << "Message #" << i
                  << " (seq=" << msg->getSeqNumber()
                  << ", instr=" << msg->getInstructionsCount()
                  << ")\n";

        // Parcours des instructions
        for (int j = 0; j < msg->getInstructionsCount(); j++) {
            Instruction* instr = msg->getInstruction(j);

            if (auto serverCmd = instr->getServerCommand()) {
                std::cout << "  [ServerCmd] seq=" << serverCmd->getSequenceNumber()
                          << " cmd=\"" << serverCmd->getCommand() << "\"\n";
            }
            else if (auto snapshot = instr->getSnapshot()) {
                std::cout << "  [Snapshot] serverTime=" << snapshot->getServertime()
                          << " delta=" << snapshot->getDeltanum()
                          << " flags=" << snapshot->getSnapflags()
                          << "\n";
            }
            else if (auto gs = instr->getGamestate()) {
                std::cout << "  [Gamestate] client=" << gs->getMagicSeed()
                          << " configstrings=" << gs->getConfigstring(0) << " ...\n";
            }
            else if (auto mc = instr->getMapChange()) {
                std::cout << "  [MapChange]\n";
            }
        }
    }

    demo.close();
    return 0;
}
