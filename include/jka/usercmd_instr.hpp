#pragma once
#include <cstdint>
#include <memory>
#include <iostream>

#include <jka/instruction.h>   // base Instruction + INSTR_USERCMD
#include <jka/message.hpp>     // Message (encapsule MessageBuffer)
#include <jka/usercmd.hpp>     // struct UserCmd moderne (serverTime, angles[3], buttons, weapon, forward/right/up)

namespace jka {

/**
 * Bit flags DM_26 pour delta usercmd (aligné avec Q3/OpenJK).
 * Si un bit est présent dans "bits", le champ correspondant est sérialisé.
 */
struct UserCmdDeltaBits {
    static constexpr std::uint32_t SERVER_TIME  = 1u << 0;
    static constexpr std::uint32_t ANGLE1       = 1u << 1;
    static constexpr std::uint32_t ANGLE2       = 1u << 2;
    static constexpr std::uint32_t ANGLE3       = 1u << 3;
    static constexpr std::uint32_t BUTTONS      = 1u << 4;
    static constexpr std::uint32_t WEAPON       = 1u << 5;
    static constexpr std::uint32_t FORWARDMOVE  = 1u << 6;
    static constexpr std::uint32_t RIGHTMOVE    = 1u << 7;
    static constexpr std::uint32_t UPMOVE       = 1u << 8;
    // (JKA peut étendre ; ajouter ici si nécessaire)
};

/**
 * Helpers bas niveau pour écrire/lire des entiers de taille fixe via Message.
 * On utilise writeBytes/readBytes pour rester neutre vis-à-vis de l’endianness
 * de ton projet (tes helpers internes peuvent déjà la gérer).
 */
inline void write_u32(Message& msg, std::uint32_t v) { msg.writeBytes(&v, sizeof(v)); }
inline void write_i32(Message& msg, std::int32_t  v) { msg.writeBytes(&v, sizeof(v)); }
inline void write_u16(Message& msg, std::uint16_t v) { msg.writeBytes(&v, sizeof(v)); }
inline void write_i16(Message& msg, std::int16_t  v) { msg.writeBytes(&v, sizeof(v)); }
inline void write_u8 (Message& msg, std::uint8_t  v) { msg.writeBytes(&v, sizeof(v)); }
inline void write_i8 (Message& msg, std::int8_t   v) { msg.writeBytes(&v, sizeof(v)); }

inline void read_u32(Message& msg, std::uint32_t& v) { msg.readBytes(&v, sizeof(v)); }
inline void read_i32(Message& msg, std::int32_t&  v) { msg.readBytes(&v, sizeof(v)); }
inline void read_u16(Message& msg, std::uint16_t& v) { msg.readBytes(&v, sizeof(v)); }
inline void read_i16(Message& msg, std::int16_t&  v) { msg.readBytes(&v, sizeof(v)); }
inline void read_u8 (Message& msg, std::uint8_t&  v) { msg.readBytes(&v, sizeof(v)); }
inline void read_i8 (Message& msg, std::int8_t&   v) { msg.readBytes(&v, sizeof(v)); }

/**
 * Optionnel : obfuscation “keying” à la Quake.
 * Par défaut key=0 → noop.
 * NOTE: L’implémentation exacte d’OpenJK peut faire évoluer le key entre octets.
 * Ici on laisse un hook trivial (XOR sur octets constants). Si tu veux brancher
 * la version exacte, remplace ces wrappers par ta logique et garde l’API.
 */
inline void writeBytesKeyed(Message& msg, const void* data, size_t size, std::uint32_t key = 0) {
    if (key == 0) {
        msg.writeBytes(data, size);
        return;
    }
    const std::uint8_t* p = static_cast<const std::uint8_t*>(data);
    for (size_t i = 0; i < size; ++i) {
        std::uint8_t obf = static_cast<std::uint8_t>(p[i] ^ (key & 0xFF));
        msg.writeBytes(&obf, 1);
        // hook pour faire évoluer le key si besoin :
        // key = ROTL(key) ^ obf;  // à remplacer par ton schéma exact si nécessaire
    }
}

inline void readBytesKeyed(Message& msg, void* data, size_t size, std::uint32_t key = 0) {
    if (key == 0) {
        msg.readBytes(data, size);
        return;
    }
    std::uint8_t* p = static_cast<std::uint8_t*>(data);
    for (size_t i = 0; i < size; ++i) {
        std::uint8_t obf{};
        msg.readBytes(&obf, 1);
        p[i] = static_cast<std::uint8_t>(obf ^ (key & 0xFF));
        // key = ROTL(key) ^ obf; // idem commentaire ci-dessus
    }
}

/**
 * Écriture/lecture delta d’un UserCmd (façon MSG_*DeltaUsercmdKey)
 * - On écrit d’abord un bitmask 32-bit.
 * - Puis, dans l’ordre, on n’écrit que les champs dont le bit est présent.
 * - On lit de la même manière (bitmask puis champs présents), et on reconstruit
 *   `to` en copiant `from` puis en appliquant les champs présents.
 */
struct UserCmdDeltaCodec {

