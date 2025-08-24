// entitystate_instr.hpp - Implementation for EntityState delta instructions  
#pragma once
#include "messagebuffer.hpp"
#include "netfields.h"
#include <unordered_map>
#include <optional>

namespace jka {

    /**
     * @brief Container for EntityState delta instructions
     * 
     * Similar to PlayerStateInstr but for entities. Stores entity number
     * plus delta fields that changed from baseline.
     */
    class EntityStateInstr {
    private:
        int entityNumber_ = -1; ///< Entity number (always transmitted first)
        std::unordered_map<int, int> changedFields_; ///< fieldIndex -> value
        
    public:
        EntityStateInstr() = default;
        explicit EntityStateInstr(int entityNum) : entityNumber_(entityNum) {}
        
        /**
         * @brief Parse EntityState delta from network buffer
         * @param buf MessageBuffer containing compressed delta data
         * 
         * Protocol format:
         * - Read entity number (10 bits, ENTITY_NUMBER field)  
         * - Read field index (0 = end of fields)
         * - Read field value based on NetField definition
         * - Repeat until field index 0
         */
        void read(MessageBuffer& buf) {
            changedFields_.clear();
            
            // Entity number is always transmitted first
            entityNumber_ = buf.readBits(10); // ENTITY_NUMBER is 10 bits
            
            while (true) {
                int fieldNum = buf.readByte();
                if (fieldNum == 0) break; // End of changed fields
                
                if (fieldNum < 0 || fieldNum >= static_cast<int>(EntityNetfields.size())) {
                    throw std::runtime_error("Invalid EntityState field number: " + 
                                           std::to_string(fieldNum));
                }
                
                const NetField& field = EntityNetfields[fieldNum];
                
                // Read value based on field bit width
                int value;
                if (field.bits <= 8) {
                    value = field.isSigned() ? buf.readSignedByte() : buf.readByte();
                } else if (field.bits <= 16) {
                    value = field.isSigned() ? buf.readSignedShort() : buf.readShort();
                } else if (field.bits <= 24) {
                    value = buf.readBits(field.bits); // For packed coordinates, etc.
                } else {
                    value = field.isSigned() ? buf.readSignedInt() : buf.readInt();
                }
                
                changedFields_[fieldNum] = value;
            }
        }
        
        /**
         * @brief Write EntityState delta to network buffer
         * @param buf MessageBuffer to write delta data to
         */
        void write(MessageBuffer& buf) const {
            if (entityNumber_ < 0) {
                throw std::runtime_error("EntityState: invalid entity number for write");
            }
            
            // Write entity number first
            buf.writeBits(entityNumber_, 10);
            
            // Write changed fields
            for (const auto& [fieldNum, value] : changedFields_) {
                buf.writeByte(fieldNum);
                
                const NetField& field = EntityNetfields[fieldNum];
                
                if (field.bits <= 8) {
                    buf.writeByte(value);
                } else if (field.bits <= 16) {
                    buf.writeShort(value);
                } else if (field.bits <= 24) {
                    buf.writeBits(value, field.bits);
                } else {
                    buf.writeInt(value);
                }
            }
            buf.writeByte(0); // End marker
        }
        
        /**
         * @brief Get entity number
         * @return Entity number (0-1023 for JKA)
         */
        int getNumber() const {
            return entityNumber_;
        }
        
        /**
         * @brief Set entity number
         * @param entityNum Entity number (0-1023 for JKA)
         */
        void setNumber(int entityNum) {
            if (entityNum < 0 || entityNum >= 1024) {
                throw std::runtime_error("Invalid entity number: " + std::to_string(entityNum));
            }
            entityNumber_ = entityNum;
        }
        
        /**
         * @brief Check if a specific field changed from baseline
         * @param fieldIndex Index into EntityNetfields array
         * @return true if field has a new value
         */
        bool hasField(int fieldIndex) const {
            return changedFields_.find(fieldIndex) != changedFields_.end();
        }
        
        /**
         * @brief Get new value for a changed field
         * @param fieldIndex Index into EntityNetfields array
         * @return Field value, or throws if field wasn't changed
         */
        int getField(int fieldIndex) const {
            auto it = changedFields_.find(fieldIndex);
            if (it == changedFields_.end()) {
                throw std::runtime_error("EntityState field " + std::to_string(fieldIndex) + 
                                       " was not changed in this delta");
            }
            return it->second;
        }
        
        /**
         * @brief Get new value for a changed field (safe version)
         * @param fieldIndex Index into EntityNetfields array
         * @return Field value if changed, nullopt otherwise
         */
        std::optional<int> getFieldSafe(int fieldIndex) const {
            auto it = changedFields_.find(fieldIndex);
            return (it != changedFields_.end()) ? std::optional<int>(it->second) : std::nullopt;
        }
        
        /**
         * @brief Set a field value (for creating deltas)
         * @param fieldIndex Index into EntityNetfields array  
         * @param value New field value
         */
        void setField(int fieldIndex, int value) {
            if (fieldIndex < 0 || fieldIndex >= static_cast<int>(EntityNetfields.size())) {
                throw std::runtime_error("Invalid EntityState field index: " + 
                                       std::to_string(fieldIndex));
            }
            changedFields_[fieldIndex] = value;
        }
        
        /**
         * @brief Remove a field from delta
         * @param fieldIndex Index into EntityNetfields array
         */
        void removeField(int fieldIndex) {
            changedFields_.erase(fieldIndex);
        }
        
        /**
         * @brief Get all changed fields
         * @return Map of fieldIndex -> value for all changed fields
         */
        const std::unordered_map<int, int>& getChangedFields() const {
            return changedFields_;
        }
        
        /**
         * @brief Check if any fields changed (besides entity number)
         * @return true if this is an empty delta
         */
        bool isEmpty() const {
            return changedFields_.empty();
        }
        
        /**
         * @brief Get number of changed fields
         * @return Count of fields that differ from baseline
         */
        size_t getChangedFieldCount() const {
            return changedFields_.size();
        }
        
        /**
         * @brief Clear all changes (but keep entity number)
         */
        void clear() {
            changedFields_.clear();
        }
        
        /**
         * @brief Check if this entity instruction is valid
         * @return true if entity number is valid
         */
        bool isValid() const {
            return entityNumber_ >= 0 && entityNumber_ < 1024;
        }
    };

} // namespace jka