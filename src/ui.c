#include "global.h"
#include "modding.h"
#include "recomputils.h"
#include "recompui.h"
#include "modelmatrixids.h"

static void refreshFileList();

typedef struct {
    const char *displayName;
    bool isVisible;
    bool isUsedByCurrentGame;
    Link_CustomModelCategory category;
    bool isNeedsDiskSave;
} CategoryInfo;

static bool sIsForceAllCategoriesVisible = false;

#define DECLARE_DEFAULT_CAT_INFO(name, isUsed, cat, isVisibleByDefault) {.displayName = name, .isVisible = isVisibleByDefault, .isUsedByCurrentGame = isUsed, .category = cat, .isNeedsDiskSave = false}
#define DECLARE_CAT_INFO(name, isUsed, cat) DECLARE_DEFAULT_CAT_INFO(name, isUsed, cat, false)

#define CAT_USED_MM true

// Always opposite of MM
#define CAT_USED_OOT (!CAT_USED_MM)

// used by both games
#define CAT_USED_Z64 true

static CategoryInfo sCategoryInfos[] = {
    // At least one category must be visible or the category selector goes into an infinite loop
    DECLARE_CAT_INFO("Model Packs", CAT_USED_Z64, LINK_CMC_MODEL_PACK),
    DECLARE_DEFAULT_CAT_INFO("Young Link", CAT_USED_OOT, LINK_CMC_CHILD, CAT_USED_OOT),
    DECLARE_DEFAULT_CAT_INFO("Adult Link", CAT_USED_OOT, LINK_CMC_ADULT, CAT_USED_OOT),
    DECLARE_DEFAULT_CAT_INFO("Human", CAT_USED_MM, LINK_CMC_HUMAN, CAT_USED_MM),
    DECLARE_DEFAULT_CAT_INFO("Deku", CAT_USED_MM, LINK_CMC_DEKU, CAT_USED_MM),
    DECLARE_DEFAULT_CAT_INFO("Goron", CAT_USED_MM, LINK_CMC_GORON, CAT_USED_MM),
    DECLARE_DEFAULT_CAT_INFO("Zora", CAT_USED_MM, LINK_CMC_ZORA, CAT_USED_MM),
    DECLARE_DEFAULT_CAT_INFO("Fierce Deity", CAT_USED_MM, LINK_CMC_FIERCE_DEITY, CAT_USED_MM),
    DECLARE_CAT_INFO("Kokiri Sword", CAT_USED_Z64, LINK_CMC_SWORD1),
    DECLARE_CAT_INFO("Razor Sword", CAT_USED_MM, LINK_CMC_SWORD2),
    DECLARE_CAT_INFO("Gilded Sword", CAT_USED_Z64, LINK_CMC_SWORD3),
    DECLARE_CAT_INFO("Fierce Deity's Sword", CAT_USED_Z64, LINK_CMC_SWORD4),
    DECLARE_CAT_INFO("Great Fairy's Sword", CAT_USED_MM, LINK_CMC_SWORD5),
    DECLARE_CAT_INFO("Deku Shield", CAT_USED_OOT, LINK_CMC_SHIELD1),
    DECLARE_CAT_INFO("Hero's Shield", CAT_USED_Z64, LINK_CMC_SHIELD2),
    DECLARE_CAT_INFO("Mirror Shield", CAT_USED_Z64, LINK_CMC_SHIELD3),
    DECLARE_CAT_INFO("Hookshot", CAT_USED_Z64, LINK_CMC_HOOKSHOT),
    DECLARE_CAT_INFO("Bow", CAT_USED_Z64, LINK_CMC_BOW),
    DECLARE_CAT_INFO("Slingshot", CAT_USED_OOT, LINK_CMC_SLINGSHOT),
    DECLARE_CAT_INFO("Bottle", CAT_USED_Z64, LINK_CMC_BOTTLE),
    DECLARE_CAT_INFO("Fairy Ocarina", CAT_USED_OOT, LINK_CMC_OCARINA_FAIRY),
    DECLARE_CAT_INFO("Ocarina of Time", CAT_USED_Z64, LINK_CMC_OCARINA_TIME),
    DECLARE_CAT_INFO("Boomerang", CAT_USED_OOT, LINK_CMC_BOOMERANG),
    DECLARE_CAT_INFO("Hammer", CAT_USED_OOT, LINK_CMC_HAMMER),
    DECLARE_CAT_INFO("Deku Stick", CAT_USED_Z64, LINK_CMC_DEKU_STICK),
    DECLARE_CAT_INFO("Deku Pipes", CAT_USED_MM, LINK_CMC_PIPES),
    DECLARE_CAT_INFO("Goron Drums", CAT_USED_MM, LINK_CMC_DRUMS),
    DECLARE_CAT_INFO("Zora Guitar", CAT_USED_MM, LINK_CMC_GUITAR),
    DECLARE_CAT_INFO("Skull Mask", CAT_USED_OOT, LINK_CMC_MASK_SKULL),
    DECLARE_CAT_INFO("Spooky Mask", CAT_USED_OOT, LINK_CMC_MASK_SPOOKY),
    DECLARE_CAT_INFO("Gerudo Mask", CAT_USED_OOT, LINK_CMC_MASK_GERUDO),
    DECLARE_CAT_INFO("Mask of Truth", CAT_USED_Z64, LINK_CMC_MASK_TRUTH),
    DECLARE_CAT_INFO("Kafei's Mask", CAT_USED_MM, LINK_CMC_MASK_KAFEIS_MASK),
    DECLARE_CAT_INFO("All Night Mask", CAT_USED_MM, LINK_CMC_MASK_ALL_NIGHT),
    DECLARE_CAT_INFO("Bunny Hood", CAT_USED_Z64, LINK_CMC_MASK_BUNNY),
    DECLARE_CAT_INFO("Keaton Mask", CAT_USED_Z64, LINK_CMC_MASK_KEATON),
    DECLARE_CAT_INFO("Garo Mask", CAT_USED_MM, LINK_CMC_MASK_GARO),
    DECLARE_CAT_INFO("Romani Mask", CAT_USED_MM, LINK_CMC_MASK_ROMANI),
    DECLARE_CAT_INFO("Circus Leader's Mask", CAT_USED_MM, LINK_CMC_MASK_CIRCUS_LEADER),
    DECLARE_CAT_INFO("Couple's Mask", CAT_USED_MM, LINK_CMC_MASK_COUPLE),
    DECLARE_CAT_INFO("Postman's Hat", CAT_USED_MM, LINK_CMC_MASK_POSTMAN),
    DECLARE_CAT_INFO("Great Fairy Mask", CAT_USED_MM, LINK_CMC_MASK_GREAT_FAIRY),
    DECLARE_CAT_INFO("Gibdo Mask", CAT_USED_MM, LINK_CMC_MASK_GIBDO),
    DECLARE_CAT_INFO("Don Gero's Mask", CAT_USED_MM, LINK_CMC_MASK_DON_GERO),
    DECLARE_CAT_INFO("Kamaro Mask", CAT_USED_MM, LINK_CMC_MASK_KAMARO),
    DECLARE_CAT_INFO("Captain's Hat", CAT_USED_MM, LINK_CMC_MASK_CAPTAIN),
    DECLARE_CAT_INFO("Stone Mask", CAT_USED_MM, LINK_CMC_MASK_STONE),
    DECLARE_CAT_INFO("Bremen Mask", CAT_USED_MM, LINK_CMC_MASK_BREMEN),
    DECLARE_CAT_INFO("Blast Mask", CAT_USED_MM, LINK_CMC_MASK_BLAST),
    DECLARE_CAT_INFO("Mask of Scents", CAT_USED_MM, LINK_CMC_MASK_SCENTS),
    DECLARE_CAT_INFO("Giant's Mask", CAT_USED_MM, LINK_CMC_MASK_GIANT),
    DECLARE_CAT_INFO("Deku Mask", CAT_USED_MM, LINK_CMC_MASK_DEKU),
    DECLARE_CAT_INFO("Goron Mask", CAT_USED_Z64, LINK_CMC_MASK_GORON),
    DECLARE_CAT_INFO("Zora Mask", CAT_USED_Z64, LINK_CMC_MASK_ZORA),
    DECLARE_CAT_INFO("Fierce Deity Mask", CAT_USED_MM, LINK_CMC_MASK_FIERCE_DEITY),
};

