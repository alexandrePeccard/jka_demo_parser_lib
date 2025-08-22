#ifndef DEMO_H
#define DEMO_H

#include <memory>
#include <string>
#include <string_view>
#include <fstream>
#include <jka/message.hpp>

namespace jka {

    enum VehicleCheck {
        VEHICLE_NOT_CHECKED = 0,
        VEHICLE_INSIDE,
        VEHICLE_NOT_INSIDE
    };

    class DemoImpl;

    /**
     * @brief High-level interface for working with JKA demo files (dm_26).
     *
     * Encapsulates demo parsing, indexing, message access, and analysis
     * such as map transitions and vehicle states.
     */
    class Demo {
    private:
        std::unique_ptr<DemoImpl> impl;

    public:
        Demo();
        ~Demo();

        /// Attempts to load demo file with given name and optional analysis.
        /// @param filename path to the demo file
        /// @param analysis whether to perform analysis after loading
        /// @return true if successful
        bool open(std::string_view filename, bool analysis = true);

        /// Checks whether a demo file is currently loaded.
        bool isOpen() const noexcept;

        /// Closes demo and clears all resources (also called in destructor).
        void close();

        /// Saves current content to a demo file, overwriting if necessary.
        /// @param filename output path
        /// @param endSign append two consecutive -1 messages as terminator
        /// @return true if successful
        bool save(std::string_view filename, bool endSign = false) const;

        /// Ensures message with given id is loaded into memory.
        void loadMessage(int id);

        /// Checks if message with given id is loaded in memory.
        bool isMessageLoaded(int id) const;

        /// Unloads message with given id, keeping only metadata.
        void unloadMessage(int id);

        /// Performs analysis: map transitions, restarts, vehicle states.
        void analyse();

        /// Returns pointer to a message (loads if not already loaded).
        Message* getMessage(int id);

        /// Total number of messages in the demo.
        int getMessageCount() const;

        /// Number of map changes/restarts (requires analyse()).
        int getMapsCount() const;

        /// Name of the map at given index (requires analyse()).
        std::string getMapName(int mapId) const;

        /// Message index of first snapshot for given map (requires analyse()).
        int getMapId(int mapId) const;

        /// True if mapId corresponds to a restart instead of a new map.
        bool isMapRestart(int mapId) const;

        /// Starting time of map (from configstring 21) (requires analyse()).
        int getMapStartTime(int mapId) const;

        /// Ending time of map (next map/restart or demo end) (requires analyse()).
        int getMapEndTime(int mapId) const;

        /// Saves selected message to output stream.
        void saveMessage(int id, std::ofstream& os) const;

        /// Deletes one or several messages, including metadata and analysis info.
        /// WARNING: This corrupts the demo unless references are fixed afterwards.
        void deleteMessage(int startid, int endid = 0);
    };
}
#endif