#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <jka/defs.h>
#include <jka/state.hpp>
#include <jka/messagebuffer.hpp>

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <ostream>

namespace jka {

    class State;
    class EntityState;
    class PlayerState;
    class VehicleState;

    // Types d'instructions DM_26
    enum InstrTypes {
        INSTR_BASE = 0,
        INSTR_MAPCHANGE,
        INSTR_SERVERCOMMAND,
        INSTR_SNAPSHOT,   // Définition réelle déplacée dans snapshot.hpp
        INSTR_GAMESTATE,
    };

    // Pré-déclarations pour conversions typées
    class Gamestate;
    class MapChange;
    class Snapshot;      // <— défini dans snapshot.hpp
    class ServerCommand;

    /**
     * Instruction de base
     */
    class Instruction {
    protected:
        int type;

        // Map des entités utilisée par plusieurs instructions
    protected:
        using entitymap    = std::map<int, EntityState>;
        using entitymap_it = entitymap::iterator;
        using entitymap_cit= entitymap::const_iterator;

    public:
        explicit Instruction(int t = INSTR_BASE) : type(t) {}
        virtual ~Instruction() = default;

        // I/O virtuels (implémentations dans les .cpp correspondants)
        virtual void Save() const {}
        virtual void Load() {}
        virtual void report(std::ostream& os) const;

        // Accès
        int getType() const noexcept { return type; }

        // Conversions dynamiques (compat héritée)
        MapChange*     getMapChange()     { return dynamic_cast<MapChange*>(this); }
        Gamestate*     getGamestate()     { return dynamic_cast<Gamestate*>(this); }
        Snapshot*      getSnapshot()      { return dynamic_cast<Snapshot*>(this); }
        ServerCommand* getServerCommand() { return dynamic_cast<ServerCommand*>(this); }
    };

    /**
     * Changement de carte
     */
    class MapChange : public Instruction {
    private:
        std::string mapChange;

    public:
        MapChange() : Instruction(INSTR_MAPCHANGE) {}
        explicit MapChange(const std::string& map)
            : Instruction(INSTR_MAPCHANGE), mapChange(map) {}

        // I/O
        void Save() const override;
        void report(std::ostream& os) const override;

        // Accès modernes
        const std::string& getMapChange() const noexcept { return mapChange; }
        void setMapChange(const std::string& map) { mapChange = map; }
    };

    /**
     * Commande serveur (configstrings, etc.)
     */
    class ServerCommand : public Instruction {
    private:
        int         sequenceNumber {0};
        std::string command;

    public:
        ServerCommand() : Instruction(INSTR_SERVERCOMMAND) {}

        // I/O
        void Save() const override;
        void Load() override;
        void report(std::ostream& os) const override;

        // Accès
        int getSequenceNumber() const noexcept { return sequenceNumber; }
        const std::string& getCommand() const noexcept { return command; }

        void setSequenceNumber(int seq) noexcept { sequenceNumber = seq; }
        void setCommand(const std::string& cmd) { command = cmd; }
    };

    /**
     * GameState initial (configstrings + entités de base + "magic")
     */
    class Gamestate : public Instruction {
        struct MagicData {
            int byte1{0}, byte2{0};
            int int1{0},  int2{0};
        };

    protected:
        using stringmap    = std::map<int, std::string>;
        using stringmap_it = stringmap::iterator;
        using stringmap_cit= stringmap::const_iterator;

        int commandSequence{0};
        int clientNumber{0};
        int checksumFeed{0};

        std::string            magicStuff;
        int                    magicSeed{0};
        std::vector<MagicData> magicData;

        entitymap baseEntities;
        stringmap configStrings;

    public:
        Gamestate() : Instruction(INSTR_GAMESTATE) {}

        // I/O
        void Save() const override;
        void Load() override;
        void report(std::ostream& os) const override;

        // Accès
        std::string getConfigstring(int id) const;
        const std::string& getMagicStuff() const noexcept { return magicStuff; }
        int  getMagicSeed()  const noexcept { return magicSeed; }
        int  getMagicDataCount() const noexcept { return static_cast<int>(magicData.size()); }
        void getMagicData(unsigned id, int* byte1, int* byte2, int* int1, int* int2) const;

        void setConfigstring(int id, const std::string& s);
        void setMagicStuff(const std::string& s) { magicStuff = s; }
        void setMagicSeed(int seed) noexcept { magicSeed = seed; }
        void setMagicData(unsigned id, int byte1, int byte2, int int1, int int2);

        void removeConfigstring(int id);

        // Mise à jour depuis un ServerCommand (configstrings incrémentaux, etc.)
        void update(const ServerCommand* servercommand);

        // Getters modernes
        int getCommandSequence() const noexcept { return commandSequence; }
        int getClientNumber()    const noexcept { return clientNumber; }
        int getChecksumFeed()    const noexcept { return checksumFeed; }

        const entitymap& getBaseEntities() const noexcept { return baseEntities; }
        const stringmap& getConfigStrings() const noexcept { return configStrings; }
    };
}

// NOTE IMPORTANTE : la classe Snapshot est déplacée dans snapshot.hpp.
// Inclure <jka/snapshot.hpp> dans les TU qui manipulent Snapshot.

#endif // INSTRUCTION_H
