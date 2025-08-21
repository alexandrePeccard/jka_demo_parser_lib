#ifndef STATE_H
#define STATE_H

#include <jka/defs.h>
#include <jka/netfields.h>
#include <jka/instruction.h>
#include <jka/messagebuffer.h>

DEMO_NAMESPACE_START

union Attribute {
    float fVal; //32bit float
    int   iVal; //32bit int
    
    // Modern constructors
    Attribute() : iVal(0) {}
    explicit Attribute(float f) : fVal(f) {}
    explicit Attribute(int i) : iVal(i) {}
};

// Legacy typedef for backward compatibility
using Atribute = Attribute;

class MessageBuffer;

enum DataType : int {
    INTEGER = 0,
    FLOAT = 1
};

enum class StateType : int {
    BASE = 0,
    DELTA_ENTITY,
    PLAYER_STATE,
    PILOT_STATE,
    VEHICLE_STATE
};

// Legacy enum for backward compatibility
enum STATE_TYPE {
    STATE_BASE = 0,
    STATE_DELTAENTITY,
    STATE_PLAYERSTATE,
    STATE_PILOTSTATE,
    STATE_VEHICLESTATE
};

class PlayerState;

class State {
    friend class PlayerState;

protected:
    using AttributeMap = std::map<int, Attribute>;
    using AttributeMapIt = AttributeMap::iterator;
    using AttributeMapCit = AttributeMap::const_iterator;
    
    // Legacy typedefs for backward compatibility
    using IntAtributeMap = AttributeMap;
    using IntAtributeMapIt = AttributeMapIt;
    using IntAtributeMapCit = AttributeMapCit;

protected:
    int type;
    AttributeMap attributes;

public:
    explicit State(int type) : type(type) {}
    virtual ~State() = default;

    // Disable copy constructor and assignment for base class
    State(const State&) = delete;
    State& operator=(const State&) = delete;

    //clone
    virtual std::unique_ptr<State> clone() const = 0;

    //I/O methods
    virtual void report(std::ostream& os) const = 0;
    virtual void save() const = 0;
    virtual void load() = 0;

    //get methods
    int getType() const noexcept { return type; }

    float getAttributeFloat(int id) const;
    int getAttributeInt(int id) const;
    
    // Legacy method names for backward compatibility
    float getAtributeFloat(int id) const { return getAttributeFloat(id); }
    int getAtributeInt(int id) const { return getAttributeInt(id); }

    virtual bool isAttributeFloat(int id) const = 0;
    virtual bool isAttributeInteger(int id) const = 0;
    
    // Legacy method names for backward compatibility
    virtual bool isAtributeFloat(int id) const { return isAttributeFloat(id); }
    virtual bool isAtributeInteger(int id) const { return isAttributeInteger(id); }

    int getAttributesCount() const noexcept;
    int getAtributesCount() const noexcept { return getAttributesCount(); } // Legacy

    //overloaded methods for setting attributes
    void setAttribute(int id, float value);
    void setAttribute(int id, int value);
    
    // Legacy method names for backward compatibility
    void setAtribute(int id, float value) { setAttribute(id, value); }
    void setAtribute(int id, int value) { setAttribute(id, value); }

    //other const methods
    bool isAttributeSet(int id) const noexcept;
    bool isAtributeSet(int id) const noexcept { return isAttributeSet(id); } // Legacy

    //means that user changed something in objects
    virtual bool isChanged() const = 0;

    //means that nothing is happening in this message
    virtual bool noChanged() const = 0;

    //removes all 0 in states
    virtual void removeNull() {}

    virtual void clear();

    PlayerState* getPlayerstate();
    const PlayerState* getPlayerstate() const;

protected:
    // Protected access to attributes for derived classes
    AttributeMap& getAttributes() noexcept { return attributes; }
    const AttributeMap& getAttributes() const noexcept { return attributes; }
};

class EntityState : public State {
protected:
    bool toRemove;
    bool previousToRemove;

public:
    EntityState() : State(STATE_DELTAENTITY), toRemove(false), previousToRemove(false) {}
    ~EntityState() override = default;

