#pragma once
// snapshot_converter.hpp - Bridge between SnapshotParser and final Snapshot
// Connects your existing architecture: Parser → Instructions → Converter → Complete State

#include "snapshot.hpp"
#include "snapshot_instr.hpp"
#include "playerstate_adapter.hpp"
#include "entitystate_adapter.hpp"
#include "netfields.h"

namespace jka {

    /**
     * @brief Converts delta instructions to complete world state
     * 
     * Takes SnapshotInstr (delta/compressed data) + baseline state
     * Returns complete Snapshot ready for game logic consumption
     */
    class SnapshotConverter {
    public:
        /**
         * @brief Convert instruction-based snapshot to complete state
         * @param instr Raw instructions from SnapshotParser
         * @param baseline Previous complete state (for delta decompression)
         * @return Complete snapshot with all entities and states resolved
         */
        static Snapshot fromInstructions(const SnapshotInstr& instr, 
                                        const Snapshot& baseline = {}) {
            Snapshot result;
            
            // 1. Copy metadata directly
            result.serverTime = instr.serverTime;
            result.deltaNum = instr.deltaNum;
            result.flags = instr.snapFlags;
            result.areaMask = instr.areamask;
            
            // 2. Convert PlayerState (delta → complete)
            if (instr.playerStateInstr) {
                result.playerState = convertPlayerState(*instr.playerStateInstr, 
                                                       baseline.playerState);
            } else {
                result.playerState = baseline.playerState; // No changes
            }
            
            // 3. Start with baseline entities
            result.entities = baseline.entities;
            
            // 4. Apply entity changes from instructions
            for (const auto& entInstr : instr.entitiesInstr) {
                int entityNum = extractEntityNumber(*entInstr);
                
                // Find baseline for this entity
                const EntityState* baselineEntity = baseline.findEntity(entityNum);
                EntityState baselineDefault{}; // Empty if not found
                
                // Convert delta instruction to complete entity state
                EntityState newState = convertEntityState(*entInstr, 
                    baselineEntity ? *baselineEntity : baselineDefault);
                
                // Update result
                result.entities[entityNum] = newState;
            }
            
            return result;
        }

    private:
        /**
         * @brief Convert PlayerStateInstr (delta) to complete PlayerState
         * @param instr Delta instructions for player
         * @param baseline Previous complete player state
         * @return Complete player state with deltas applied
         */
        static PlayerState convertPlayerState(const PlayerStateInstr& instr, 
                                             const PlayerState& baseline) {
            PlayerState result = baseline; // Start with previous state
            
            // Apply each changed field from instructions
            for (size_t fieldIdx = 0; fieldIdx < PlayerNetfields.size(); ++fieldIdx) {
                if (instr.hasField(fieldIdx)) {
                    const NetField& field = PlayerNetfields[fieldIdx];
                    int value = instr.getField(fieldIdx);
                    
                    // Apply field value to result based on field type
                    applyPlayerField(result, field, value);
                }
            }
            
            return result;
        }

        /**
         * @brief Convert EntityStateInstr (delta) to complete EntityState
         * @param instr Delta instructions for entity  
         * @param baseline Previous complete entity state
         * @return Complete entity state with deltas applied
         */
        static EntityState convertEntityState(const EntityStateInstr& instr,
                                             const EntityState& baseline) {
            EntityState result = baseline; // Start with previous state
            
            // Apply each changed field from instructions
            for (size_t fieldIdx = 0; fieldIdx < EntityNetfields.size(); ++fieldIdx) {
                if (instr.hasField(fieldIdx)) {
                    const NetField& field = EntityNetfields[fieldIdx];
                    int value = instr.getField(fieldIdx);
                    
                    // Apply field value to result based on field type
                    applyEntityField(result, field, value);
                }
            }
            
            return result;
        }