#define SELECTING_CATEGORY -99

static int sCurrentCategoryInfo = SELECTING_CATEGORY;

static bool isValidCategoryInfoIndex(int i) {
    return i >= 0 && i < ARRAY_COUNT(sCategoryInfos);
}

static bool isSelectingCategory() {
    return sCurrentCategoryInfo == SELECTING_CATEGORY;
}

static bool isSelectingModel() {
    return isValidCategoryInfoIndex(sCurrentCategoryInfo);
}

static CategoryInfo *getCurrentCategoryInfo() {
    if (isSelectingModel()) {
        return &sCategoryInfos[sCurrentCategoryInfo];
    }

    return NULL;
}

typedef struct {
    RecompuiColor borderColor;
    RecompuiColor bgColor;
} ButtonColor;

static const ButtonColor sModelRemovedButtonColor = {
    .borderColor = {
        .r = 224,
        .g = 74,
        .b = 89,
        .a = 204,
    },
    .bgColor = {
        .r = 224,
        .g = 74,
        .b = 89,
        .a = 13,
    },
};

static RecompuiResource createListBoxButton(RecompuiContext context, RecompuiResource parent, const char *text, RecompuiButtonStyle style) {
    RecompuiResource button = recompui_create_button(context, parent, text, style);
    recompui_set_flex_shrink(button, 0);
    recompui_set_width(button, 100.0f, UNIT_PERCENT);
    recompui_set_text_align(button, TEXT_ALIGN_CENTER);

    return button;
}

