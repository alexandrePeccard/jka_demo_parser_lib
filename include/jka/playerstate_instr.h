// playerstate_instr.hpp - Implementation for PlayerState delta instructions
#pragma once
#include "messagebuffer.hpp"
#include "netfields.h"
#include <unordered_map>
#include <optional>

namespace jka {

    /**
     * @brief Container for PlayerState delta instructions
     * 
     * Stores only the fields that changed from baseline, along with their values.
     * Used by SnapshotParser to parse binary delta data, then by SnapshotConverter
     * to reconstruct complete PlayerState.
     */
    class PlayerStateInstr {
    private:
        std::unordered_map<int, int> changedFields_; ///< fieldIndex -> value
        
    public:
        PlayerStateInstr() = default;
        
        /**
         * @brief Parse PlayerState delta from network buffer
         * @param buf MessageBuffer containing compressed delta data
         * 
         * Protocol format:
         * - Read field index (0 = end of fields)
         * - Read field value based on NetField definition
         * - Repeat until field index 0
         */
        void read(MessageBuffer& buf) {
            changedFields_.clear();
            
            while (true) {
                int fieldNum = buf.readByte();
                if (fieldNum == 0) break; // End of changed fields
                
                if (fieldNum < 0 || fieldNum >= static_cast<int>(PlayerNetfields.size())) {
                    // Invalid field number - protocol error or corruption
                    throw std::runtime_error("Invalid PlayerState field number: " + 
                                           std::to_string(fieldNum));
                }
                
                const NetField& field = PlayerNetfields[fieldNum];
                
                // Read value based on field bit width
                int value;
                if (field.bits <= 8) {
                    value = field.isSigned() ? buf.readSignedByte() : buf.readByte();
                } else if (field.bits <= 16) {
                    value = field.isSigned() ? buf.readSignedShort() : buf.readShort();
                } else {
                    value = field.isSigned() ? buf.readSignedInt() : buf.readInt();
                }
                
                changedFields_[fieldNum] = value;
            }
        }
        
        /**
         * @brief Write PlayerState delta to network buffer
         * @param buf MessageBuffer to write delta data to
         * 
         * Inverse of read() - writes field indexes and values, terminated by 0
         */
        void write(MessageBuffer& buf) const {
            for (const auto& [fieldNum, value] : changedFields_) {
                buf.writeByte(fieldNum);
                
                const NetField& field = PlayerNetfields[fieldNum];
                
                // Write value based on field bit width
                if (field.bits <= 8) {
                    buf.writeByte(value);
                } else if (field.bits <= 16) {
                    buf.writeShort(value);
                } else {
                    buf.writeInt(value);
                }
            }
            buf.writeByte(0); // End marker
        }
        
        /**
         * @brief Check if a specific field changed from baseline
         * @param fieldIndex Index into PlayerNetfields array
         * @return true if field has a new value
         */
        bool hasField(int fieldIndex) const {
            return changedFields_.find(fieldIndex) != changedFields_.end();
        }
        
        /**
         * @brief Get new value for a changed field
         * @param fieldIndex Index into PlayerNetfields array
         * @return Field value, or throws if field wasn't changed
         */
        int getField(int fieldIndex) const {
            auto it = changedFields_.find(fieldIndex);
            if (it == changedFields_.end()) {
                throw std::runtime_error("PlayerState field " + std::to_string(fieldIndex) + 
                                       " was not changed in this delta");
            }
            return it->second;
        }
        
        /**
         * @brief Get new value for a changed field (safe version)
         * @param fieldIndex Index into PlayerNetfields array
         * @return Field value if changed, nullopt otherwise
         */
        std::optional<int> getFieldSafe(int fieldIndex) const {
            auto it = changedFields_.find(fieldIndex);
            return (it != changedFields_.end()) ? std::optional<int>(it->second) : std::nullopt;
        }
        
        /**
         * @brief Set a field value (for creating deltas)
         * @param fieldIndex Index into PlayerNetfields array
         * @param value New field value
         */
        void setField(int fieldIndex, int value) {
            if (fieldIndex < 0 || fieldIndex >= static_cast<int>(PlayerNetfields.size())) {
                throw std::runtime_error("Invalid PlayerState field index: " + 
                                       std::to_string(fieldIndex));
            }
            changedFields_[fieldIndex] = value;
        }
        
        /**
         * @brief Remove a field from delta (revert to baseline)
         * @param fieldIndex Index into PlayerNetfields array
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
         * @brief Check if any fields changed
         * @return true if this is an empty delta (no changes)
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
         * @brief Clear all changes
         */
        void clear() {
            changedFields_.clear();
        }
    };

} // namespace jka