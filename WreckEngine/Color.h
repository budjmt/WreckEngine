#pragma once

#include <glm/glm.hpp>

namespace Color
{
    using value_type = glm::vec4;
    using packed_type = uint32_t;

    static constexpr float inv255 = 1.0f / 255.0f;

    inline value_type FromRgb(int r, int g, int b)
    {
        return value_type(r * inv255, g * inv255, b * inv255, 1.0f);
    }

    inline value_type FromRgba(int r, int g, int b, int a)
    {
        return value_type(r * inv255, g * inv255, b * inv255, a * inv255);
    }

    packed_type Pack(const value_type& color);

    value_type Unpack(packed_type color);

    #pragma region Pre-defined Colors

    // All pre-defined color information comes from Wikipedia

    #pragma region Pink Colors

    /// <summary>
    /// The color #FFC0CB.
    /// </summary>
    extern const value_type Pink;

    /// <summary>
    /// The color #FFB6C1.
    /// </summary>
    extern const value_type LightPink;

    /// <summary>
    /// The color #FF69B4.
    /// </summary>
    extern const value_type HotPink;

    /// <summary>
    /// The color #FF1493.
    /// </summary>
    extern const value_type DeepPink;

    /// <summary>
    /// The color #DB7093.
    /// </summary>
    extern const value_type PaleVioletRed;

    /// <summary>
    /// The color #C71585.
    /// </summary>
    extern const value_type MediumVioletRed;

    #pragma endregion

    #pragma region Red Colors

    /// <summary>
    /// The color #FFA07A.
    /// </summary>
    extern const value_type LightSalmon;

    /// <summary>
    /// The color #FA8072.
    /// </summary>
    extern const value_type Salmon;

    /// <summary>
    /// The color #E9967A.
    /// </summary>
    extern const value_type DarkSalmon;

    /// <summary>
    /// The color #F08080.
    /// </summary>
    extern const value_type LightCoral;

    /// <summary>
    /// The color #CD5C5C.
    /// </summary>
    extern const value_type IndianRed;

    /// <summary>
    /// The color #DC143C.
    /// </summary>
    extern const value_type Crimson;

    /// <summary>
    /// The color #B22222.
    /// </summary>
    extern const value_type FireBrick;

    /// <summary>
    /// The color #8B0000.
    /// </summary>
    extern const value_type DarkRed;

    /// <summary>
    /// The color #FF0000.
    /// </summary>
    extern const value_type Red;

    #pragma endregion

    #pragma region Orange Colors

    /// <summary>
    /// The color #FF4500.
    /// </summary>
    extern const value_type OrangeRed;

    /// <summary>
    /// The color #FF6347.
    /// </summary>
    extern const value_type Tomato;

    /// <summary>
    /// The color #FF7F50.
    /// </summary>
    extern const value_type Coral;

    /// <summary>
    /// The color #FF8C00.
    /// </summary>
    extern const value_type DarkOrange;

    /// <summary>
    /// The color #FFA500.
    /// </summary>
    extern const value_type Orange;

    #pragma endregion

    #pragma region Yellow Colors

    /// <summary>
    /// The color #FFFF00.
    /// </summary>
    extern const value_type Yellow;

    /// <summary>
    /// The color #FFFFE0.
    /// </summary>
    extern const value_type LightYellow;

    /// <summary>
    /// The color #FFFACD.
    /// </summary>
    extern const value_type LemonChiffon;

    /// <summary>
    /// The color #FAFAD2.
    /// </summary>
    extern const value_type LightGoldenrodYellow;

    /// <summary>
    /// The color #FFEFD5.
    /// </summary>
    extern const value_type PapayaWhip;

    /// <summary>
    /// The color #FFE4B5.
    /// </summary>
    extern const value_type Moccasin;

    /// <summary>
    /// The color #FFDAB9.
    /// </summary>
    extern const value_type PeachPuff;

    /// <summary>
    /// The color #EEE8AA.
    /// </summary>
    extern const value_type PaleGoldenrod;

    /// <summary>
    /// The color #F0E68C.
    /// </summary>
    extern const value_type Khaki;

    /// <summary>
    /// The color #BDB76B.
    /// </summary>
    extern const value_type DarkKhaki;