// Connect a list of buttons vertically with the first entry at the top
//
// Left and right inputs will navigate to the top and bottom of the list,
// respectively
static void connectListBoxButtons(const RecompuiResource buttons[], size_t n) {
    if (n < 2) {
        return;
    }

    RecompuiResource first = buttons[0];
    RecompuiResource last = buttons[n - 1];

    for (size_t i = 1; i < n - 1; ++i) {
        RecompuiResource curr = buttons[i];
        RecompuiResource next = buttons[i + 1];

        recompui_set_nav(buttons[i], NAVDIRECTION_DOWN, next);
        recompui_set_nav(buttons[i], NAVDIRECTION_LEFT, first);
        recompui_set_nav(buttons[i], NAVDIRECTION_RIGHT, last);

        recompui_set_nav(next, NAVDIRECTION_UP, curr);
    }

    recompui_set_nav(first, NAVDIRECTION_DOWN, buttons[1]);
    recompui_set_nav(first, NAVDIRECTION_RIGHT, last);

    recompui_set_nav(last, NAVDIRECTION_UP, buttons[n - 2]);
    recompui_set_nav(last, NAVDIRECTION_LEFT, first);
}

RecompuiResource sListButtons[1024];
size_t sListButtonsSize;

static const RecompuiResource *getCurrentButtonArray() {
    return sListButtons;
}

static size_t getCurrentButtonArraySize() {
    return sListButtonsSize;
}

static void pushButtonToList(RecompuiResource button) {
    if (sListButtonsSize < ARRAY_COUNT(sListButtons)) {
        sListButtons[sListButtonsSize] = button;
        sListButtonsSize++;
    }
}

static void clearButtonList() {
    sListButtonsSize = 0;
}