    static std::uint32_t computeBits(const UserCmd& from, const UserCmd& to) {
        std::uint32_t bits = 0;
        if (to.serverTime   != from.serverTime)   bits |= UserCmdDeltaBits::SERVER_TIME;
        if (to.angles[0]    != from.angles[0])    bits |= UserCmdDeltaBits::ANGLE1;
        if (to.angles[1]    != from.angles[1])    bits |= UserCmdDeltaBits::ANGLE2;
        if (to.angles[2]    != from.angles[2])    bits |= UserCmdDeltaBits::ANGLE3;
        if (to.buttons      != from.buttons)      bits |= UserCmdDeltaBits::BUTTONS;
        if (to.weapon       != from.weapon)       bits |= UserCmdDeltaBits::WEAPON;
        if (to.forwardmove  != from.forwardmove)  bits |= UserCmdDeltaBits::FORWARDMOVE;
        if (to.rightmove    != from.rightmove)    bits |= UserCmdDeltaBits::RIGHTMOVE;
        if (to.upmove       != from.upmove)       bits |= UserCmdDeltaBits::UPMOVE;
        return bits;
    }

    static void writeDelta(Message& msg, const UserCmd& from, const UserCmd& to, std::uint32_t key = 0) {
        const std::uint32_t bits = computeBits(from, to);
        write_u32(msg, bits); // le bitmask lui-même n’est pas “keyed” traditionnellement ; laisse-le brut

        if (bits & UserCmdDeltaBits::SERVER_TIME) {
            const std::int32_t v = to.serverTime;
            writeBytesKeyed(msg, &v, sizeof(v), key);
        }
        if (bits & UserCmdDeltaBits::ANGLE1) {
            const std::int16_t v = to.angles[0];
            writeBytesKeyed(msg, &v, sizeof(v), key);
        }
        if (bits & UserCmdDeltaBits::ANGLE2) {
            const std::int16_t v = to.angles[1];
            writeBytesKeyed(msg, &v, sizeof(v), key);
        }
        if (bits & UserCmdDeltaBits::ANGLE3) {
            const std::int16_t v = to.angles[2];
            writeBytesKeyed(msg, &v, sizeof(v), key);
        }
        if (bits & UserCmdDeltaBits::BUTTONS) {
            const std::uint32_t v = to.buttons;
            writeBytesKeyed(msg, &v, sizeof(v), key);
        }
        if (bits & UserCmdDeltaBits::WEAPON) {
            const std::uint8_t v = to.weapon;
            writeBytesKeyed(msg, &v, sizeof(v), key);
        }
        if (bits & UserCmdDeltaBits::FORWARDMOVE) {
            const std::int8_t v = to.forwardmove;
            writeBytesKeyed(msg, &v, sizeof(v), key);
        }
        if (bits & UserCmdDeltaBits::RIGHTMOVE) {
            const std::int8_t v = to.rightmove;
            writeBytesKeyed(msg, &v, sizeof(v), key);
        }
        if (bits & UserCmdDeltaBits::UPMOVE) {
            const std::int8_t v = to.upmove;
            writeBytesKeyed(msg, &v, sizeof(v), key);
        }
    }