    /// <summary>
    /// The color #FFD700.
    /// </summary>
    extern const value_type Gold;

    #pragma endregion

    #pragma region Brown Colors

    /// <summary>
    /// The color #FFF8DC.
    /// </summary>
    extern const value_type Cornsilk;

    /// <summary>
    /// The color #FFEBCD.
    /// </summary>
    extern const value_type BlanchedAlmond;

    /// <summary>
    /// The color #FFE4C4.
    /// </summary>
    extern const value_type Bisque;

    /// <summary>
    /// The color #FFDEAD.
    /// </summary>
    extern const value_type NavajoWhite;

    /// <summary>
    /// The color #F5DEB3.
    /// </summary>
    extern const value_type Wheat;

    /// <summary>
    /// The color #DEB887.
    /// </summary>
    extern const value_type BurlyWood;

    /// <summary>
    /// The color #D2B48C.
    /// </summary>
    extern const value_type Tan;

    /// <summary>
    /// The color #BC8F8F.
    /// </summary>
    extern const value_type RosyBrown;

    /// <summary>
    /// The color #F4A460.
    /// </summary>
    extern const value_type SandyBrown;

    /// <summary>
    /// The color #DAA520.
    /// </summary>
    extern const value_type Goldenrod;

    /// <summary>
    /// The color #B8860B.
    /// </summary>
    extern const value_type DarkGoldenrod;

    /// <summary>
    /// The color #CD853F.
    /// </summary>
    extern const value_type Peru;

    /// <summary>
    /// The color #D2691E.
    /// </summary>
    extern const value_type Chocolate;

    /// <summary>
    /// The color #8B4513.
    /// </summary>
    extern const value_type SaddleBrown;

    /// <summary>
    /// The color #A0522D.
    /// </summary>
    extern const value_type Sienna;

    /// <summary>
    /// The color #A52A2A.
    /// </summary>
    extern const value_type Brown;

    /// <summary>
    /// The color #800000.
    /// </summary>
    extern const value_type Maroon;

    #pragma endregion

    #pragma region Green Colors

    /// <summary>
    /// The color #556B2F.
    /// </summary>
    extern const value_type DarkOliveGreen;

    /// <summary>
    /// The color #808000.
    /// </summary>
    extern const value_type Olive;

    /// <summary>
    /// The color #6B8E23.
    /// </summary>
    extern const value_type OliveDrab;

    /// <summary>
    /// The color #9ACD32.
    /// </summary>
    extern const value_type YellowGreen;

    /// <summary>
    /// The color #32CD32.
    /// </summary>
    extern const value_type LimeGreen;

    /// <summary>
    /// The color #00FF00.
    /// </summary>
    extern const value_type Lime;

    /// <summary>
    /// The color #7CFC00.
    /// </summary>
    extern const value_type LawnGreen;

    /// <summary>
    /// The color #7FFF00.
    /// </summary>
    extern const value_type Chartreuse;

    /// <summary>
    /// The color #ADFF2F.
    /// </summary>
    extern const value_type GreenYellow;

    /// <summary>
    /// The color #00FF7F.
    /// </summary>
    extern const value_type SpringGreen;

    /// <summary>
    /// The color #00FA9A.
    /// </summary>
    extern const value_type MediumSpringGreen;

    /// <summary>
    /// The color #90EE90.
    /// </summary>
    extern const value_type LightGreen;

    /// <summary>
    /// The color #98FB98.
    /// </summary>
    extern const value_type PaleGreen;

    /// <summary>
    /// The color #8FBC8F.
    /// </summary>
    extern const value_type DarkSeaGreen;

    /// <summary>
    /// The color #66CDAA.
    /// </summary>
    extern const value_type MediumAquamarine;

    /// <summary>
    /// The color #3CB371.
    /// </summary>
    extern const value_type MediumSeaGreen;

    /// <summary>
    /// The color #2E8B57.
    /// </summary>
    extern const value_type SeaGreen;

    /// <summary>
    /// The color #228B22.
    /// </summary>
    extern const value_type ForestGreen;

    /// <summary>
    /// The color #008000.
    /// </summary>
    extern const value_type Green;

    /// <summary>
    /// The color #006400.
    /// </summary>
    extern const value_type DarkGreen;

