#include "Color.h"

namespace Color
{
    using byte = unsigned char;

    static inline byte PackFloat(float value)
    {
        return static_cast<byte>(glm::round(glm::clamp(value, 0.0f, 1.0f) * 255.0f));
    }

    static inline float UnpackFloat(byte value)
    {
        return value * inv255;
    }

    packed_type pack(const value_type& color)
    {
        byte r = PackFloat(color.r),
             g = PackFloat(color.g),
             b = PackFloat(color.b),
             a = PackFloat(color.a);
        return (r << 24) | (g << 16) | (b << 8) | a;
    }

    value_type unpack(packed_type color)
    {
        byte r = (color >> 24) & 255,
             g = (color >> 16) & 255,
             b = (color >>  8) & 255,
             a = (color >>  0) & 255;
        return value_type(UnpackFloat(r),
                          UnpackFloat(g),
                          UnpackFloat(b),
                          UnpackFloat(a));
    }

    #pragma region Pre-defined Colors

    #pragma region Pink Colors
    const value_type Pink             = fromRgb(255, 192, 203);
    const value_type LightPink        = fromRgb(255, 182, 193);
    const value_type HotPink          = fromRgb(255, 105, 180);
    const value_type DeepPink         = fromRgb(255, 20, 147);
    const value_type PaleVioletRed    = fromRgb(219, 112, 147);
    const value_type MediumVioletRed  = fromRgb(199, 21, 133);
    #pragma endregion

    #pragma region Red Colors
    const value_type LightSalmon      = fromRgb(255, 160, 122);
    const value_type Salmon           = fromRgb(250, 128, 114);
    const value_type DarkSalmon       = fromRgb(233, 150, 122);
    const value_type LightCoral       = fromRgb(240, 128, 128);
    const value_type IndianRed        = fromRgb(205, 92, 92);
    const value_type Crimson          = fromRgb(220, 20, 60);
    const value_type FireBrick        = fromRgb(178, 34, 34);
    const value_type DarkRed          = fromRgb(139, 0, 0);
    const value_type Red              = fromRgb(255, 0, 0);
    #pragma endregion

    #pragma region Orange Colors
    const value_type OrangeRed        = fromRgb(255, 69, 0);
    const value_type Tomato           = fromRgb(255, 99, 71);
    const value_type Coral            = fromRgb(255, 127, 80);
    const value_type DarkOrange       = fromRgb(255, 140, 0);
    const value_type Orange           = fromRgb(255, 165, 0);
    #pragma endregion

    #pragma region Yellow Colors
    const value_type Yellow           = fromRgb(255, 255, 0);
    const value_type LightYellow      = fromRgb(255, 255, 224);
    const value_type LemonChiffon     = fromRgb(255, 250, 205);
    const value_type LightGoldenrodYellow = fromRgb(250, 250, 210);
    const value_type PapayaWhip       = fromRgb(255, 239, 213);
    const value_type Moccasin         = fromRgb(255, 228, 181);
    const value_type PeachPuff        = fromRgb(255, 218, 185);
    const value_type PaleGoldenrod    = fromRgb(238, 232, 170);
    const value_type Khaki            = fromRgb(240, 230, 140);
    const value_type DarkKhaki        = fromRgb(189, 183, 107);
    const value_type Gold             = fromRgb(255, 215, 0);
    #pragma endregion

    #pragma region Brown Colors
    const value_type Cornsilk         = fromRgb(255, 248, 220);
    const value_type BlanchedAlmond   = fromRgb(255, 235, 205);
    const value_type Bisque           = fromRgb(255, 228, 196);
    const value_type NavajoWhite      = fromRgb(255, 222, 173);
    const value_type Wheat            = fromRgb(245, 222, 179);
    const value_type BurlyWood        = fromRgb(222, 184, 135);
    const value_type Tan              = fromRgb(210, 180, 140);
    const value_type RosyBrown        = fromRgb(188, 143, 143);
    const value_type SandyBrown       = fromRgb(244, 164, 96);
    const value_type Goldenrod        = fromRgb(218, 165, 32);
    const value_type DarkGoldenrod    = fromRgb(184, 134, 11);
    const value_type Peru             = fromRgb(205, 133, 63);
    const value_type Chocolate        = fromRgb(210, 105, 30);
    const value_type SaddleBrown      = fromRgb(139, 69, 19);
    const value_type Sienna           = fromRgb(160, 82, 45);
    const value_type Brown            = fromRgb(165, 42, 42);
    const value_type Maroon           = fromRgb(128, 0, 0);
    #pragma endregion

    #pragma region Green Colors
    const value_type DarkOliveGreen   = fromRgb(85, 107, 47);
    const value_type Olive            = fromRgb(128, 128, 0);
    const value_type OliveDrab        = fromRgb(107, 142, 35);
    const value_type YellowGreen      = fromRgb(154, 205, 50);
    const value_type LimeGreen        = fromRgb(50, 205, 50);
    const value_type Lime             = fromRgb(0, 255, 0);
    const value_type LawnGreen        = fromRgb(124, 252, 0);
    const value_type Chartreuse       = fromRgb(127, 255, 0);
    const value_type GreenYellow      = fromRgb(173, 255, 47);
    const value_type SpringGreen      = fromRgb(0, 255, 127);
    const value_type MediumSpringGreen = fromRgb(0, 250, 154);
    const value_type LightGreen       = fromRgb(144, 238, 144);
    const value_type PaleGreen        = fromRgb(152, 251, 152);
    const value_type DarkSeaGreen     = fromRgb(143, 188, 143);
    const value_type MediumAquamarine = fromRgb(102, 205, 170);
    const value_type MediumSeaGreen   = fromRgb(60, 179, 113);
    const value_type SeaGreen         = fromRgb(46, 139, 87);
    const value_type ForestGreen      = fromRgb(34, 139, 34);
    const value_type Green            = fromRgb(0, 128, 0);
    const value_type DarkGreen        = fromRgb(0, 100, 0);
    #pragma endregion

