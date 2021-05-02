#pragma once

/**
*	@file
*
*	these are the key numbers that should be passed to Key_Event
*/

constexpr int K_TAB = 9;
constexpr int K_ENTER = 13;
constexpr int K_ESCAPE = 27;
constexpr int K_SPACE = 32;

// normal keys should be passed as lowercased ascii

constexpr int K_BACKSPACE = 127;
constexpr int K_UPARROW = 128;
constexpr int K_DOWNARROW = 129;
constexpr int K_LEFTARROW = 130;
constexpr int K_RIGHTARROW = 131;

constexpr int K_ALT = 132;
constexpr int K_CTRL = 133;
constexpr int K_SHIFT = 134;
constexpr int K_F1 = 135;
constexpr int K_F2 = 136;
constexpr int K_F3 = 137;
constexpr int K_F4 = 138;
constexpr int K_F5 = 139;
constexpr int K_F6 = 140;
constexpr int K_F7 = 141;
constexpr int K_F8 = 142;
constexpr int K_F9 = 143;
constexpr int K_F10 = 144;
constexpr int K_F11 = 145;
constexpr int K_F12 = 146;
constexpr int K_INS = 147;
constexpr int K_DEL = 148;
constexpr int K_PGDN = 149;
constexpr int K_PGUP = 150;
constexpr int K_HOME = 151;
constexpr int K_END = 152;

constexpr int K_KP_HOME = 160;
constexpr int K_KP_UPARROW = 161;
constexpr int K_KP_PGUP = 162;
constexpr int K_KP_LEFTARROW = 163;
constexpr int K_KP_5 = 164;
constexpr int K_KP_RIGHTARROW = 165;
constexpr int K_KP_END = 166;
constexpr int K_KP_DOWNARROW = 167;
constexpr int K_KP_PGDN = 168;
constexpr int K_KP_ENTER = 169;
constexpr int K_KP_INS = 170;
constexpr int K_KP_DEL = 171;
constexpr int K_KP_SLASH = 172;
constexpr int K_KP_MINUS = 173;
constexpr int K_KP_PLUS = 174;
constexpr int K_CAPSLOCK = 175;
constexpr int K_KP_MUL = 176;
constexpr int K_WIN = 177;

//
// joystick buttons
//
constexpr int K_JOY1 = 203;
constexpr int K_JOY2 = 204;
constexpr int K_JOY3 = 205;
constexpr int K_JOY4 = 206;

//
// aux keys are for multi-buttoned joysticks to generate so they can use
// the normal binding process
//
constexpr int K_AUX1 = 207;
constexpr int K_AUX2 = 208;
constexpr int K_AUX3 = 209;
constexpr int K_AUX4 = 210;
constexpr int K_AUX5 = 211;
constexpr int K_AUX6 = 212;
constexpr int K_AUX7 = 213;
constexpr int K_AUX8 = 214;
constexpr int K_AUX9 = 215;
constexpr int K_AUX10 = 216;
constexpr int K_AUX11 = 217;
constexpr int K_AUX12 = 218;
constexpr int K_AUX13 = 219;
constexpr int K_AUX14 = 220;
constexpr int K_AUX15 = 221;
constexpr int K_AUX16 = 222;
constexpr int K_AUX17 = 223;
constexpr int K_AUX18 = 224;
constexpr int K_AUX19 = 225;
constexpr int K_AUX20 = 226;
constexpr int K_AUX21 = 227;
constexpr int K_AUX22 = 228;
constexpr int K_AUX23 = 229;
constexpr int K_AUX24 = 230;
constexpr int K_AUX25 = 231;
constexpr int K_AUX26 = 232;
constexpr int K_AUX27 = 233;
constexpr int K_AUX28 = 234;
constexpr int K_AUX29 = 235;
constexpr int K_AUX30 = 236;
constexpr int K_AUX31 = 237;
constexpr int K_AUX32 = 238;
constexpr int K_MWHEELDOWN = 239;
constexpr int K_MWHEELUP = 240;

constexpr int K_PAUSE = 255;

//
// mouse buttons generate virtual keys
//
constexpr int K_MOUSE1 = 241;
constexpr int K_MOUSE2 = 242;
constexpr int K_MOUSE3 = 243;
constexpr int K_MOUSE4 = 244;
constexpr int K_MOUSE5 = 245;