    #pragma endregion

    #pragma region Cyan Colors

    /// <summary>
    /// The color #00FFFF.
    /// </summary>
    extern const value_type Aqua;

    /// <summary>
    /// The color #00FFFF.
    /// </summary>
    extern const value_type Cyan;

    /// <summary>
    /// The color #E0FFFF.
    /// </summary>
    extern const value_type LightCyan;

    /// <summary>
    /// The color #AFEEEE.
    /// </summary>
    extern const value_type PaleTurquoise;

    /// <summary>
    /// The color #7FFFD4.
    /// </summary>
    extern const value_type Aquamarine;

    /// <summary>
    /// The color #40E0D0.
    /// </summary>
    extern const value_type Turquoise;

    /// <summary>
    /// The color #48D1CC.
    /// </summary>
    extern const value_type MediumTurquoise;

    /// <summary>
    /// The color #00CED1.
    /// </summary>
    extern const value_type DarkTurquoise;

    /// <summary>
    /// The color #20B2AA.
    /// </summary>
    extern const value_type LightSeaGreen;

    /// <summary>
    /// The color #5F9EA0.
    /// </summary>
    extern const value_type CadetBlue;

    /// <summary>
    /// The color #008B8B.
    /// </summary>
    extern const value_type DarkCyan;

    /// <summary>
    /// The color #008080.
    /// </summary>
    extern const value_type Teal;

    #pragma endregion

    #pragma region Blue Colors

    /// <summary>
    /// The color #B0C4DE.
    /// </summary>
    extern const value_type LightSteelBlue;

    /// <summary>
    /// The color #B0E0E6.
    /// </summary>
    extern const value_type PowderBlue;

    /// <summary>
    /// The color #ADD8E6.
    /// </summary>
    extern const value_type LightBlue;

    /// <summary>
    /// The color #87CEEB.
    /// </summary>
    extern const value_type SkyBlue;

    /// <summary>
    /// The color #87CEFA.
    /// </summary>
    extern const value_type LightSkyBlue;

    /// <summary>
    /// The color #00BFFF.
    /// </summary>
    extern const value_type DeepSkyBlue;

    /// <summary>
    /// The color #1E90FF.
    /// </summary>
    extern const value_type DodgerBlue;

    /// <summary>
    /// The color #6495ED.
    /// </summary>
    extern const value_type CornflowerBlue;

    /// <summary>
    /// The color #4682B4.
    /// </summary>
    extern const value_type SteelBlue;

    /// <summary>
    /// The color #4169E1.
    /// </summary>
    extern const value_type RoyalBlue;

    /// <summary>
    /// The color #0000FF.
    /// </summary>
    extern const value_type Blue;

    /// <summary>
    /// The color #0000CD.
    /// </summary>
    extern const value_type MediumBlue;

    /// <summary>
    /// The color #00008B.
    /// </summary>
    extern const value_type DarkBlue;

    /// <summary>
    /// The color #000080.
    /// </summary>
    extern const value_type Navy;

    /// <summary>
    /// The color #191970.
    /// </summary>
    extern const value_type MidnightBlue;

    #pragma endregion

    #pragma region Purple, Violet, and Magenta colors

    /// <summary>
    /// The color #E6E6FA.
    /// </summary>
    extern const value_type Lavender;

    /// <summary>
    /// The color #D8BFD8.
    /// </summary>
    extern const value_type Thistle;

    /// <summary>
    /// The color #DDA0DD.
    /// </summary>
    extern const value_type Plum;

    /// <summary>
    /// The color #EE82EE.
    /// </summary>
    extern const value_type Violet;

    /// <summary>
    /// The color #DA70D6.
    /// </summary>
    extern const value_type Orchid;

    /// <summary>
    /// The color #FF00FF.
    /// </summary>
    extern const value_type Fuchsia;

    /// <summary>
    /// The color #FF00FF.
    /// </summary>
    extern const value_type Magenta;

    /// <summary>
    /// The color #BA55D3.
    /// </summary>
    extern const value_type MediumOrchid;

    /// <summary>
    /// The color #9370DB.
    /// </summary>
    extern const value_type MediumPurple;

    /// <summary>
    /// The color #8A2BE2.
    /// </summary>
    extern const value_type BlueViolet;

