#ifndef __TOOLS_HH__
#define __TOOLS_HH__

#include <BF.h>
#include <sstream>



inline BF encodeInBFBits(BFManager &mgr, int const value, std::vector<BF> const &bits) {
    // Assume that we start with 0 in value;
    BF out = mgr.constantTrue();
    for (unsigned int i=0; i<bits.size(); i++) {
        if ((value & (1 << i))>0) {
            out &= bits[i];
        } else {
            out &= !bits[i];
        }

    }
    return out;
}

template<size_t size> class IntArrayHasher {
public:
    size_t operator()(const std::array<int,size> &in) const {
        size_t result = 0;
        for (const auto &element : in) {
            result ^= 0x195F36B9 + (element ^ (result << 5) ^ (result >> 2));
        }
        return result;
    }
};

template<size_t size> std::string intArrayPrinter(const std::array<int,size> &in) {
    std::ostringstream os;
    os << "(";
    bool first = true;
    for (auto it : in) {
        if (first) {
            first = false;
        } else {
            os << ",";
        }
        os << it;
    }
    os << ")";
    return os.str();
}

inline std::string printDoubleVector(const std::vector<double> &in) {
    std::ostringstream os;
    bool first=true;
    for (double a : in) {
        if (!first) os<<",";
        first = false;
        os << a;
    }
    return os.str();
}

inline int rotMin(int diff, int rotation) {
    return std::min(std::min(std::abs(diff),std::abs(diff-rotation)),std::abs(diff+rotation));
}

inline bool endsWith(std::string const & value, std::string const & ending)
{
    if (ending.size()>value.size()) return false;
    return value.substr(value.size()-ending.size())==ending;
}

#endif