RecompuiResource createAndPushButtonToList(RecompuiContext context, RecompuiResource parent, const char *text, RecompuiButtonStyle style) {
    RecompuiResource button = createListBoxButton(context, parent, text, style);
    pushButtonToList(button);
    return button;
}

static void setButtonColor(RecompuiResource button, const ButtonColor *color) {
    recompui_set_background_color(button, &color->bgColor);
    recompui_set_border_color(button, &color->borderColor);
}

static RecompuiContext sUIContext;
static RecompuiResource sUIRoot;
static RecompuiResource sContainerMain;
static RecompuiResource sRowTop;
static RecompuiResource sRowCategory;
static RecompuiResource sButtonCategoryNext;
static RecompuiResource sButtonCategoryPrev;
static RecompuiResource sLabelCategory;
static RecompuiResource sButtonClose;
static RecompuiResource sButtonUpOneMenuLevel;

static bool sIsUIContextShown = false;

RecompuiResource sContainerListButtons;
RecompuiResource sLabelAuthorPrefix;
RecompuiResource sLabelAuthor;

static bool sIsLivePreviewEnabled = false;

static void removeAllModelsButtonPressed(RecompuiResource resource, const RecompuiEventData *data, void *userdata) {
    if (sIsUIContextShown) {
        if (data->type == UI_EVENT_CLICK) {
            if (isSelectingCategory()) {
            }
        }
    }
}

static void closeButtonPressed(RecompuiResource resource, const RecompuiEventData *data, void *userdata) {
    if (sIsUIContextShown) {
        if (data->type == UI_EVENT_CLICK) {
            recompui_hide_context(sUIContext);
            sIsUIContextShown = false;
        } else if (data->type == UI_EVENT_FOCUS || data->type == UI_EVENT_HOVER) {
        }
    }
}

static void destroyModelButtons();

void refreshCategoryName() {
    if (sLabelCategory) {
        recompui_destroy_element(sRowCategory, sLabelCategory);
        sLabelCategory = 0;
    }

    if (isSelectingCategory()) {
        sLabelCategory = recompui_create_label(sUIContext, sRowCategory, "Categories", LABELSTYLE_LARGE);

    } else if (isSelectingModel()) {
        CategoryInfo *catInf = getCurrentCategoryInfo();

        if (catInf) {
            sLabelCategory = recompui_create_label(sUIContext, sRowCategory, catInf->displayName, LABELSTYLE_LARGE);
        }
    }
}

static void incrementCurrentCategory() {
    CategoryInfo *currInf;

    do {
        sCurrentCategoryInfo = (sCurrentCategoryInfo + 1) % ARRAY_COUNT(sCategoryInfos);

        currInf = &sCategoryInfos[sCurrentCategoryInfo];
    } while (!(currInf->isVisible || sIsForceAllCategoriesVisible));
}

static void decrementCurrentCategory() {
    CategoryInfo *currInf;

    do {
        sCurrentCategoryInfo = (sCurrentCategoryInfo - 1 + ARRAY_COUNT(sCategoryInfos)) % ARRAY_COUNT(sCategoryInfos);

        currInf = &sCategoryInfos[sCurrentCategoryInfo];
    } while (!(currInf->isVisible || sIsForceAllCategoriesVisible));
}

static void changeCategoryButtonPressed(RecompuiResource resource, const RecompuiEventData *data, void *userdata) {
    if (sIsUIContextShown) {
        if (data->type == UI_EVENT_CLICK) {

            bool isNextButton = !!userdata;

            if (isNextButton) {
                incrementCurrentCategory();
            } else {
                decrementCurrentCategory();
            }

            refreshFileList();
        } else if (data->type == UI_EVENT_FOCUS || data->type == UI_EVENT_HOVER) {
        }
    }
}

