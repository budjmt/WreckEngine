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

    packed_type Pack(const value_type& color)
    {
        byte r = PackFloat(color.r),
             g = PackFloat(color.g),
             b = PackFloat(color.b),
             a = PackFloat(color.a);
        return (r << 24) | (g << 16) | (b << 8) | a;
    }

    value_type Unpack(packed_type color)
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
    const value_type Pink             = FromRgb(255, 192, 203);
    const value_type LightPink        = FromRgb(255, 182, 193);
    const value_type HotPink          = FromRgb(255, 105, 180);
    const value_type DeepPink         = FromRgb(255, 20, 147);
    const value_type PaleVioletRed    = FromRgb(219, 112, 147);
    const value_type MediumVioletRed  = FromRgb(199, 21, 133);
    #pragma endregion

    #pragma region Red Colors
    const value_type LightSalmon      = FromRgb(255, 160, 122);
    const value_type Salmon           = FromRgb(250, 128, 114);
    const value_type DarkSalmon       = FromRgb(233, 150, 122);
    const value_type LightCoral       = FromRgb(240, 128, 128);
    const value_type IndianRed        = FromRgb(205, 92, 92);
    const value_type Crimson          = FromRgb(220, 20, 60);
    const value_type FireBrick        = FromRgb(178, 34, 34);
    const value_type DarkRed          = FromRgb(139, 0, 0);
    const value_type Red              = FromRgb(255, 0, 0);
    #pragma endregion

    #pragma region Orange Colors
    const value_type OrangeRed        = FromRgb(255, 69, 0);
    const value_type Tomato           = FromRgb(255, 99, 71);
    const value_type Coral            = FromRgb(255, 127, 80);
    const value_type DarkOrange       = FromRgb(255, 140, 0);
    const value_type Orange           = FromRgb(255, 165, 0);
    #pragma endregion

    #pragma region Yellow Colors
    const value_type Yellow           = FromRgb(255, 255, 0);
    const value_type LightYellow      = FromRgb(255, 255, 224);
    const value_type LemonChiffon     = FromRgb(255, 250, 205);
    const value_type LightGoldenrodYellow = FromRgb(250, 250, 210);
    const value_type PapayaWhip       = FromRgb(255, 239, 213);
    const value_type Moccasin         = FromRgb(255, 228, 181);
    const value_type PeachPuff        = FromRgb(255, 218, 185);
    const value_type PaleGoldenrod    = FromRgb(238, 232, 170);
    const value_type Khaki            = FromRgb(240, 230, 140);
    const value_type DarkKhaki        = FromRgb(189, 183, 107);
    const value_type Gold             = FromRgb(255, 215, 0);
    #pragma endregion

    #pragma region Brown Colors
    const value_type Cornsilk         = FromRgb(255, 248, 220);
    const value_type BlanchedAlmond   = FromRgb(255, 235, 205);
    const value_type Bisque           = FromRgb(255, 228, 196);
    const value_type NavajoWhite      = FromRgb(255, 222, 173);
    const value_type Wheat            = FromRgb(245, 222, 179);
    const value_type BurlyWood        = FromRgb(222, 184, 135);
    const value_type Tan              = FromRgb(210, 180, 140);
    const value_type RosyBrown        = FromRgb(188, 143, 143);
    const value_type SandyBrown       = FromRgb(244, 164, 96);
    const value_type Goldenrod        = FromRgb(218, 165, 32);
    const value_type DarkGoldenrod    = FromRgb(184, 134, 11);
    const value_type Peru             = FromRgb(205, 133, 63);
    const value_type Chocolate        = FromRgb(210, 105, 30);
    const value_type SaddleBrown      = FromRgb(139, 69, 19);
    const value_type Sienna           = FromRgb(160, 82, 45);
    const value_type Brown            = FromRgb(165, 42, 42);
    const value_type Maroon           = FromRgb(128, 0, 0);
    #pragma endregion

    #pragma region Green Colors
    const value_type DarkOliveGreen   = FromRgb(85, 107, 47);
    const value_type Olive            = FromRgb(128, 128, 0);
    const value_type OliveDrab        = FromRgb(107, 142, 35);
    const value_type YellowGreen      = FromRgb(154, 205, 50);
    const value_type LimeGreen        = FromRgb(50, 205, 50);
    const value_type Lime             = FromRgb(0, 255, 0);
    const value_type LawnGreen        = FromRgb(124, 252, 0);
    const value_type Chartreuse       = FromRgb(127, 255, 0);
    const value_type GreenYellow      = FromRgb(173, 255, 47);
    const value_type SpringGreen      = FromRgb(0, 255, 127);
    const value_type MediumSpringGreen = FromRgb(0, 250, 154);
    const value_type LightGreen       = FromRgb(144, 238, 144);
    const value_type PaleGreen        = FromRgb(152, 251, 152);
    const value_type DarkSeaGreen     = FromRgb(143, 188, 143);
    const value_type MediumAquamarine = FromRgb(102, 205, 170);
    const value_type MediumSeaGreen   = FromRgb(60, 179, 113);
    const value_type SeaGreen         = FromRgb(46, 139, 87);
    const value_type ForestGreen      = FromRgb(34, 139, 34);
    const value_type Green            = FromRgb(0, 128, 0);
    const value_type DarkGreen        = FromRgb(0, 100, 0);
    #pragma endregion

    #pragma region Cyan Colors
    const value_type Aqua             = FromRgb(0, 255, 255);
    const value_type Cyan             = FromRgb(0, 255, 255);
    const value_type LightCyan        = FromRgb(224, 255, 255);
    const value_type PaleTurquoise    = FromRgb(175, 238, 238);
    const value_type Aquamarine       = FromRgb(127, 255, 212);
    const value_type Turquoise        = FromRgb(64, 224, 208);
    const value_type MediumTurquoise  = FromRgb(72, 209, 204);
    const value_type DarkTurquoise    = FromRgb(0, 206, 209);
    const value_type LightSeaGreen    = FromRgb(32, 178, 170);
    const value_type CadetBlue        = FromRgb(95, 158, 160);
    const value_type DarkCyan         = FromRgb(0, 139, 139);
    const value_type Teal             = FromRgb(0, 128, 128);
    #pragma endregion

    #pragma region Blue Colors
    const value_type LightSteelBlue   = FromRgb(176, 196, 222);
    const value_type PowderBlue       = FromRgb(176, 224, 230);
    const value_type LightBlue        = FromRgb(173, 216, 230);
    const value_type SkyBlue          = FromRgb(135, 206, 235);
    const value_type LightSkyBlue     = FromRgb(135, 206, 250);
    const value_type DeepSkyBlue      = FromRgb(0, 191, 255);
    const value_type DodgerBlue       = FromRgb(30, 144, 255);
    const value_type CornflowerBlue   = FromRgb(100, 149, 237);
    const value_type SteelBlue        = FromRgb(70, 130, 180);
    const value_type RoyalBlue        = FromRgb(65, 105, 225);
    const value_type Blue             = FromRgb(0, 0, 255);
    const value_type MediumBlue       = FromRgb(0, 0, 205);
    const value_type DarkBlue         = FromRgb(0, 0, 139);
    const value_type Navy             = FromRgb(0, 0, 128);
    const value_type MidnightBlue     = FromRgb(25, 25, 112);
    #pragma endregion

    #pragma region Purple, Violet, and Magenta colors
    const value_type Lavender         = FromRgb(230, 230, 250);
    const value_type Thistle          = FromRgb(216, 191, 216);
    const value_type Plum             = FromRgb(221, 160, 221);
    const value_type Violet           = FromRgb(238, 130, 238);
    const value_type Orchid           = FromRgb(218, 112, 214);
    const value_type Fuchsia          = FromRgb(255, 0, 255);
    const value_type Magenta          = FromRgb(255, 0, 255);
    const value_type MediumOrchid     = FromRgb(186, 85, 211);
    const value_type MediumPurple     = FromRgb(147, 112, 219);
    const value_type BlueViolet       = FromRgb(138, 43, 226);
    const value_type DarkViolet       = FromRgb(148, 0, 211);
    const value_type DarkOrchid       = FromRgb(153, 50, 204);
    const value_type DarkMagenta      = FromRgb(139, 0, 139);
    const value_type Purple           = FromRgb(128, 0, 128);
    const value_type Indigo           = FromRgb(75, 0, 130);
    const value_type DarkSlateBlue    = FromRgb(72, 61, 139);
    const value_type SlateBlue        = FromRgb(106, 90, 205);
    const value_type MediumSlateBlue  = FromRgb(123, 104, 238);
    #pragma endregion

    #pragma region White Colors
    const value_type White            = FromRgb(255, 255, 255);
    const value_type Snow             = FromRgb(255, 250, 250);
    const value_type Honeydew         = FromRgb(240, 255, 240);
    const value_type MintCream        = FromRgb(245, 255, 250);
    const value_type Azure            = FromRgb(240, 255, 255);
    const value_type AliceBlue        = FromRgb(240, 248, 255);
    const value_type GhostWhite       = FromRgb(248, 248, 255);
    const value_type WhiteSmoke       = FromRgb(245, 245, 245);
    const value_type Seashell         = FromRgb(255, 245, 238);
    const value_type Beige            = FromRgb(245, 245, 220);
    const value_type OldLace          = FromRgb(253, 245, 230);
    const value_type FloralWhite      = FromRgb(255, 250, 240);
    const value_type Ivory            = FromRgb(255, 255, 240);
    const value_type AntiqueWhite     = FromRgb(250, 235, 215);
    const value_type Linen            = FromRgb(250, 240, 230);
    const value_type LavenderBlush    = FromRgb(255, 240, 245);
    const value_type MistyRose        = FromRgb(255, 228, 225);
    #pragma endregion

    #pragma region Gray And Black Colors
    const value_type Gainsboro        = FromRgb(220, 220, 220);
    const value_type LightGray        = FromRgb(211, 211, 211);
    const value_type Silver           = FromRgb(192, 192, 192);
    const value_type DarkGray         = FromRgb(169, 169, 169);
    const value_type Gray             = FromRgb(128, 128, 128);
    const value_type DimGray          = FromRgb(105, 105, 105);
    const value_type LightSlateGray   = FromRgb(119, 136, 153);
    const value_type SlateGray        = FromRgb(112, 128, 144);
    const value_type DarkSlateGray    = FromRgb(47, 79, 79);
    const value_type Black            = FromRgb(0, 0, 0);
    #pragma endregion

    const value_type TransparentBlack = FromRgba(0, 0, 0, 0);
    const value_type TransparentWhite = FromRgba(255, 255, 255, 0);

    #pragma endregion
}
