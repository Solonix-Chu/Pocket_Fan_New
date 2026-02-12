/*
* SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
*
* SPDX-License-Identifier: MIT
*/
#pragma once
#include <cstdint>
#include "theme/types.h"
#include "localization/types.h"
#include "../hal/types.h"
#include "../hal/hal.h"

// LVGL Fonts
extern "C" const lv_font_t lv_font_Gully_Bold_16;
extern "C" const lv_font_t lv_font_Gully_Bold_12;
extern "C" const lv_font_t lv_font_lcdsolid_vpzb_16;
extern "C" const lv_font_t lv_font_lcdsolid_vpzb_12;
extern "C" const lv_font_t lv_font_AaPingPingGuoGuoXiangSuTi_16;
extern "C" const lv_font_t lv_font_AaPingPingGuoGuoXiangSuTi_12;
#if LV_FONT_SOURCE_HAN_SANS_SC_14_CJK
extern "C" const lv_font_t lv_font_source_han_sans_sc_14_cjk;
#endif

// LVGL Images (Menu App)
extern "C" const lv_image_dsc_t _About_RGB565A8_34x26;
extern "C" const lv_image_dsc_t _Detail_RGB565A8_34x26;
extern "C" const lv_image_dsc_t _Emoji_RGB565A8_34x26;
extern "C" const lv_image_dsc_t _Enjoy_RGB565A8_34x26;
extern "C" const lv_image_dsc_t _Health_RGB565A8_34x26;
extern "C" const lv_image_dsc_t _Quit_RGB565A8_34x26;
extern "C" const lv_image_dsc_t _Setting_RGB565A8_34x26;
extern "C" const lv_image_dsc_t _start_124X22_RGB565A8_124x22;
extern "C" const lv_image_dsc_t _HomePage_1_RGB565A8_128x64;
extern "C" const lv_image_dsc_t _HomePage_2_RGB565A8_128x64;

/**
 * @brief A struct to define static binary asset
 *
 */
struct StaticAsset_t
{
    // FontPool_t Font;
    // ImagePool_t Image;
    ColorPool_t Color;
    TextPool_t Text;
};

/**
 * @brief A Class to handle static asset
 *
 */
class AssetPool
{
    /* -------------------------------------------------------------------------- */
    /*                                  Singleton                                 */
    /* -------------------------------------------------------------------------- */
private:
    static AssetPool* _asset_pool;

public:
    static AssetPool* Get();

private:
    struct Data_t
    {
        StaticAsset_t* static_asset = nullptr;
        LocalTextPoolMap_t local_text_pool_map{};
        LocaleCode_t locale_code = locale_code_en;
    };
    Data_t _data;

    /* ------------------------------ Static asset ------------------------------ */
public:
    StaticAsset_t* getStaticAsset();
    bool injectStaticAsset(StaticAsset_t* pool);

    /* ---------------------------------- Text ---------------------------------- */
public:
    void setLocalTextTo(LocaleCode_t code);
    void setLocaleCode(LocaleCode_t code);
    LocaleCode_t getLocaleCode() { return _data.locale_code; }
    bool isLocaleEn() { return _data.locale_code == locale_code_en; }
    inline const LocalTextPoolMap_t& getText() { return _data.local_text_pool_map; }

    /* ---------------------------------- Font ---------------------------------- */
private:
    void _create_efont_pool();

public:
    // LVGL Font Access
    static const lv_font_t* GetGullyBold16() { return &lv_font_Gully_Bold_16; }
    static const lv_font_t* GetGullyBold12() { return &lv_font_Gully_Bold_12; }

    // Font families
    static const lv_font_t* GetLatinFontLarge() { return &lv_font_lcdsolid_vpzb_16; }
    static const lv_font_t* GetLatinFontSmall() { return &lv_font_lcdsolid_vpzb_12; }
    static const lv_font_t* GetChineseFontLarge() { return &lv_font_AaPingPingGuoGuoXiangSuTi_16; }
    static const lv_font_t* GetChineseFontSmall() { return &lv_font_AaPingPingGuoGuoXiangSuTi_12; }

    // Fonts that can render EN + CN in the same label:
    // - Latin/digits: lcdsolid_vpzb (primary)
    // - CJK: AaPingPingGuoGuoXiangSuTi (fallback)
    static const lv_font_t* GetLocaleFontLarge()
    {
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
        static lv_font_t s_font = lv_font_lcdsolid_vpzb_16;
        static bool s_inited = false;
        if (!s_inited) {
            s_font.fallback = &lv_font_AaPingPingGuoGuoXiangSuTi_16;
            s_inited = true;
        }
        return &s_font;
#else
        // LVGL without font fallback: choose by locale.
        return IsLocaleEn() ? GetLatinFontLarge() : GetChineseFontLarge();
#endif
    }

    static const lv_font_t* GetLocaleFontSmall()
    {
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
        static lv_font_t s_font = lv_font_lcdsolid_vpzb_12;
        static bool s_inited = false;
        if (!s_inited) {
            s_font.fallback = &lv_font_AaPingPingGuoGuoXiangSuTi_12;
            s_inited = true;
        }
        return &s_font;
#else
        // LVGL without font fallback: choose by locale.
        return IsLocaleEn() ? GetLatinFontSmall() : GetChineseFontSmall();
#endif
    }

    // LVGL Image Access (Menu App)
    static const lv_image_dsc_t* GetImgAbout() { return &_About_RGB565A8_34x26; }
    static const lv_image_dsc_t* GetImgDetail() { return &_Detail_RGB565A8_34x26; }
    static const lv_image_dsc_t* GetImgEmoji() { return &_Emoji_RGB565A8_34x26; }
    static const lv_image_dsc_t* GetImgEnjoy() { return &_Enjoy_RGB565A8_34x26; }
    static const lv_image_dsc_t* GetImgHealth() { return &_Health_RGB565A8_34x26; }
    static const lv_image_dsc_t* GetImgQuit() { return &_Quit_RGB565A8_34x26; }
    static const lv_image_dsc_t* GetImgSetting() { return &_Setting_RGB565A8_34x26; }
    static const lv_image_dsc_t* GetImgStartup() { return &_start_124X22_RGB565A8_124x22; }
    static const lv_image_dsc_t* GetImgHomePage1() { return &_HomePage_1_RGB565A8_128x64; }
    static const lv_image_dsc_t* GetImgHomePage2() { return &_HomePage_2_RGB565A8_128x64; }

    /* ------------------------------- Static wrap ------------------------------ */
public:
    static StaticAsset_t* GetStaticAsset() { return Get()->getStaticAsset(); }
    static bool InjectStaticAsset(StaticAsset_t* asset) { return Get()->injectStaticAsset(asset); }

    // static const ImagePool_t& GetImage() { return GetStaticAsset()->Image; }
    static const ColorPool_t& GetColor() { return GetStaticAsset()->Color; }
    static const TextPool_t& GetTextPool() { return GetStaticAsset()->Text; }
    static const LocalTextPoolMap_t& GetText() { return Get()->getText(); }

    static void SetLocaleCode(LocaleCode_t code) { Get()->setLocaleCode(code); }
    static void SetLocalTextTo(LocaleCode_t code) { Get()->setLocalTextTo(code); }
    static LocaleCode_t GetLocaleCode() { return Get()->getLocaleCode(); }
    static bool IsLocaleEn() { return Get()->isLocaleEn(); }

public:
#ifndef ESP_PLATFORM
    /* -------------------------- Generate static asset ------------------------- */
    static StaticAsset_t* CreateStaticAsset();
    static StaticAsset_t* GetStaticAssetFromBin();
#endif
};