static void destroyNextPrevCategoryButtons() {
    if (sButtonCategoryNext) {
        recompui_destroy_element(sRowCategory, sButtonCategoryNext);
        sButtonCategoryNext = 0;
    }

    if (sButtonCategoryPrev) {
        recompui_destroy_element(sRowCategory, sButtonCategoryPrev);
        sButtonCategoryPrev = 0;
    }
}

static void createNextPrevCategoryButtons() {
    destroyNextPrevCategoryButtons();

    sButtonCategoryPrev = recompui_create_button(sUIContext, sRowCategory, "◀", BUTTONSTYLE_SECONDARY);
    sButtonCategoryNext = recompui_create_button(sUIContext, sRowCategory, "▶", BUTTONSTYLE_SECONDARY);

    recompui_register_callback(sButtonCategoryPrev, changeCategoryButtonPressed, NULL);
    recompui_register_callback(sButtonCategoryNext, changeCategoryButtonPressed, (void *)1);
}

RECOMP_CALLBACK("*", recomp_on_init)
void initUIOnRecompInit() {
    RecompuiColor bg_color;
    bg_color.r = 255;
    bg_color.g = 255;
    bg_color.b = 255;
    bg_color.a = 0.1f * 255;

    RecompuiColor border_color;
    border_color.r = 255;
    border_color.g = 255;
    border_color.b = 255;
    border_color.a = 0.2f * 255;

    RecompuiColor modal_color;
    modal_color.r = 8;
    modal_color.g = 7;
    modal_color.b = 13;
    modal_color.a = 0.9f * 255;

    const float body_padding = 64.0f;
    const float modal_height = RECOMPUI_TOTAL_HEIGHT - (2 * body_padding);
    const float modal_max_width = modal_height * (16.0f / 9.0f);
    const float modal_border_width = 1.1f;
    const float modal_border_radius = 16.0f;

    sUIContext = recompui_create_context();
    recompui_open_context(sUIContext);

    sUIRoot = recompui_context_root(sUIContext);
    // Set up the sUIRoot element so it takes up the full screen.
    recompui_set_position(sUIRoot, POSITION_ABSOLUTE);
    recompui_set_top(sUIRoot, 0, UNIT_DP);
    recompui_set_right(sUIRoot, 0, UNIT_DP);
    recompui_set_bottom(sUIRoot, 0, UNIT_DP);
    recompui_set_left(sUIRoot, 0, UNIT_DP);
    recompui_set_width_auto(sUIRoot);
    recompui_set_height_auto(sUIRoot);

    // Set up the sUIRoot element's padding so the modal contents don't touch the screen edges.
    recompui_set_padding(sUIRoot, body_padding, UNIT_DP);
    // recompui_set_background_color(sUIRoot, &bg_color);

    // Set up the flexbox properties of the sUIRoot element.
    recompui_set_flex_direction(sUIRoot, FLEX_DIRECTION_COLUMN);
    recompui_set_justify_content(sUIRoot, JUSTIFY_CONTENT_FLEX_START);
    recompui_set_align_items(sUIRoot, ALIGN_ITEMS_FLEX_START);

    // Create a sContainerMain to act as the modal background and hold the elements in the modal.
    sContainerMain = recompui_create_element(sUIContext, sUIRoot);

    // Take up the a fixed height and the full width, up to a maximum width.
    recompui_set_height(sContainerMain, 25.0f, UNIT_PERCENT);
    recompui_set_flex_grow(sContainerMain, 1.0f);
    recompui_set_max_width(sContainerMain, modal_max_width, UNIT_DP);
    recompui_set_width(sContainerMain, 33.0f, UNIT_PERCENT);

    // Set up the properties of the sContainerMain.
    recompui_set_display(sContainerMain, DISPLAY_FLEX);
    recompui_set_justify_content(sContainerMain, JUSTIFY_CONTENT_FLEX_START);
    recompui_set_flex_direction(sContainerMain, FLEX_DIRECTION_COLUMN);
    recompui_set_padding(sContainerMain, 16.0f, UNIT_DP);
    recompui_set_gap(sContainerMain, 16.0f, UNIT_DP);
    recompui_set_align_items(sContainerMain, ALIGN_ITEMS_FLEX_START);

    // Set up the sContainerMain to be the modal's background.
    recompui_set_border_width(sContainerMain, modal_border_width, UNIT_DP);
    recompui_set_border_radius(sContainerMain, modal_border_radius, UNIT_DP);
    recompui_set_border_color(sContainerMain, &border_color);
    recompui_set_background_color(sContainerMain, &modal_color);

    sRowTop = recompui_create_element(sUIContext, sContainerMain);
    recompui_set_flex_basis(sRowTop, 100.0f, UNIT_DP);
    recompui_set_flex_grow(sRowTop, 0);
    recompui_set_flex_shrink(sRowTop, 0);
    recompui_set_display(sRowTop, DISPLAY_FLEX);
    recompui_set_flex_direction(sRowTop, FLEX_DIRECTION_ROW);
    recompui_set_justify_content(sRowTop, JUSTIFY_CONTENT_FLEX_START);
    recompui_set_align_items(sRowTop, ALIGN_ITEMS_FLEX_END);
    recompui_set_gap(sRowTop, 16.0f, UNIT_DP);

    sRowCategory = recompui_create_element(sUIContext, sContainerMain);
    recompui_set_flex_basis(sRowCategory, 100.0f, UNIT_DP);
    recompui_set_flex_grow(sRowCategory, 0);
    recompui_set_flex_shrink(sRowCategory, 0);
    recompui_set_display(sRowCategory, DISPLAY_FLEX);
    recompui_set_flex_direction(sRowCategory, FLEX_DIRECTION_ROW);
    recompui_set_justify_content(sRowCategory, JUSTIFY_CONTENT_FLEX_START);
    recompui_set_align_items(sRowCategory, ALIGN_ITEMS_CENTER);
    recompui_set_gap(sRowCategory, 16.0f, UNIT_DP);

    sButtonClose = recompui_create_button(sUIContext, sRowTop, "Apply", BUTTONSTYLE_SECONDARY);
    recompui_set_text_align(sButtonClose, TEXT_ALIGN_CENTER);

    // Bind the shared callback to the two buttons.
    recompui_register_callback(sButtonClose, closeButtonPressed, NULL);

    // set up scrolling sContainerMain for models
    sContainerListButtons = recompui_create_element(sUIContext, sContainerMain);
    recompui_set_flex_basis(sContainerListButtons, 100.0f, UNIT_DP);
    recompui_set_flex_grow(sContainerListButtons, 1.0f);
    recompui_set_flex_shrink(sContainerListButtons, 0.0f);
    recompui_set_display(sContainerListButtons, DISPLAY_FLEX);
    recompui_set_flex_direction(sContainerListButtons, FLEX_DIRECTION_COLUMN);
    recompui_set_justify_content(sContainerListButtons, JUSTIFY_CONTENT_FLEX_START);
    recompui_set_align_items(sContainerListButtons, ALIGN_ITEMS_FLEX_START);
    recompui_set_gap(sContainerListButtons, 16.0f, UNIT_DP);
    recompui_set_overflow_y(sContainerListButtons, OVERFLOW_SCROLL);
    recompui_set_overflow_x(sContainerListButtons, OVERFLOW_HIDDEN);

    recompui_close_context(sUIContext);

    sIsUIContextShown = false;

    for (int i = 0; i < ARRAY_COUNT(sCategoryInfos); ++i) {
        CategoryInfo *catInf = &sCategoryInfos[i];
        catInf->isVisible = catInf->isVisible || catInf->isUsedByCurrentGame;
    }

    recompui_open_context(sUIContext);
    refreshFileList();
    recompui_close_context(sUIContext);
}