    /// <summary>
    /// The color #9400D3.
    /// </summary>
    extern const value_type DarkViolet;

    /// <summary>
    /// The color #9932CC.
    /// </summary>
    extern const value_type DarkOrchid;

    /// <summary>
    /// The color #8B008B.
    /// </summary>
    extern const value_type DarkMagenta;

    /// <summary>
    /// The color #800080.
    /// </summary>
    extern const value_type Purple;

    /// <summary>
    /// The color #4B0082.
    /// </summary>
    extern const value_type Indigo;

    /// <summary>
    /// The color #483D8B.
    /// </summary>
    extern const value_type DarkSlateBlue;

    /// <summary>
    /// The color #6A5ACD.
    /// </summary>
    extern const value_type SlateBlue;

    /// <summary>
    /// The color #7B68EE.
    /// </summary>
    extern const value_type MediumSlateBlue;

    #pragma endregion

    #pragma region White Colors

    /// <summary>
    /// The color #FFFFFF.
    /// </summary>
    extern const value_type White;

    /// <summary>
    /// The color #FFFAFA.
    /// </summary>
    extern const value_type Snow;

    /// <summary>
    /// The color #F0FFF0.
    /// </summary>
    extern const value_type Honeydew;

    /// <summary>
    /// The color #F5FFFA.
    /// </summary>
    extern const value_type MintCream;

    /// <summary>
    /// The color #F0FFFF.
    /// </summary>
    extern const value_type Azure;

    /// <summary>
    /// The color #F0F8FF.
    /// </summary>
    extern const value_type AliceBlue;

    /// <summary>
    /// The color #F8F8FF.
    /// </summary>
    extern const value_type GhostWhite;

    /// <summary>
    /// The color #F5F5F5.
    /// </summary>
    extern const value_type WhiteSmoke;

    /// <summary>
    /// The color #FFF5EE.
    /// </summary>
    extern const value_type Seashell;

    /// <summary>
    /// The color #F5F5DC.
    /// </summary>
    extern const value_type Beige;

    /// <summary>
    /// The color #FDF5E6.
    /// </summary>
    extern const value_type OldLace;

    /// <summary>
    /// The color #FFFAF0.
    /// </summary>
    extern const value_type FloralWhite;

    /// <summary>
    /// The color #FFFFF0.
    /// </summary>
    extern const value_type Ivory;

    /// <summary>
    /// The color #FAEBD7.
    /// </summary>
    extern const value_type AntiqueWhite;

    /// <summary>
    /// The color #FAF0E6.
    /// </summary>
    extern const value_type Linen;

    /// <summary>
    /// The color #FFF0F5.
    /// </summary>
    extern const value_type LavenderBlush;

    /// <summary>
    /// The color #FFE4E1.
    /// </summary>
    extern const value_type MistyRose;

    #pragma endregion

    #pragma region Gray And Black Colors

    /// <summary>
    /// The color #DCDCDC.
    /// </summary>
    extern const value_type Gainsboro;

    /// <summary>
    /// The color #D3D3D3.
    /// </summary>
    extern const value_type LightGray;

    /// <summary>
    /// The color #C0C0C0.
    /// </summary>
    extern const value_type Silver;

    /// <summary>
    /// The color #A9A9A9.
    /// </summary>
    extern const value_type DarkGray;

    /// <summary>
    /// The color #808080.
    /// </summary>
    extern const value_type Gray;

    /// <summary>
    /// The color #696969.
    /// </summary>
    extern const value_type DimGray;

    /// <summary>
    /// The color #778899.
    /// </summary>
    extern const value_type LightSlateGray;

    /// <summary>
    /// The color #708090.
    /// </summary>
    extern const value_type SlateGray;

    /// <summary>
    /// The color #2F4F4F.
    /// </summary>
    extern const value_type DarkSlateGray;

    /// <summary>
    /// The color #000000.
    /// </summary>
    extern const value_type Black;

    #pragma endregion

    /// <summary>
    /// The color rgba(0, 0, 0, 0).
    /// </summary>
    extern const value_type TransparentBlack;

    /// <summary>
    /// The color rgba(255, 255, 255, 0).
    /// </summary>
    extern const value_type TransparentWhite;

    #pragma endregion
}
