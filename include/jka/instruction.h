/*#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <jka/defs.h>
#include <jka/state.h>
#include <jka/messagebuffer.h>

DEMO_NAMESPACE_START

class State;
class EntityState;

enum InstrTypes {
    INSTR_BASE = 0,
    INSTR_MAPCHANGE,
    INSTR_SERVERCOMMAND,
    INSTR_SNAPSHOT,
    INSTR_GAMESTATE,
};

class Gamestate;
class MapChange;
class Snapshot;
class ServerCommand;

class Instruction {
protected:
    int type;

    typedef std::map<int, EntityState> entitymap;
    typedef std::map<int, EntityState>::iterator entitymap_it;
    typedef std::map<int, EntityState>::const_iterator entitymap_cit;

public:
    explicit Instruction(int type = INSTR_BASE) : type(type) {}
    virtual ~Instruction() = default;

    //I/O methods
    virtual void Save() const;
    virtual void Load();
    virtual void report(std::ostream& os) const;

    //get methods
    int getType() const noexcept { return type; }

    //convert methods
    //BAD IDEA, need to modify base class for every new instruction type
    //TODO: remove and make clients use dynamic_cast instead
    MapChange* getMapChange();
    Gamestate* getGamestate();
    Snapshot* getSnapshot();
    ServerCommand* getServerCommand();
};

class MapChange : public Instruction {
private:
    std::string mapChange;

public:
    MapChange() : Instruction(INSTR_MAPCHANGE) {}
    explicit MapChange(const std::string& map) : Instruction(INSTR_MAPCHANGE), mapChange(map) {}

    //I/O methods
    void Save() const override;
    void report(std::ostream& os) const override;

    // Modern getters/setters
    const std::string& getMapChange() const noexcept { return mapChange; }
    void setMapChange(const std::string& map) { mapChange = map; }
};

class ServerCommand : public Instruction {
private:
    int         sequenceNumber;
    std::string command;

public:
    ServerCommand() : Instruction(INSTR_SERVERCOMMAND), sequenceNumber(0) {}

    //I/O methods
    void Save() const override;
    void Load() override;
    void report(std::ostream& os) const override;

    int getSequenceNumber() const noexcept { return sequenceNumber; }
    const std::string& getCommand() const noexcept { return command; }
    
    void setSequenceNumber(int seq) noexcept { sequenceNumber = seq; }
    void setCommand(const std::string& cmd) { command = cmd; }
};

class PlayerState;
class VehicleState;

class Snapshot : public Instruction {
protected:
    int serverTime;
    int deltaNum;
    int flags;
    std::vector<byte> areaMask;

    PlayerState* playerState;
    PlayerState* vehicleState;
    entitymap entities;

public:
    Snapshot() : Instruction(INSTR_SNAPSHOT), serverTime(0), deltaNum(0), flags(0),
                 playerState(nullptr), vehicleState(nullptr) {}
    ~Snapshot() override;

    // Disable copy constructor and assignment (modern C++)
    Snapshot(const Snapshot&) = delete;
    Snapshot& operator=(const Snapshot&) = delete;

    //clone
    std::unique_ptr<Snapshot> clone() const;

    //I/O methods
    void Save() const override;
    void Load() override;
    void report(std::ostream& os) const override;

    //get methods
    int getAreamaskLen() const noexcept { return static_cast<int>(areaMask.size()); }
    int getAreamask(int id) const { return static_cast<int>(areaMask.at(id)); }
    int getDeltanum() const noexcept { return deltaNum; }
    int getServertime() const noexcept { return serverTime; }
    int getSnapflags() const noexcept { return flags; }
    PlayerState* getPlayerstate() noexcept { return playerState; }
    PlayerState* getVehiclestate() noexcept { return vehicleState; }
    const PlayerState* getPlayerstate() const noexcept { return playerState; }
    const PlayerState* getVehiclestate() const noexcept { return vehicleState; }

    //set methods
    void setAreamask(int id, int value) { areaMask.at(id) = static_cast<byte>(value); }
    void setAreamaskLen(int value) { areaMask.resize(static_cast<size_t>(value)); }
    void setSnapflags(int value) noexcept { flags = value; }
    void setDeltanum(int value) noexcept { deltaNum = value; }
    void setServertime(int value) noexcept { serverTime = value; }

    //if this is uncompressed snapshot, use this to remove all 0 values
    //TODO: maybe private methods?
    void makeInit();
    void removeNotChanged();

    //cutting commands
    //BAD, move this to Cutter class etc
    void applyOn(Snapshot* snap);
    void delta(Snapshot* snap);

    //hack like, only for purpose of optimizer
    //BAD
    entitymap& getEntities() noexcept { return entities; }
    const entitymap& getEntities() const noexcept { return entities; }
};

class Gamestate : public Instruction {
    struct MagicData {
        MagicData() : byte1(0), byte2(0), int1(0), int2(0) {}

        int byte1, byte2;
        int int1, int2;
    };

protected:
    typedef std::map<int, std::string> stringmap;
    typedef std::map<int, std::string>::iterator stringmap_it;
    typedef std::map<int, std::string>::const_iterator stringmap_cit;

    int commandSequence;
    int clientNumber;
    int checksumFeed;

    std::string             magicStuff;
    int                     magicSeed;
    std::vector<MagicData>  magicData;

    entitymap baseEntities;
    stringmap configStrings;

public:
    Gamestate() : Instruction(INSTR_GAMESTATE),
        commandSequence(0), clientNumber(0),
        checksumFeed(0), magicSeed(0) {}

    //I/O methods
    void Save() const override;
    void Load() override;
    void report(std::ostream& os) const override;

    //get methods
    std::string getConfigstring(int id) const;
    const std::string& getMagicStuff() const noexcept { return magicStuff; }
    int getMagicSeed() const noexcept { return magicSeed; }
    int getMagicDataCount() const noexcept { return static_cast<int>(magicData.size()); }
    void getMagicData(unsigned id, int* byte1, int* byte2, int* int1, int* int2) const;

    //set methods
    void setConfigstring(int id, const std::string& s);
    void setMagicStuff(const std::string& s) { magicStuff = s; }
    void setMagicSeed(int seed) noexcept { magicSeed = seed; }
    void setMagicData(unsigned id, int byte1, int byte2, int int1, int int2);

    void removeConfigstring(int id);

    //load new things from server message
    void update(const ServerCommand* servercommand);

    // Modern getters
    int getCommandSequence() const noexcept { return commandSequence; }
    int getClientNumber() const noexcept { return clientNumber; }
    int getChecksumFeed() const noexcept { return checksumFeed; }

    // Access to entities and config strings
    const entitymap& getBaseEntities() const noexcept { return baseEntities; }
    const stringmap& getConfigStrings() const noexcept { return configStrings; }

};

DEMO_NAMESPACE_END

#endif*/

#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <jka/defs.h>
#include <jka/state.h>
#include <jka/messagebuffer.h>

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <ostream>

DEMO_NAMESPACE_START

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

DEMO_NAMESPACE_END

// NOTE IMPORTANTE : la classe Snapshot est déplacée dans snapshot.hpp.
// Inclure <jka/snapshot.hpp> dans les TU qui manipulent Snapshot.

#endif // INSTRUCTION_H
