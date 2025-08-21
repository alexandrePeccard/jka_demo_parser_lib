#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>
#include <jka/demo.h>
#include <jka/message.h>
#include <jka/instruction.h>
#include <jka/state.h>

using json = nlohmann::json;
using namespace DemoJKA;

static json serializeInstruction(const Instruction* instr) {
    json j;
    if (!instr) return j;

    switch (instr->getType()) {
        case InstructionType::MapChange: {
            auto* mc = dynamic_cast<const MapChange*>(instr);
            j["type"] = "map_change";
            j["map"]  = mc->map();
            break;
        }
        case InstructionType::ServerCommand: {
            auto* sc = dynamic_cast<const ServerCommand*>(instr);
            j["type"]    = "server_command";
            j["command"] = sc->command();
            j["args"]    = sc->args();
            break;
        }
        case InstructionType::PlayerState: {
            auto* ps = dynamic_cast<const PlayerStateInstr*>(instr);
            j["type"] = "player_state";
            j["origin"] = { ps->origin()[0], ps->origin()[1], ps->origin()[2] };
            j["angles"] = { ps->angles()[0], ps->angles()[1], ps->angles()[2] };
            j["weapon"] = ps->weapon();
            j["health"] = ps->health();
            break;
        }
        case InstructionType::EntityState: {
            auto* es = dynamic_cast<const EntityStateInstr*>(instr);
            j["type"] = "entity_state";
            j["entity_num"] = es->entityNum();

            json fields;
            for (const auto& [fieldName, value] : es->netFields()) {
                fields[fieldName] = value;
            }
            j["fields"] = fields;
            break;
        }
        default:
            j["type"] = "unknown";
            break;
    }
    return j;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <input.dm_26> <output.json>" << std::endl;
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];

    Demo demo;
    if (!demo.open(inputFile.c_str(), true)) {
        std::cerr << "Failed to open demo file: " << inputFile << std::endl;
        return 1;
    }

    json root;
    root["filename"] = inputFile;
    root["messages_count"] = demo.getMessageCount();

    auto huffStream = std::make_shared<HuffmanStream>();
    json messages = json::array();

    for (int i = 0; i < demo.getMessageCount(); i++) {
        Message* msg = demo.getMessage(i);
        if (!msg) continue;

        json jmsg;
        jmsg["index"] = i;
        jmsg["size"]  = msg->size();

        json instructions = json::array();
        for (const auto& instr : msg->instructions()) {
            instructions.push_back(serializeInstruction(instr.get()));
        }

        jmsg["instructions"] = instructions;
        messages.push_back(jmsg);
    }

    root["messages"] = messages;

    std::ofstream out(outputFile);
    if (!out) {
        std::cerr << "Failed to write JSON to " << outputFile << std::endl;
        return 1;
    }

    out << root.dump(2);
    std::cout << "Exported demo JSON to " << outputFile << std::endl;
    return 0;
}
