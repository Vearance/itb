#pragma once
#include <string>

enum class ColorGroup {
    BROWN,
    LIGHT_BLUE,
    PINK,
    ORANGE,
    RED,
    YELLOW,
    GREEN,
    DARK_BLUE,
    RAILROAD,
    UTILITY,
    NONE
};

inline std::string colorGroupToString(ColorGroup group) {
    switch (group) {
    case ColorGroup::BROWN:
        return "COKLAT";
    case ColorGroup::LIGHT_BLUE:
        return "BIRU_MUDA";
    case ColorGroup::PINK:
        return "MERAH_MUDA";
    case ColorGroup::ORANGE:
        return "ORANGE";
    case ColorGroup::RED:
        return "MERAH";
    case ColorGroup::YELLOW:
        return "KUNING";
    case ColorGroup::GREEN:
        return "HIJAU";
    case ColorGroup::DARK_BLUE:
        return "BIRU_TUA";
    case ColorGroup::RAILROAD:
        return "STASIUN";
    case ColorGroup::UTILITY:
        return "UTILITY";
    default:
        return "DEFAULT";
    }
}

inline ColorGroup colorGroupFromString(const std::string& s) {
    if (s == "COKLAT") {
        return ColorGroup::BROWN;
    }
    if (s == "BIRU_MUDA") {
        return ColorGroup::LIGHT_BLUE;
    }
    if (s == "MERAH_MUDA") {
        return ColorGroup::PINK;
    }
    if (s == "ORANGE") {
        return ColorGroup::ORANGE;
    }
    if (s == "MERAH") {
        return ColorGroup::RED;
    }
    if (s == "KUNING") {
        return ColorGroup::YELLOW;
    }
    if (s == "HIJAU") {
        return ColorGroup::GREEN;
    }
    if (s == "BIRU_TUA") {
        return ColorGroup::DARK_BLUE;
    }
    if (s == "DEFAULT") {
        return ColorGroup::NONE;
    }
    if (s == "ABU_ABU") {
        return ColorGroup::UTILITY;
    }
    return ColorGroup::NONE;
}
