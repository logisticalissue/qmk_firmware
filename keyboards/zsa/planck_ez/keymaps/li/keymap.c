#include QMK_KEYBOARD_H
#include "eeprom.h"
#ifndef ZSA_SAFE_RANGE
#define ZSA_SAFE_RANGE SAFE_RANGE
#endif

enum custom_keycodes {
  RGB_SLD = ZSA_SAFE_RANGE,
  LOWER_GLOBE
};


enum planck_layers {
  _BASE,
  _LOWER,
  _RAISE,
  _ADJUST,
};

#define LOWER MO(_LOWER)
#define RAISE MO(_RAISE)


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [_BASE] = LAYOUT_planck_grid(
    KC_TAB,         KC_Q,           KC_W,           KC_F,           KC_P,           KC_G,           KC_J,           KC_L,           KC_U,           KC_Y,           KC_SCLN,        KC_BSPC,
    KC_ESCAPE,MT(MOD_LGUI, KC_A),MT(MOD_LALT, KC_R),MT(MOD_LCTL, KC_S),MT(MOD_LSFT, KC_T),KC_D,           KC_H,           MT(MOD_RSFT, KC_N),MT(MOD_RCTL, KC_E),MT(MOD_RALT, KC_I),MT(MOD_RGUI, KC_O),KC_QUOTE,
    KC_LEFT_SHIFT,  KC_Z,           KC_X,           KC_C,           KC_V,           KC_B,           KC_K,           KC_M,           KC_COMMA,       KC_DOT,         KC_SLASH,       KC_RSFT,
    KC_LEFT,        AP_FN,   LOWER_GLOBE,    LOWER,          RAISE,          KC_SPACE,       KC_NO,          KC_ENTER,       KC_GRAVE,       KC_DOWN,        KC_UP,          KC_RIGHT
  ),

  [_LOWER] = LAYOUT_planck_grid(
    _______, KC_F1,          KC_F2,          KC_F3,          KC_F4,          _______, KC_HOME,        KC_PAGE_UP,     MS_UP,       KC_PGDN,        KC_END,         _______,
    _______, KC_F5,          KC_F6,          KC_F7,          KC_F8,          _______, MS_WHLD, MS_LEFT,     MS_DOWN,     MS_RGHT,    MS_WHLU, KC_MS_ACCEL0,
    _______, KC_F9,          KC_F10,         KC_F11,         KC_F12,         _______, _______, _______, _______, _______, _______, _______,
    _______, _______, _______, _______, _______, KC_MS_BTN3,     KC_NO,          KC_MS_BTN1,     KC_MS_BTN2,     _______,   _______, _______
  ),

  [_RAISE] = LAYOUT_planck_grid(
    KC_TILD, KC_1,           KC_2,           KC_3,           KC_4,           KC_5,           KC_6,           KC_7,           KC_8,           KC_9,           KC_0,           _______,
    _______, KC_LPRN,        KC_RPRN,        KC_HASH,        KC_EQUAL,       KC_PERC,        KC_CIRC,        KC_MINUS,       KC_ASTR,        KC_LCBR,        KC_RCBR,        KC_LBRC,
    KC_PIPE, KC_EXLM,        KC_AT,          KC_PLUS,        KC_DLR,         KC_COLN,        KC_UNDS,        KC_AMPR,        KC_LABK,        KC_RABK,        KC_BSLS,        KC_RBRC,
    _______, _______, _______, _______, _______, _______, KC_NO,          _______, KC_PAUSE,       KC_AUDIO_VOL_DOWN,KC_AUDIO_VOL_UP,KC_MEDIA_PLAY_PAUSE
  ),

  [_ADJUST] = LAYOUT_planck_grid(
    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
    _______, _______, QK_AUDIO_ON,    QK_AUDIO_OFF,   AU_TOGG,        _______, TOGGLE_LAYER_COLOR,RGB_TOG,        RGB_VAI,        RGB_VAD,        _______, QK_BOOT,
    _______, _______, QK_MUSIC_ON,    QK_MUSIC_OFF,   MU_TOGG,        _______, _______, RGB_MODE_FORWARD,RGB_HUI,        RGB_HUD,        _______, _______,
    _______, _______, _______, _______, _______, _______, KC_NO,          _______, _______, _______, _______, _______
  )
};


uint8_t layer_state_set_user(uint8_t state) {
    return update_tri_layer_state(state, _LOWER, _RAISE, _ADJUST);
}

#define HOLD_PLAIN 1
#define HOLD_GLOBE 2
static uint8_t lower_held = 0;

static bool apple_fn_on = false;

static void apple_fn_set(bool on) {
    if (apple_fn_on == on) return;
    apple_fn_on = on;
    keyboard_report->reserved = on ? 1 : 0;
    send_keyboard_report();
}

static void lower_update(uint8_t bit, bool pressed) {
    if (pressed) lower_held |= bit; else lower_held &= ~bit;

    /* Globe first on the way in, last on the way out, so it's already
       in the report before any layer key is sent */
    if (lower_held & HOLD_GLOBE) apple_fn_set(true);

    if (lower_held) layer_on(_LOWER); else layer_off(_LOWER);

    if (!(lower_held & HOLD_GLOBE)) apple_fn_set(false);
}

static bool last_was_number;

bool process_symbol_angle_switch(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed || !IS_LAYER_ON(_RAISE))
        return true;

    if (last_was_number)
    {
        switch (keycode) {
            case KC_LT:
                tap_code16(KC_COMM);
                return false;
            case KC_GT:
                tap_code16(KC_DOT);
                return false;
        }
    }

    return true;
}

void update_last_was_number(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed)
        return;

    switch (keycode) {
        case KC_1:
        case KC_2:
        case KC_3:
        case KC_4:
        case KC_5:
        case KC_6:
        case KC_7:
        case KC_8:
        case KC_9:
        case KC_0:
            last_was_number = true;
            break;
        default:
            last_was_number = false;
            break;
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case LOWER:
            lower_update(HOLD_PLAIN, record->event.pressed);
            return false;
        case LOWER_GLOBE:
            lower_update(HOLD_GLOBE, record->event.pressed);
            return false;
        case RGB_SLD:
            if (rawhid_state.rgb_control) { }
            if (record->event.pressed) { rgblight_mode(1); }
    }
    if (!process_symbol_angle_switch(keycode, record)) return false;
    update_last_was_number(keycode, record);
    return true;
}
