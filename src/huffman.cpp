#include "huffman.hpp"
#include <algorithm>
#include <cstring>

namespace jka {

// ===== BitWriter =====
AdaptiveHuffman::BitWriter::BitWriter(std::vector<uint8_t>& out) : out_(out) {}

void AdaptiveHuffman::BitWriter::putBit(int bit) {
    if (bit) cur_ |= (1u << bitpos_);
    ++bitpos_;
    if (bitpos_ == 8) flushByte();
}

void AdaptiveHuffman::BitWriter::putByte(uint8_t b) {
    // LSB-first (comme les msg_t de Quake3/OpenJK)
    for (int i = 0; i < 8; ++i) {
        putBit((b >> i) & 1);
    }
}

void AdaptiveHuffman::BitWriter::flushByte() {
    out_.push_back(cur_);
    cur_ = 0;
    bitpos_ = 0;
}

void AdaptiveHuffman::BitWriter::flushPartialByte() {
    if (bitpos_ > 0) flushByte();
}

// ===== BitReader =====
AdaptiveHuffman::BitReader::BitReader(const uint8_t* data, size_t len) : data_(data), len_(len) {}

bool AdaptiveHuffman::BitReader::eof() const {
    return pos_ >= len_ && bitpos_ == 0;
}

int AdaptiveHuffman::BitReader::getBit() {
    if (pos_ >= len_) return -1;
    int bit = (data_[pos_] >> bitpos_) & 1;
    ++bitpos_;
    if (bitpos_ == 8) {
        bitpos_ = 0;
        ++pos_;
    }
    return bit;
}

bool AdaptiveHuffman::BitReader::getByte(uint8_t& out) {
    int v = 0;
    for (int i=0;i<8;++i) {
        int b = getBit();
        if (b < 0) return false;
        v |= (b << i);
    }
    out = static_cast<uint8_t>(v);
    return true;
}

// ===== Context =====

AdaptiveHuffman::AdaptiveHuffman() { reset(); }

void AdaptiveHuffman::Context::reset() {
    blocNode = 0;
    loc.fill(nullptr);
    lhead = ltail = nullptr;
    blocPtrs = 0;
    tree = nyt = nullptr;

    Node* r = newNode();
    r->symbol = NYT;
    r->weight = 0;
    r->parent = r->left = r->right = nullptr;
    r->next = r->prev = nullptr;

    Node** headPtr = getHeadSlot();
    *headPtr = r;
    r->head = headPtr;

    lhead = ltail = r;
    tree = nyt = r;
}

AdaptiveHuffman::Node* AdaptiveHuffman::Context::newNode() {
    if (blocNode >= static_cast<int>(pool.size()))
        throw std::runtime_error("AdaptiveHuffman: node pool exhausted");
    Node* n = &pool[blocNode++];
    n->symbol = NYT;
    n->weight = 0;
    n->parent = n->left = n->right = nullptr;
    n->next = n->prev = nullptr;
    n->head = nullptr;
    return n;
}

AdaptiveHuffman::Node** AdaptiveHuffman::Context::getHeadSlot() {
    if (blocPtrs >= static_cast<int>(pppool.size()))
        throw std::runtime_error("AdaptiveHuffman: head slot pool exhausted");
    Node** slot = &pppool[blocPtrs++];
    *slot = nullptr;
    return slot;
}

// ===== mechanics =====

bool AdaptiveHuffman::isLeaf(const Node* n) { return n && !n->left && !n->right; }

AdaptiveHuffman::Node* AdaptiveHuffman::highestInBlock(Node* n) {
    if (!n || !n->head) return n;
    return *n->head ? *n->head : n;
}

void AdaptiveHuffman::swap(Context& h, Node* a, Node* b) {
    if (a == b || !a || !b) return;

    // swap dans la liste doublement chaînée
    if (a->next == b) {
        Node* ap = a->prev;
        Node* bn = b->next;
        b->prev = ap; if (ap) ap->next = b; else h.lhead = b;
        b->next = a;
        a->prev = b;
        a->next = bn; if (bn) bn->prev = a; else h.ltail = a;
    } else if (b->next == a) {
        swap(h, b, a);
        return;
    } else {
        std::swap(a->prev, b->prev);
        std::swap(a->next, b->next);
        if (a->prev) a->prev->next = a; else h.lhead = a;
        if (a->next) a->next->prev = a; else h.ltail = a;
        if (b->prev) b->prev->next = b; else h.lhead = b;
        if (b->next) b->next->prev = b; else h.ltail = b;
    }

    // ré-attacher au parent
    if (a->parent == b->parent) {
        // même parent -> inverser left/right
        Node* p = a->parent;
        std::swap(p->left, p->right);
    } else {
        if (a->parent) {
            if (a->parent->left == a) a->parent->left = b; else a->parent->right = b;
        }
        if (b->parent) {
            if (b->parent->left == b) b->parent->left = a; else b->parent->right = a;
        }
        std::swap(a->parent, b->parent);
    }

    // les deux nodes gardent leurs pointeurs head, mais on échange le slot référencé
    std::swap(a->head, b->head);
}

void AdaptiveHuffman::increment(Context& h, Node* n) {
    while (n) {
        Node* highest = highestInBlock(n);
        if (highest != n && n->parent != highest) {
            swap(h, n, highest);
        }
        // mettre à jour la tête du "block" (poids identique)
        if (n->prev && n->prev->weight == n->weight) {
            *n->head = n->prev;
        } else {
            *n->head = n;
        }
        n->weight++;
        // si le suivant a un poids différent, créer une nouvelle tête de block
        if (n->next && n->next->weight != n->weight) {
            Node** headPtr = h.getHeadSlot();
            *headPtr = n;
            n->head = headPtr;
        }
        n = n->parent;
    }
}

void AdaptiveHuffman::addRef(Context& h, int ch) {
    Node* n = h.loc[ch];
    if (!n) {
        // split du NYT : création d’un interne + feuille
        Node* oldNyt   = h.nyt;
        Node* internal = h.newNode();
        Node* leaf     = h.newNode();

        internal->symbol = INTERNAL_NODE;
        internal->weight = 1;

        // branchements
        internal->left   = oldNyt;
        internal->right  = leaf;
        internal->parent = oldNyt->parent;

        if (oldNyt->parent) {
            if (oldNyt->parent->left == oldNyt) oldNyt->parent->left = internal;
            else oldNyt->parent->right = internal;
        } else {
            h.tree = internal;
        }
        oldNyt->parent = internal;

        leaf->symbol = ch;
        leaf->weight = 1;
        leaf->parent = internal;

        // insertion dans la liste : oldNyt -> internal -> leaf -> (ancien suivant)
        internal->prev = oldNyt;
        internal->next = oldNyt->next;
        if (internal->next) internal->next->prev = internal; else h.ltail = internal;
        oldNyt->next = internal;

        leaf->prev = internal;
        leaf->next = internal->next;
        if (leaf->next) leaf->next->prev = leaf; else h.ltail = leaf;
        internal->next = leaf;

        // gestion des "block heads" (poids 1)
        if (leaf->next && leaf->next->weight == 1) {
            internal->head = leaf->next->head;
        } else {
            Node** hp = h.getHeadSlot();
            *hp = internal;
            internal->head = hp;
        }
        leaf->head = internal->head;

        // maj table des feuilles
        h.loc[ch] = leaf;

        // le NYT reste la même node (poids 0)
        h.nyt = oldNyt;

        // propager l’incrément (depuis le parent si présent, sinon internal)
        increment(h, internal->parent ? internal->parent : internal);
    } else {
        increment(h, n);
    }
}

void AdaptiveHuffman::sendPathToNode(Context& /*h*/, BitWriter& bw, Node* n) {
    // remonter jusqu’à la racine et empiler les bits, puis les émettre à l’endroit
    std::array<int, NODE_CAPACITY> stack{};
    int top = 0;
    Node* cur = n;
    while (cur && cur->parent) {
        int bit = (cur == cur->parent->right) ? 1 : 0;
        stack[top++] = bit;
        cur = cur->parent;
    }
    for (int i = top-1; i >= 0; --i) {
        bw.putBit(stack[i]);
    }
}

void AdaptiveHuffman::sendSymbol(Context& h, BitWriter& bw, int ch) {
    Node* n = h.loc[ch];
    if (!n) {
        // émettre le chemin du NYT puis l’octet brut (8 bits LSB-first)
        sendPathToNode(h, bw, h.nyt);
        bw.putByte(static_cast<uint8_t>(ch));
    } else {
        sendPathToNode(h, bw, n);
    }
    addRef(h, ch);
}

bool AdaptiveHuffman::receiveSymbol(Context& h, BitReader& br, int& outSym) {
    Node* n = h.tree;
    if (!n) return false;
    while (true) {
        if (isLeaf(n)) {
            if (n->symbol == NYT) {
                uint8_t lit = 0;
                if (!br.getByte(lit)) return false; // fin propre
                outSym = lit;
            } else if (n->symbol == INTERNAL_NODE) {
                return false; // ne doit pas arriver
            } else {
                outSym = n->symbol;
            }
            addRef(h, outSym);
            return true;
        }
        int bit = br.getBit();
        if (bit < 0) return false; // fin des bits
        n = (bit ? n->right : n->left);
        if (!n) return false; // flux corrompu
    }
}

// ===== public API =====

void AdaptiveHuffman::compress(const uint8_t* data, size_t len, std::vector<uint8_t>& out) {
    BitWriter bw(out);
    for (size_t i=0;i<len;++i) {
        sendSymbol(enc_, bw, data[i]);
    }
    bw.flushPartialByte();
}

std::vector<uint8_t> AdaptiveHuffman::compress(const std::vector<uint8_t>& in) {
    std::vector<uint8_t> out;
    out.reserve(in.size() / 2 + 16);
    compress(in.data(), in.size(), out);
    return out;
}

void AdaptiveHuffman::decompress(const uint8_t* data, size_t len, std::vector<uint8_t>& out) {
    BitReader br(data, len);
    while (true) {
        int sym;
        if (!receiveSymbol(dec_, br, sym)) break;
        if (sym < 0 || sym > 255) throw std::runtime_error("AdaptiveHuffman: invalid symbol decoded");
        out.push_back(static_cast<uint8_t>(sym));
    }
}

std::vector<uint8_t> AdaptiveHuffman::decompress(const std::vector<uint8_t>& in) {
    std::vector<uint8_t> out;
    out.reserve(in.size() * 2 + 16);
    decompress(in.data(), in.size(), out);
    return out;
}

void AdaptiveHuffman::reset() {
    enc_.reset();
    dec_.reset();
}

} // namespace jka