    static void readDelta(Message& msg, const UserCmd& from, UserCmd& to, std::uint32_t key = 0) {
        std::uint32_t bits{};
        read_u32(msg, bits);

        // base = from
        to = from;

        if (bits & UserCmdDeltaBits::SERVER_TIME) {
            std::int32_t v{};
            readBytesKeyed(msg, &v, sizeof(v), key);
            to.serverTime = v;
        }
        if (bits & UserCmdDeltaBits::ANGLE1) {
            std::int16_t v{};
            readBytesKeyed(msg, &v, sizeof(v), key);
            to.angles[0] = v;
        }
        if (bits & UserCmdDeltaBits::ANGLE2) {
            std::int16_t v{};
            readBytesKeyed(msg, &v, sizeof(v), key);
            to.angles[1] = v;
        }
        if (bits & UserCmdDeltaBits::ANGLE3) {
            std::int16_t v{};
            readBytesKeyed(msg, &v, sizeof(v), key);
            to.angles[2] = v;
        }
        if (bits & UserCmdDeltaBits::BUTTONS) {
            std::uint32_t v{};
            readBytesKeyed(msg, &v, sizeof(v), key);
            to.buttons = v;
        }
        if (bits & UserCmdDeltaBits::WEAPON) {
            std::uint8_t v{};
            readBytesKeyed(msg, &v, sizeof(v), key);
            to.weapon = v;
        }
        if (bits & UserCmdDeltaBits::FORWARDMOVE) {
            std::int8_t v{};
            readBytesKeyed(msg, &v, sizeof(v), key);
            to.forwardmove = v;
        }
        if (bits & UserCmdDeltaBits::RIGHTMOVE) {
            std::int8_t v{};
            readBytesKeyed(msg, &v, sizeof(v), key);
            to.rightmove = v;
        }
        if (bits & UserCmdDeltaBits::UPMOVE) {
            std::int8_t v{};
            readBytesKeyed(msg, &v, sizeof(v), key);
            to.upmove = v;
        }
    }
};

/**
 * Instruction encapsulant un UserCmd.
 * - Contient la commande courante `cmd_`.
 * - Peut (optionnellement) contenir une baseline `from_` utilisée pour le delta.
 * - Save/Load utilisent le codec ci-dessus (compatible DM_26).
 */
class UserCmdInstr : public Instruction {
private:
    std::unique_ptr<UserCmd> from_;  // baseline (optionnelle)
    std::unique_ptr<UserCmd> cmd_;   // commande courante

    std::uint32_t key_{0}; // key “Quake-like” optionnel ; 0 => pas d’obfuscation

public:
    UserCmdInstr()
        : Instruction(INSTR_USERCMD),
          cmd_(std::make_unique<UserCmd>()) {}

    explicit UserCmdInstr(std::unique_ptr<UserCmd> current,
                          std::unique_ptr<UserCmd> from = nullptr,
                          std::uint32_t key = 0)
        : Instruction(INSTR_USERCMD),
          from_(std::move(from)),
          cmd_(std::move(current)),
          key_(key) {}

    // --- I/O sur Message (delta DM_26) ---

    // Écrit un delta (from_ → cmd_). Si from_ absent, delta depuis “zero-baseline”.
    void Save(Message& msg) const override {
        const UserCmd zero{};
        const UserCmd& base = (from_ ? *from_ : zero);
        const UserCmd& cur  = *cmd_;
        UserCmdDeltaCodec::writeDelta(msg, base, cur, key_);
    }

    // Lit un delta et reconstruit cmd_ à partir de from_ (ou zero-baseline si absent).
    void Load(Message& msg) override {
        if (!cmd_)  cmd_  = std::make_unique<UserCmd>();
        const UserCmd zero{};
        const UserCmd& base = (from_ ? *from_ : zero);
        UserCmdDeltaCodec::readDelta(msg, base, *cmd_, key_);
    }

    void report(std::ostream& os) const override {
        if (!cmd_) { os << "[UserCmdInstr] <null>\n"; return; }
        os << "[UserCmdInstr] time=" << cmd_->serverTime
           << " ang=(" << cmd_->angles[0] << "," << cmd_->angles[1] << "," << cmd_->angles[2] << ")"
           << " buttons=0x" << std::hex << cmd_->buttons << std::dec
           << " fwd=" << int(cmd_->forwardmove)
           << " right=" << int(cmd_->rightmove)
           << " up=" << int(cmd_->upmove)
           << " weapon=" << int(cmd_->weapon)
           << " key=" << key_ << "\n";
    }

    // --- Accès / config ---
    const UserCmd* current() const noexcept { return cmd_.get(); }
    UserCmd*       current()       noexcept { return cmd_.get(); }

    const UserCmd* baseline() const noexcept { return from_.get(); }
    void           setBaseline(std::unique_ptr<UserCmd> from) { from_ = std::move(from); }

    std::uint32_t  key() const noexcept { return key_; }
    void           setKey(std::uint32_t k) noexcept { key_ = k; }
};

} // namespace jka