    std::unique_ptr<State> clone() const override;

    //I/O methods
    void report(std::ostream& os) const override;
    void save() const override;
    void load() override;

    //get methods
    bool isAttributeFloat(int id) const override;
    bool isAttributeInteger(int id) const override;

    //set methods
    void setRemove(bool remove) noexcept { toRemove = remove; }

    //other const methods
    bool isRemoved() const noexcept { return toRemove; }

    //means that user changed something in the object
    bool isChanged() const override;

    //means that nothing is happening in this message
    bool noChanged() const override;

    void delta(const EntityState* state);
    void applyOn(const EntityState* state);
    void removeNull() override;

    void clear() override;

    void setPreviousToRemove(bool set) noexcept { previousToRemove = set; }
    bool getPreviousToRemove() const noexcept { return previousToRemove; }
    
    // Legacy method names for backward compatibility
    void setprevtoremove(bool set) noexcept { setPreviousToRemove(set); }
    bool getprevtoremove() const noexcept { return getPreviousToRemove(); }
};

class PlayerState : public State {
protected:
    using StatsArray = std::map<int, int>;
    using StatsArrayIt = StatsArray::iterator;
    using StatsArrayCit = StatsArray::const_iterator;
    
    // Legacy typedefs for backward compatibility
    using statsarray = StatsArray;
    using statsarray_it = StatsArrayIt;
    using statsarray_cit = StatsArrayCit;

protected:
    //additional attributes
    StatsArray stats;
    StatsArray persistant;
    StatsArray ammo;
    StatsArray powerups;

    explicit PlayerState(int id) : State(id) {}

public:
    PlayerState() : State(STATE_PLAYERSTATE) {}

    virtual std::unique_ptr<PlayerState> clone() const;
    std::unique_ptr<State> clone() const override { 
        return std::unique_ptr<State>(clone().release()); 
    }

    ~PlayerState() override = default;

    void report(std::ostream& os) const override;
    void save() const override;
    void load() override;
    bool isChanged() const override;
    bool noChanged() const override;

    virtual bool hasVehicleSet() const;

    bool isAttributeFloat(int id) const override;
    bool isAttributeInteger(int id) const override;

    void removeNull() override;
    void delta(const PlayerState* state, bool isUncompressed);
    void applyOn(PlayerState* state);

    void clear() override;

    // Modern getters for stats arrays
    const StatsArray& getStats() const noexcept { return stats; }
    const StatsArray& getPersistant() const noexcept { return persistant; }
    const StatsArray& getAmmo() const noexcept { return ammo; }
    const StatsArray& getPowerups() const noexcept { return powerups; }

    // Setters for individual stats
    void setStat(int id, int value) { stats[id] = value; }
    void setPersistant(int id, int value) { persistant[id] = value; }
    void setAmmo(int id, int value) { ammo[id] = value; }
    void setPowerup(int id, int value) { powerups[id] = value; }

    // Getters for individual stats with bounds checking
    int getStat(int id) const;
    int getPersistantValue(int id) const;
    int getAmmoValue(int id) const;
    int getPowerupValue(int id) const;
};

class PilotState : public PlayerState {
public:
    PilotState() : PlayerState(STATE_PILOTSTATE) {}
    ~PilotState() override = default;

    void report(std::ostream& os) const override;
    void save() const override;
    void load() override;

    bool hasVehicleSet() const override;

    bool isAttributeFloat(int id) const override;
    bool isAttributeInteger(int id) const override;

    std::unique_ptr<PlayerState> clone() const override;
};

class VehicleState : public PlayerState {
public:
    VehicleState() : PlayerState(STATE_VEHICLESTATE) {}
    ~VehicleState() override = default;

    void report(std::ostream& os) const override;
    void save() const override;
    void load() override;

    bool isAttributeFloat(int id) const override;
    bool isAttributeInteger(int id) const override;

    std::unique_ptr<PlayerState> clone() const override;
};

DEMO_NAMESPACE_END

#endif