static void onCategoryButtonPressed(RecompuiResource resource, const RecompuiEventData *data, void *userdata) {
    if (sIsUIContextShown) {
        if (data->type == UI_EVENT_CLICK) {
            int index = (int)userdata;

            if (isValidCategoryInfoIndex(index)) {
                sCurrentCategoryInfo = index;
                refreshFileList();
            }
        }
    }
}

static void onUpOneLevelButtonPressed(RecompuiResource resource, const RecompuiEventData *data, void *userdata) {
    if (sIsUIContextShown) {
        if (data->type == UI_EVENT_CLICK) {
            if (isSelectingModel()) {
                sCurrentCategoryInfo = SELECTING_CATEGORY;
                refreshFileList();
            }
        } else if (data->type == UI_EVENT_FOCUS || data->type == UI_EVENT_HOVER) {
        }
    }
}

static void destroyUpOneLevelButton() {
    if (sButtonUpOneMenuLevel) {
        recompui_destroy_element(sRowTop, sButtonUpOneMenuLevel);
        sButtonUpOneMenuLevel = 0;
    }
}

static void createUpOneLevelButton() {
    destroyUpOneLevelButton();
    sButtonUpOneMenuLevel = recompui_create_button(sUIContext, sRowTop, "▲", BUTTONSTYLE_SECONDARY);
    recompui_register_callback(sButtonUpOneMenuLevel, onUpOneLevelButtonPressed, NULL);
}

