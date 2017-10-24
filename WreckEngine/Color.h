#pragma once

#include "MarchMath.h"

namespace Color {
    using value_type = vec4;
    using packed_type = uint32_t;

    static constexpr float inv255 = 1.0f / 255.0f;

    inline value_type fromRgb(int r, int g, int b) {
        return { r * inv255, g * inv255, b * inv255, 1.0f };
    }

    inline value_type fromRgba(int r, int g, int b, int a) {
        return { r * inv255, g * inv255, b * inv255, a * inv255 };
    }

    inline constexpr value_type fromRgb_c(int r, int g, int b) {
        return { vec3(r * inv255, g * inv255, b * inv255), 1.0f };
    }

    inline constexpr value_type fromRgba_c(int r, int g, int b, int a) {
        return { vec3(r * inv255, g * inv255, b * inv255), a * inv255 };
    }

    packed_type pack(const value_type& color);
    value_type unpack(packed_type color);

#pragma region Pink Colors
    constexpr value_type Pink            = fromRgb_c(255, 192, 203); // #FFC0CB
    constexpr value_type LightPink       = fromRgb_c(255, 182, 193); // #FFB6C1
    constexpr value_type HotPink         = fromRgb_c(255, 105, 180); // #FF69B4
    constexpr value_type DeepPink        = fromRgb_c(255, 20, 147);  // #FF1493
    constexpr value_type PaleVioletRed   = fromRgb_c(219, 112, 147); // #DB7093
    constexpr value_type MediumVioletRed = fromRgb_c(199, 21, 133);  // #C71585
#pragma endregion

#pragma region Red Colors
    constexpr value_type LightSalmon = fromRgb_c(255, 160, 122); // #FFA07A
    constexpr value_type Salmon      = fromRgb_c(250, 128, 114); // #FA8072
    constexpr value_type DarkSalmon  = fromRgb_c(233, 150, 122); // #E9967A
    constexpr value_type LightCoral  = fromRgb_c(240, 128, 128); // #F08080
    constexpr value_type IndianRed   = fromRgb_c(205, 92, 92);   // #CD5C5C
    constexpr value_type Crimson     = fromRgb_c(220, 20, 60);   // #DC143C
    constexpr value_type FireBrick   = fromRgb_c(178, 34, 34);   // #B22222
    constexpr value_type DarkRed     = fromRgb_c(139, 0, 0);     // #8B0000
    constexpr value_type Red         = fromRgb_c(255, 0, 0);     // #FF0000
#pragma endregion

#pragma region Orange Colors
    constexpr value_type OrangeRed  = fromRgb_c(255, 69, 0);   // #FF4500
    constexpr value_type Tomato     = fromRgb_c(255, 99, 71);  // #FF6347
    constexpr value_type Coral      = fromRgb_c(255, 127, 80); // #FF7F50
    constexpr value_type DarkOrange = fromRgb_c(255, 140, 0);  // #FF8C00
    constexpr value_type Orange     = fromRgb_c(255, 165, 0);  // #FFA500
#pragma endregion

#pragma region Yellow Colors
    constexpr value_type Yellow               = fromRgb_c(255, 255, 0);   // #FFFF00
    constexpr value_type LightYellow          = fromRgb_c(255, 255, 224); // #FFFFE0
    constexpr value_type LemonChiffon         = fromRgb_c(255, 250, 205); // #FFFACD
    constexpr value_type LightGoldenrodYellow = fromRgb_c(250, 250, 210); // #FAFAD2
    constexpr value_type PapayaWhip           = fromRgb_c(255, 239, 213); // #FFEFD5
    constexpr value_type Moccasin             = fromRgb_c(255, 228, 181); // #FFE4B5
    constexpr value_type PeachPuff            = fromRgb_c(255, 218, 185); // #FFDAB9
    constexpr value_type PaleGoldenrod        = fromRgb_c(238, 232, 170); // #EEE8AA
    constexpr value_type Khaki                = fromRgb_c(240, 230, 140); // #F0E68C
    constexpr value_type DarkKhaki            = fromRgb_c(189, 183, 107); // #BDB76B
    constexpr value_type Gold                 = fromRgb_c(255, 215, 0);   // #FFD700
#pragma endregion

#pragma region Brown Colors
    constexpr value_type Cornsilk       = fromRgb_c(255, 248, 220); // #FFF8DC
    constexpr value_type BlanchedAlmond = fromRgb_c(255, 235, 205); // #FFEBCD
    constexpr value_type Bisque         = fromRgb_c(255, 228, 196); // #FFE4C4
    constexpr value_type NavajoWhite    = fromRgb_c(255, 222, 173); // #FFDEAD
    constexpr value_type Wheat          = fromRgb_c(245, 222, 179); // #F5DEB3
    constexpr value_type BurlyWood      = fromRgb_c(222, 184, 135); // #DEB887
    constexpr value_type Tan            = fromRgb_c(210, 180, 140); // #D2B48C
    constexpr value_type RosyBrown      = fromRgb_c(188, 143, 143); // #BC8F8F
    constexpr value_type SandyBrown     = fromRgb_c(244, 164, 96);  // #F4A460
    constexpr value_type Goldenrod      = fromRgb_c(218, 165, 32);  // #DAA520
    constexpr value_type DarkGoldenrod  = fromRgb_c(184, 134, 11);  // #B8860B
    constexpr value_type Peru           = fromRgb_c(205, 133, 63);  // #CD853F
    constexpr value_type Chocolate      = fromRgb_c(210, 105, 30);  // #D2691E
    constexpr value_type SaddleBrown    = fromRgb_c(139, 69, 19);   // #8B4513
    constexpr value_type Sienna         = fromRgb_c(160, 82, 45);   // #A0522D
    constexpr value_type Brown          = fromRgb_c(165, 42, 42);   // #A52A2A
    constexpr value_type Maroon         = fromRgb_c(128, 0, 0);     // #800000
#pragma endregion

#pragma region Green Colors
    constexpr value_type DarkOliveGreen    = fromRgb_c(85, 107, 47);   // #556B2F
    constexpr value_type Olive             = fromRgb_c(128, 128, 0);   // #808000
    constexpr value_type OliveDrab         = fromRgb_c(107, 142, 35);  // #6B8E23
    constexpr value_type YellowGreen       = fromRgb_c(154, 205, 50);  // #9ACD32
    constexpr value_type LimeGreen         = fromRgb_c(50, 205, 50);   // #32CD32
    constexpr value_type Lime              = fromRgb_c(0, 255, 0);     // #00FF00
    constexpr value_type LawnGreen         = fromRgb_c(124, 252, 0);   // #7CFC00
    constexpr value_type Chartreuse        = fromRgb_c(127, 255, 0);   // #7FFF00
    constexpr value_type GreenYellow       = fromRgb_c(173, 255, 47);  // #ADFF2F
    constexpr value_type SpringGreen       = fromRgb_c(0, 255, 127);   // #00FF7F
    constexpr value_type MediumSpringGreen = fromRgb_c(0, 250, 154);   // #00FA9A
    constexpr value_type LightGreen        = fromRgb_c(144, 238, 144); // #90EE90
    constexpr value_type PaleGreen         = fromRgb_c(152, 251, 152); // #98FB98
    constexpr value_type DarkSeaGreen      = fromRgb_c(143, 188, 143); // #8FBC8F
    constexpr value_type MediumAquamarine  = fromRgb_c(102, 205, 170); // #66CDAA
    constexpr value_type MediumSeaGreen    = fromRgb_c(60, 179, 113);  // #3CB371
    constexpr value_type SeaGreen          = fromRgb_c(46, 139, 87);   // #2E8B57
    constexpr value_type ForestGreen       = fromRgb_c(34, 139, 34);   // #228B22
    constexpr value_type Green             = fromRgb_c(0, 128, 0);     // #008000
    constexpr value_type DarkGreen         = fromRgb_c(0, 100, 0);     // #006400
#pragma endregion

#pragma region Cyan Colors
    constexpr value_type Aqua            = fromRgb_c(0, 255, 255);   // #00FFFF
    constexpr value_type Cyan            = fromRgb_c(0, 255, 255);   // #00FFFF
    constexpr value_type LightCyan       = fromRgb_c(224, 255, 255); // #E0FFFF
    constexpr value_type PaleTurquoise   = fromRgb_c(175, 238, 238); // #AFEEEE
    constexpr value_type Aquamarine      = fromRgb_c(127, 255, 212); // #7FFFD4
    constexpr value_type Turquoise       = fromRgb_c(64, 224, 208);  // #40E0D0
    constexpr value_type MediumTurquoise = fromRgb_c(72, 209, 204);  // #48D1CC
    constexpr value_type DarkTurquoise   = fromRgb_c(0, 206, 209);   // #00CED1
    constexpr value_type LightSeaGreen   = fromRgb_c(32, 178, 170);  // #20B2AA
    constexpr value_type CadetBlue       = fromRgb_c(95, 158, 160);  // #5F9EA0
    constexpr value_type DarkCyan        = fromRgb_c(0, 139, 139);   // #008B8B
    constexpr value_type Teal            = fromRgb_c(0, 128, 128);   // #008080
#pragma endregion

#pragma region Blue Colors
    constexpr value_type LightSteelBlue = fromRgb_c(176, 196, 222); // #B0C4DE
    constexpr value_type PowderBlue     = fromRgb_c(176, 224, 230); // #B0E0E6
    constexpr value_type LightBlue      = fromRgb_c(173, 216, 230); // #ADD8E6
    constexpr value_type SkyBlue        = fromRgb_c(135, 206, 235); // #87CEEB
    constexpr value_type LightSkyBlue   = fromRgb_c(135, 206, 250); // #87CEFA
    constexpr value_type DeepSkyBlue    = fromRgb_c(0, 191, 255);   // #00BFFF
    constexpr value_type DodgerBlue     = fromRgb_c(30, 144, 255);  // #1E90FF
    constexpr value_type CornflowerBlue = fromRgb_c(100, 149, 237); // #6495ED
    constexpr value_type SteelBlue      = fromRgb_c(70, 130, 180);  // #4682B4
    constexpr value_type RoyalBlue      = fromRgb_c(65, 105, 225);  // #4169E1
    constexpr value_type Blue           = fromRgb_c(0, 0, 255);     // #0000FF
    constexpr value_type MediumBlue     = fromRgb_c(0, 0, 205);     // #0000CD
    constexpr value_type DarkBlue       = fromRgb_c(0, 0, 139);     // #00008B
    constexpr value_type Navy           = fromRgb_c(0, 0, 128);     // #000080
    constexpr value_type MidnightBlue   = fromRgb_c(25, 25, 112);   // #191970
#pragma endregion

#pragma region Purple, Violet, and Magenta colors
    constexpr value_type Lavender        = fromRgb_c(230, 230, 250); // #E6E6FA
    constexpr value_type Thistle         = fromRgb_c(216, 191, 216); // #D8BFD8
    constexpr value_type Plum            = fromRgb_c(221, 160, 221); // #DDA0DD
    constexpr value_type Violet          = fromRgb_c(238, 130, 238); // #EE82EE
    constexpr value_type Orchid          = fromRgb_c(218, 112, 214); // #DA70D6
    constexpr value_type Fuchsia         = fromRgb_c(255, 0, 255);   // #FF00FF
    constexpr value_type Magenta         = fromRgb_c(255, 0, 255);   // #FF00FF
    constexpr value_type MediumOrchid    = fromRgb_c(186, 85, 211);  // #BA55D3
    constexpr value_type MediumPurple    = fromRgb_c(147, 112, 219); // #9370DB
    constexpr value_type BlueViolet      = fromRgb_c(138, 43, 226);  // #8A2BE2
    constexpr value_type DarkViolet      = fromRgb_c(148, 0, 211);   // #9400D3
    constexpr value_type DarkOrchid      = fromRgb_c(153, 50, 204);  // #9932CC
    constexpr value_type DarkMagenta     = fromRgb_c(139, 0, 139);   // #8B008B
    constexpr value_type Purple          = fromRgb_c(128, 0, 128);   // #800080
    constexpr value_type Indigo          = fromRgb_c(75, 0, 130);    // #4B0082
    constexpr value_type DarkSlateBlue   = fromRgb_c(72, 61, 139);   // #483D8B
    constexpr value_type SlateBlue       = fromRgb_c(106, 90, 205);  // #6A5ACD
    constexpr value_type MediumSlateBlue = fromRgb_c(123, 104, 238); // #7B68EE
#pragma endregion

#pragma region White Colors
    constexpr value_type White         = fromRgb_c(255, 255, 255); // #FFFFFF
    constexpr value_type Snow          = fromRgb_c(255, 250, 250); // #FFFAFA
    constexpr value_type Honeydew      = fromRgb_c(240, 255, 240); // #F0FFF0
    constexpr value_type MintCream     = fromRgb_c(245, 255, 250); // #F5FFFA
    constexpr value_type Azure         = fromRgb_c(240, 255, 255); // #F0FFFF
    constexpr value_type AliceBlue     = fromRgb_c(240, 248, 255); // #F0F8FF
    constexpr value_type GhostWhite    = fromRgb_c(248, 248, 255); // #F8F8FF
    constexpr value_type WhiteSmoke    = fromRgb_c(245, 245, 245); // #F5F5F5
    constexpr value_type Seashell      = fromRgb_c(255, 245, 238); // #FFF5EE
    constexpr value_type Beige         = fromRgb_c(245, 245, 220); // #F5F5DC
    constexpr value_type OldLace       = fromRgb_c(253, 245, 230); // #FDF5E6
    constexpr value_type FloralWhite   = fromRgb_c(255, 250, 240); // #FFFAF0
    constexpr value_type Ivory         = fromRgb_c(255, 255, 240); // #FFFFF0
    constexpr value_type AntiqueWhite  = fromRgb_c(250, 235, 215); // #FAEBD7
    constexpr value_type Linen         = fromRgb_c(250, 240, 230); // #FAF0E6
    constexpr value_type LavenderBlush = fromRgb_c(255, 240, 245); // #FFF0F5
    constexpr value_type MistyRose     = fromRgb_c(255, 228, 225); // #FFE4E1
#pragma endregion

#pragma region Gray And Black Colors
    constexpr value_type Gainsboro      = fromRgb_c(220, 220, 220); // #DCDCDC
    constexpr value_type LightGray      = fromRgb_c(211, 211, 211); // #D3D3D3
    constexpr value_type Silver         = fromRgb_c(192, 192, 192); // #C0C0C0
    constexpr value_type DarkGray       = fromRgb_c(169, 169, 169); // #A9A9A9
    constexpr value_type Gray           = fromRgb_c(128, 128, 128); // #808080
    constexpr value_type DimGray        = fromRgb_c(105, 105, 105); // #696969
    constexpr value_type LightSlateGray = fromRgb_c(119, 136, 153); // #778899
    constexpr value_type SlateGray      = fromRgb_c(112, 128, 144); // #708090
    constexpr value_type DarkSlateGray  = fromRgb_c(47, 79, 79);    // #2F4F4F
    constexpr value_type Black          = fromRgb_c(0, 0, 0);       // #000000
#pragma endregion

    constexpr value_type TransparentBlack = fromRgba_c(0, 0, 0, 0);
    constexpr value_type TransparentWhite = fromRgba_c(255, 255, 255, 0);

#pragma endregion
}