        /**
         * @brief Apply a single field value to PlayerState based on netfield definition
         */
        static void applyPlayerField(PlayerState& ps, const NetField& field, int value) {
            using namespace std::string_view_literals;
            
            // Handle different field types
            switch (field.type) {
                case FieldType::Time:
                case FieldType::Int:
                    if (field.name == "commandTime"sv) {
                        ps.commandTime = value;
                    } else if (field.name == "pm_type"sv) {
                        ps.pm_type = static_cast<PMType>(value);
                    } else if (field.name == "weaponTime"sv) {
                        ps.weaponTime = value;
                    } else if (field.name == "gravity"sv) {
                        ps.gravity = value;
                    } else if (field.name == "speed"sv) {
                        ps.speed = value;
                    } else if (field.name == "groundEntityNum"sv) {
                        ps.groundEntityNum = value;
                    } else if (field.name == "legsAnim"sv) {
                        ps.legsAnim = value;
                    } else if (field.name == "torsoAnim"sv) {
                        ps.torsoAnim = value;
                    } else if (field.name == "movementDir"sv) {
                        ps.movementDir = value;
                    } else if (field.name == "eventSequence"sv) {
                        ps.eventSequence = value;
                    } else if (field.name == "clientNum"sv) {
                        ps.clientNum = value;
                    } else if (field.name == "weapon"sv) {
                        ps.weapon = value;
                    }
                    break;
                    
                case FieldType::Origin:
                    if (field.name == "origin"sv) {
                        // Origin is transmitted as 3 packed coordinates
                        // This is simplified - real implementation would unpack 3D coords
                        ps.origin = Vec3f{static_cast<float>(value & 0xFFFF), 
                                         static_cast<float>((value >> 16) & 0xFFFF), 0};
                    }
                    break;
                    
                case FieldType::Vector:
                    if (field.name == "velocity"sv) {
                        // Velocity is transmitted as 3 packed coordinates  
                        ps.velocity = Vec3f{static_cast<float>(value & 0xFFFF),
                                           static_cast<float>((value >> 16) & 0xFFFF), 0};
                    }
                    break;
                    
                case FieldType::Angle:
                    if (field.name == "delta_angles"sv) {
                        // Delta angles - simplified unpacking
                        ps.delta_angles = Vec3i{value & 0xFFFF, (value >> 16) & 0xFFFF, 0};
                    } else if (field.name == "viewangles"sv) {
                        ps.viewangles = Vec3f{static_cast<float>(value & 0xFFFF) / field.divisor,
                                             static_cast<float>((value >> 16) & 0xFFFF) / field.divisor, 0};
                    }
                    break;
                    
                case FieldType::Entity:
                    // Entity references - already handled above
                    break;
            }
        }

        /**
         * @brief Apply a single field value to EntityState based on netfield definition
         */
        static void applyEntityField(EntityState& es, const NetField& field, int value) {
            using namespace std::string_view_literals;
            
            switch (field.type) {
                case FieldType::Int:
                    if (field.name == "number"sv) {
                        es.number = value;
                    } else if (field.name == "eType"sv) {
                        es.eTypeRaw = value;
                        es.eType = static_cast<EntityType>(value);
                    } else if (field.name == "torsoAnim"sv) {
                        es.torsoAnim = value;
                    } else if (field.name == "legsAnim"sv) {
                        es.legsAnim = value;
                    } else if (field.name == "weapon"sv) {
                        es.weapon = value;
                    } else if (field.name == "clientNum"sv) {
                        // Note: entities don't typically have clientNum, 
                        // this might be specific to certain entity types
                    }
                    break;
                    
                case FieldType::Time:
                    if (field.name == "time"sv) {
                        es.time = value;
                    }
                    break;
                    
                case FieldType::Entity:
                    if (field.name == "otherEntityNum"sv) {
                        es.otherEntityNum = value;
                    }
                    break;
                    
                case FieldType::Origin:
                    if (field.name == "pos.trBase"sv) {
                        // Update trajectory base position
                        es.pos.trBase = Vec3f{static_cast<float>(value & 0xFFFF),
                                             static_cast<float>((value >> 16) & 0xFFFF), 0};
                    }
                    break;
                    
                case FieldType::Angle:
                    if (field.name == "apos.trBase"sv) {
                        // Update angular trajectory base
                        es.apos.trBase = Vec3f{static_cast<float>(value & 0xFFFF) / field.divisor,
                                              static_cast<float>((value >> 16) & 0xFFFF) / field.divisor, 0};
                    }
                    break;
            }
        }

        /**
         * @brief Extract entity number from EntityStateInstr
         * @note This assumes EntityStateInstr has a method to get entity number
         * You'll need to implement this in EntityStateInstr class
         */
        static int extractEntityNumber(const EntityStateInstr& instr) {
            // This assumes EntityStateInstr has a way to get the entity number
            // You'll need to implement getNumber() or similar in EntityStateInstr
            // For now, return a placeholder that you'll need to replace
            
            // PLACEHOLDER - Replace with actual implementation
            // return instr.getNumber(); 
            
            // Temporary fallback - you'll need to implement this properly
            if (instr.hasField(0)) { // Assuming field 0 is "number" 
                return instr.getField(0);
            }
            return -1; // Invalid entity number
        }
    };

} // namespace jka