static void destroyModelButtons() {
    {
        size_t count = getCurrentButtonArraySize();
        const RecompuiResource *buttons = getCurrentButtonArray();

        for (size_t i = 0; i < count; ++i) {
            recompui_destroy_element(sContainerListButtons, buttons[i]);
        }
    }

    clearButtonList();

    if (isSelectingCategory()) {
        destroyNextPrevCategoryButtons();
        destroyUpOneLevelButton();
    }
}

static void createCategoryListButtons() {
    RecompuiResource removeModelsButton = createAndPushButtonToList(sUIContext, sContainerListButtons, "[Remove All Models]", BUTTONSTYLE_PRIMARY);
    recompui_register_callback(removeModelsButton, removeAllModelsButtonPressed, NULL);
    setButtonColor(removeModelsButton, &sModelRemovedButtonColor);

    for (int i = 0; i < ARRAY_COUNT(sCategoryInfos); ++i) {
        CategoryInfo *curr = &sCategoryInfos[i];

        if (curr->isVisible) {
            RecompuiResource categoryButton = createAndPushButtonToList(sUIContext, sContainerListButtons, curr->displayName, BUTTONSTYLE_PRIMARY);
            recompui_register_callback(categoryButton, onCategoryButtonPressed, (void *)i);
        }
    }

    connectListBoxButtons(getCurrentButtonArray(), getCurrentButtonArraySize());
}

static void createModelListButtons() {
    const char *removeText = "[None]";

    RecompuiResource removeModelButton = createAndPushButtonToList(sUIContext, sContainerListButtons, removeText, BUTTONSTYLE_PRIMARY);
    setButtonColor(removeModelButton, &sModelRemovedButtonColor);

    connectListBoxButtons(getCurrentButtonArray(), getCurrentButtonArraySize());
}