    #pragma region Cyan Colors
    const value_type Aqua             = fromRgb(0, 255, 255);
    const value_type Cyan             = fromRgb(0, 255, 255);
    const value_type LightCyan        = fromRgb(224, 255, 255);
    const value_type PaleTurquoise    = fromRgb(175, 238, 238);
    const value_type Aquamarine       = fromRgb(127, 255, 212);
    const value_type Turquoise        = fromRgb(64, 224, 208);
    const value_type MediumTurquoise  = fromRgb(72, 209, 204);
    const value_type DarkTurquoise    = fromRgb(0, 206, 209);
    const value_type LightSeaGreen    = fromRgb(32, 178, 170);
    const value_type CadetBlue        = fromRgb(95, 158, 160);
    const value_type DarkCyan         = fromRgb(0, 139, 139);
    const value_type Teal             = fromRgb(0, 128, 128);
    #pragma endregion

    #pragma region Blue Colors
    const value_type LightSteelBlue   = fromRgb(176, 196, 222);
    const value_type PowderBlue       = fromRgb(176, 224, 230);
    const value_type LightBlue        = fromRgb(173, 216, 230);
    const value_type SkyBlue          = fromRgb(135, 206, 235);
    const value_type LightSkyBlue     = fromRgb(135, 206, 250);
    const value_type DeepSkyBlue      = fromRgb(0, 191, 255);
    const value_type DodgerBlue       = fromRgb(30, 144, 255);
    const value_type CornflowerBlue   = fromRgb(100, 149, 237);
    const value_type SteelBlue        = fromRgb(70, 130, 180);
    const value_type RoyalBlue        = fromRgb(65, 105, 225);
    const value_type Blue             = fromRgb(0, 0, 255);
    const value_type MediumBlue       = fromRgb(0, 0, 205);
    const value_type DarkBlue         = fromRgb(0, 0, 139);
    const value_type Navy             = fromRgb(0, 0, 128);
    const value_type MidnightBlue     = fromRgb(25, 25, 112);
    #pragma endregion

    #pragma region Purple, Violet, and Magenta colors
    const value_type Lavender         = fromRgb(230, 230, 250);
    const value_type Thistle          = fromRgb(216, 191, 216);
    const value_type Plum             = fromRgb(221, 160, 221);
    const value_type Violet           = fromRgb(238, 130, 238);
    const value_type Orchid           = fromRgb(218, 112, 214);
    const value_type Fuchsia          = fromRgb(255, 0, 255);
    const value_type Magenta          = fromRgb(255, 0, 255);
    const value_type MediumOrchid     = fromRgb(186, 85, 211);
    const value_type MediumPurple     = fromRgb(147, 112, 219);
    const value_type BlueViolet       = fromRgb(138, 43, 226);
    const value_type DarkViolet       = fromRgb(148, 0, 211);
    const value_type DarkOrchid       = fromRgb(153, 50, 204);
    const value_type DarkMagenta      = fromRgb(139, 0, 139);
    const value_type Purple           = fromRgb(128, 0, 128);
    const value_type Indigo           = fromRgb(75, 0, 130);
    const value_type DarkSlateBlue    = fromRgb(72, 61, 139);
    const value_type SlateBlue        = fromRgb(106, 90, 205);
    const value_type MediumSlateBlue  = fromRgb(123, 104, 238);
    #pragma endregion

    #pragma region White Colors
    const value_type White            = fromRgb(255, 255, 255);
    const value_type Snow             = fromRgb(255, 250, 250);
    const value_type Honeydew         = fromRgb(240, 255, 240);
    const value_type MintCream        = fromRgb(245, 255, 250);
    const value_type Azure            = fromRgb(240, 255, 255);
    const value_type AliceBlue        = fromRgb(240, 248, 255);
    const value_type GhostWhite       = fromRgb(248, 248, 255);
    const value_type WhiteSmoke       = fromRgb(245, 245, 245);
    const value_type Seashell         = fromRgb(255, 245, 238);
    const value_type Beige            = fromRgb(245, 245, 220);
    const value_type OldLace          = fromRgb(253, 245, 230);
    const value_type FloralWhite      = fromRgb(255, 250, 240);
    const value_type Ivory            = fromRgb(255, 255, 240);
    const value_type AntiqueWhite     = fromRgb(250, 235, 215);
    const value_type Linen            = fromRgb(250, 240, 230);
    const value_type LavenderBlush    = fromRgb(255, 240, 245);
    const value_type MistyRose        = fromRgb(255, 228, 225);
    #pragma endregion

    #pragma region Gray And Black Colors
    const value_type Gainsboro        = fromRgb(220, 220, 220);
    const value_type LightGray        = fromRgb(211, 211, 211);
    const value_type Silver           = fromRgb(192, 192, 192);
    const value_type DarkGray         = fromRgb(169, 169, 169);
    const value_type Gray             = fromRgb(128, 128, 128);
    const value_type DimGray          = fromRgb(105, 105, 105);
    const value_type LightSlateGray   = fromRgb(119, 136, 153);
    const value_type SlateGray        = fromRgb(112, 128, 144);
    const value_type DarkSlateGray    = fromRgb(47, 79, 79);
    const value_type Black            = fromRgb(0, 0, 0);
    #pragma endregion

    const value_type TransparentBlack = fromRgba(0, 0, 0, 0);
    const value_type TransparentWhite = fromRgba(255, 255, 255, 0);

    #pragma endregion
}