static void refreshFileList() {
    // MUST CALL INSIDE UI CONTEXT

    destroyModelButtons();
    if (isSelectingCategory()) {
        createCategoryListButtons();

        size_t listCount = getCurrentButtonArraySize();

        if (listCount > 0) {
            const RecompuiResource *buttons = getCurrentButtonArray();
            RecompuiResource first = buttons[0];
            RecompuiResource last = buttons[listCount - 1];

            recompui_set_nav(first, NAVDIRECTION_UP, sButtonClose);
            recompui_set_nav(first, NAVDIRECTION_LEFT, sButtonClose);

            recompui_set_nav(sButtonClose, NAVDIRECTION_DOWN, first);
            recompui_set_nav(sButtonClose, NAVDIRECTION_UP, last);
            recompui_set_nav(sButtonClose, NAVDIRECTION_RIGHT, first);

            recompui_set_nav(last, NAVDIRECTION_DOWN, sButtonClose);
        }
    } else if (isSelectingModel()) {
        createModelListButtons();

        if (!sButtonUpOneMenuLevel) {
            createUpOneLevelButton();
        }

        if (!sButtonCategoryNext || !sButtonCategoryPrev) {
            createNextPrevCategoryButtons();
        }

        recompui_set_nav(sButtonUpOneMenuLevel, NAVDIRECTION_RIGHT, sButtonCategoryPrev);
        recompui_set_nav(sButtonUpOneMenuLevel, NAVDIRECTION_DOWN, sButtonCategoryNext);

        recompui_set_nav(sButtonCategoryNext, NAVDIRECTION_UP, sButtonUpOneMenuLevel);

        recompui_set_nav(sButtonCategoryPrev, NAVDIRECTION_LEFT, sButtonUpOneMenuLevel);
        recompui_set_nav(sButtonCategoryPrev, NAVDIRECTION_UP, sButtonClose);

        recompui_set_nav(sButtonClose, NAVDIRECTION_RIGHT, sButtonUpOneMenuLevel);
        recompui_set_nav(sButtonClose, NAVDIRECTION_DOWN, sButtonCategoryPrev);

        size_t listCount = getCurrentButtonArraySize();

        if (listCount > 0) {
            const RecompuiResource *buttons = getCurrentButtonArray();
            RecompuiResource first = buttons[0];
            RecompuiResource last = buttons[listCount - 1];

            recompui_set_nav(first, NAVDIRECTION_UP, sButtonCategoryPrev);
            recompui_set_nav(first, NAVDIRECTION_LEFT, sButtonCategoryNext);

            recompui_set_nav(sButtonCategoryNext, NAVDIRECTION_DOWN, first);
            recompui_set_nav(sButtonCategoryNext, NAVDIRECTION_RIGHT, first);

            recompui_set_nav(sButtonCategoryPrev, NAVDIRECTION_DOWN, first);

            recompui_set_nav(sButtonClose, NAVDIRECTION_UP, last);

            recompui_set_nav(last, NAVDIRECTION_DOWN, sButtonClose);

            recompui_set_nav(sButtonUpOneMenuLevel, NAVDIRECTION_UP, last);
        } else {
            recompui_set_nav(sButtonCategoryNext, NAVDIRECTION_DOWN, sButtonUpOneMenuLevel);

            recompui_set_nav(sButtonCategoryPrev, NAVDIRECTION_DOWN, sButtonClose);

            recompui_set_nav(sButtonClose, NAVDIRECTION_UP, sButtonCategoryPrev);

            recompui_set_nav(sButtonUpOneMenuLevel, NAVDIRECTION_UP, sButtonCategoryNext);
        }
    }
    refreshCategoryName();
}

static bool isOpenMenuComboPressed(PlayState *play) {
    Input *input = CONTROLLER1(&play->state);

    if ((CHECK_BTN_ALL(input->press.button, BTN_L) && CHECK_BTN_ALL(input->cur.button, BTN_A)) ||
        (CHECK_BTN_ALL(input->cur.button, BTN_L) && CHECK_BTN_ALL(input->press.button, BTN_A))) {
        return true;
    }

    return false;
}

static void openModelMenu() {
    if (!sIsUIContextShown) {
        recompui_show_context(sUIContext);
        sIsUIContextShown = true;
    }
}

// Hook Play_UpdateMain to check if the L button is pressed and show this mod's UI if so.
RECOMP_HOOK("Play_UpdateMain")
void checkToOpenModelMenu_on_Play_UpdateMain(PlayState *play) {
    if (isOpenMenuComboPressed(play)) {
        openModelMenu();
    }
}
