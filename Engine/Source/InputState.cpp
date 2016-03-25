// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from GameInput.h in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#include "Stdafx.h"

#include "InputState.h"

#include "DebugUtility.h"

using namespace Kodiak;

#include <XInput.h>
#pragma comment(lib, "xinput9_1_0.lib")

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")


namespace
{
bool s_Buttons[2][InputState::kNumDigitalInputs];
float s_HoldDuration[InputState::kNumDigitalInputs] = { 0.0f };
float s_Analogs[InputState::kNumAnalogInputs];
float s_AnalogsTC[InputState::kNumAnalogInputs];

IDirectInput8A* s_DI;
IDirectInputDevice8A* s_Keyboard;
IDirectInputDevice8A* s_Mouse;

_DIMOUSESTATE2 s_MouseState;
unsigned char s_Keybuffer[256];
unsigned char s_DXKeyMapping[InputState::kNumKeys]; // map DigitalInput enum to DX key codes 


float FilterAnalogInput(int val, int deadZone)
{
	if (val < 0)
	{
		if (val > -deadZone)
			return 0.0f;
		else
			return (val + deadZone) / (32768.0f - deadZone);
	}
	else
	{
		if (val < deadZone)
			return 0.0f;
		else
			return (val - deadZone) / (32767.0f - deadZone);
	}
}


void KbmBuildKeyMapping()
{
	s_DXKeyMapping[InputState::kKey_escape] = 1;
	s_DXKeyMapping[InputState::kKey_1] = 2;
	s_DXKeyMapping[InputState::kKey_2] = 3;
	s_DXKeyMapping[InputState::kKey_3] = 4;
	s_DXKeyMapping[InputState::kKey_4] = 5;
	s_DXKeyMapping[InputState::kKey_5] = 6;
	s_DXKeyMapping[InputState::kKey_6] = 7;
	s_DXKeyMapping[InputState::kKey_7] = 8;
	s_DXKeyMapping[InputState::kKey_8] = 9;
	s_DXKeyMapping[InputState::kKey_9] = 10;
	s_DXKeyMapping[InputState::kKey_0] = 11;
	s_DXKeyMapping[InputState::kKey_minus] = 12;
	s_DXKeyMapping[InputState::kKey_equals] = 13;
	s_DXKeyMapping[InputState::kKey_back] = 14;
	s_DXKeyMapping[InputState::kKey_tab] = 15;
	s_DXKeyMapping[InputState::kKey_q] = 16;
	s_DXKeyMapping[InputState::kKey_w] = 17;
	s_DXKeyMapping[InputState::kKey_e] = 18;
	s_DXKeyMapping[InputState::kKey_r] = 19;
	s_DXKeyMapping[InputState::kKey_t] = 20;
	s_DXKeyMapping[InputState::kKey_y] = 21;
	s_DXKeyMapping[InputState::kKey_u] = 22;
	s_DXKeyMapping[InputState::kKey_i] = 23;
	s_DXKeyMapping[InputState::kKey_o] = 24;
	s_DXKeyMapping[InputState::kKey_p] = 25;
	s_DXKeyMapping[InputState::kKey_lbracket] = 26;
	s_DXKeyMapping[InputState::kKey_rbracket] = 27;
	s_DXKeyMapping[InputState::kKey_return] = 28;
	s_DXKeyMapping[InputState::kKey_lcontrol] = 29;
	s_DXKeyMapping[InputState::kKey_a] = 30;
	s_DXKeyMapping[InputState::kKey_s] = 31;
	s_DXKeyMapping[InputState::kKey_d] = 32;
	s_DXKeyMapping[InputState::kKey_f] = 33;
	s_DXKeyMapping[InputState::kKey_g] = 34;
	s_DXKeyMapping[InputState::kKey_h] = 35;
	s_DXKeyMapping[InputState::kKey_j] = 36;
	s_DXKeyMapping[InputState::kKey_k] = 37;
	s_DXKeyMapping[InputState::kKey_l] = 38;
	s_DXKeyMapping[InputState::kKey_semicolon] = 39;
	s_DXKeyMapping[InputState::kKey_apostrophe] = 40;
	s_DXKeyMapping[InputState::kKey_grave] = 41;
	s_DXKeyMapping[InputState::kKey_lshift] = 42;
	s_DXKeyMapping[InputState::kKey_backslash] = 43;
	s_DXKeyMapping[InputState::kKey_z] = 44;
	s_DXKeyMapping[InputState::kKey_x] = 45;
	s_DXKeyMapping[InputState::kKey_c] = 46;
	s_DXKeyMapping[InputState::kKey_v] = 47;
	s_DXKeyMapping[InputState::kKey_b] = 48;
	s_DXKeyMapping[InputState::kKey_n] = 49;
	s_DXKeyMapping[InputState::kKey_m] = 50;
	s_DXKeyMapping[InputState::kKey_comma] = 51;
	s_DXKeyMapping[InputState::kKey_period] = 52;
	s_DXKeyMapping[InputState::kKey_slash] = 53;
	s_DXKeyMapping[InputState::kKey_rshift] = 54;
	s_DXKeyMapping[InputState::kKey_multiply] = 55;
	s_DXKeyMapping[InputState::kKey_lalt] = 56;
	s_DXKeyMapping[InputState::kKey_space] = 57;
	s_DXKeyMapping[InputState::kKey_capital] = 58;
	s_DXKeyMapping[InputState::kKey_f1] = 59;
	s_DXKeyMapping[InputState::kKey_f2] = 60;
	s_DXKeyMapping[InputState::kKey_f3] = 61;
	s_DXKeyMapping[InputState::kKey_f4] = 62;
	s_DXKeyMapping[InputState::kKey_f5] = 63;
	s_DXKeyMapping[InputState::kKey_f6] = 64;
	s_DXKeyMapping[InputState::kKey_f7] = 65;
	s_DXKeyMapping[InputState::kKey_f8] = 66;
	s_DXKeyMapping[InputState::kKey_f9] = 67;
	s_DXKeyMapping[InputState::kKey_f10] = 68;
	s_DXKeyMapping[InputState::kKey_numlock] = 69;
	s_DXKeyMapping[InputState::kKey_scroll] = 70;
	s_DXKeyMapping[InputState::kKey_numpad7] = 71;
	s_DXKeyMapping[InputState::kKey_numpad8] = 72;
	s_DXKeyMapping[InputState::kKey_numpad9] = 73;
	s_DXKeyMapping[InputState::kKey_subtract] = 74;
	s_DXKeyMapping[InputState::kKey_numpad4] = 75;
	s_DXKeyMapping[InputState::kKey_numpad5] = 76;
	s_DXKeyMapping[InputState::kKey_numpad6] = 77;
	s_DXKeyMapping[InputState::kKey_add] = 78;
	s_DXKeyMapping[InputState::kKey_numpad1] = 79;
	s_DXKeyMapping[InputState::kKey_numpad2] = 80;
	s_DXKeyMapping[InputState::kKey_numpad3] = 81;
	s_DXKeyMapping[InputState::kKey_numpad0] = 82;
	s_DXKeyMapping[InputState::kKey_decimal] = 83;
	s_DXKeyMapping[InputState::kKey_f11] = 87;
	s_DXKeyMapping[InputState::kKey_f12] = 88;
	s_DXKeyMapping[InputState::kKey_numpadenter] = 156;
	s_DXKeyMapping[InputState::kKey_rcontrol] = 157;
	s_DXKeyMapping[InputState::kKey_divide] = 181;
	s_DXKeyMapping[InputState::kKey_sysrq] = 183;
	s_DXKeyMapping[InputState::kKey_ralt] = 184;
	s_DXKeyMapping[InputState::kKey_pause] = 197;
	s_DXKeyMapping[InputState::kKey_home] = 199;
	s_DXKeyMapping[InputState::kKey_up] = 200;
	s_DXKeyMapping[InputState::kKey_pgup] = 201;
	s_DXKeyMapping[InputState::kKey_left] = 203;
	s_DXKeyMapping[InputState::kKey_right] = 205;
	s_DXKeyMapping[InputState::kKey_end] = 207;
	s_DXKeyMapping[InputState::kKey_down] = 208;
	s_DXKeyMapping[InputState::kKey_pgdn] = 209;
	s_DXKeyMapping[InputState::kKey_insert] = 210;
	s_DXKeyMapping[InputState::kKey_delete] = 211;
	s_DXKeyMapping[InputState::kKey_lwin] = 219;
	s_DXKeyMapping[InputState::kKey_rwin] = 220;
	s_DXKeyMapping[InputState::kKey_apps] = 221;
}


void KbmZeroInputs()
{
	memset(&s_MouseState, 0, sizeof(DIMOUSESTATE2));
	memset(s_Keybuffer, 0, sizeof(s_Keybuffer));
}


void KbmInitialize(HWND hwnd)
{
	KbmBuildKeyMapping();

	if (FAILED(DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&s_DI, nullptr)))
	{
		assert_msg(false, "DirectInput8 initialization failed.");
	}

	if (FAILED(s_DI->CreateDevice(GUID_SysKeyboard, &s_Keyboard, nullptr)))
	{
		assert_msg(false, "Keyboard CreateDevice failed.");
	}

	if (FAILED(s_Keyboard->SetDataFormat(&c_dfDIKeyboard)))
	{
		assert_msg(false, "Keyboard SetDataFormat failed.");
	}

	if (FAILED(s_Keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
	{
		assert_msg(false, "Keyboard SetCooperativeLevel failed.");
	}

	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = 10;

	if (FAILED(s_Keyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
	{
		assert_msg(false, "Keyboard set buffer size failed.");
	}

	if (FAILED(s_DI->CreateDevice(GUID_SysMouse, &s_Mouse, nullptr)))
	{
		assert_msg(false, "Mouse CreateDevice failed.");
	}

	if (FAILED(s_Mouse->SetDataFormat(&c_dfDIMouse2)))
	{
		assert_msg(false, "Mouse SetDataFormat failed.");
	}

	if (FAILED(s_Mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE)))
	{
		assert_msg(false, "Mouse SetCooperativeLevel failed.");
	}

	KbmZeroInputs();
}


void KbmShutdown()
{
	if (s_Keyboard)
	{
		s_Keyboard->Unacquire();
		s_Keyboard->Release();
		s_Keyboard = nullptr;
	}
	if (s_Mouse)
	{
		s_Mouse->Unacquire();
		s_Mouse->Release();
		s_Mouse = nullptr;
	}
	if (s_DI)
	{
		s_DI->Release();
		s_DI = nullptr;
	}
}


void KbmUpdate(HWND hwnd)
{
	HWND foreground = GetForegroundWindow();
	bool visible = IsWindowVisible(foreground) != 0;

	if (foreground != hwnd // wouldn't be able to acquire
		|| !visible)
	{
		KbmZeroInputs();
	}
	else
	{
		s_Mouse->Acquire();
		s_Mouse->GetDeviceState(sizeof(DIMOUSESTATE2), &s_MouseState);
		s_Keyboard->Acquire();
		s_Keyboard->GetDeviceState(sizeof(s_Keybuffer), s_Keybuffer);
	}
}

} // anonymous namespace


void InputState::Initialize(HWND hwnd)
{
	m_hwnd = hwnd;

	// For Windows 8
	//	XInputEnable(TRUE);

	ZeroMemory(s_Buttons, sizeof(s_Buttons));
	ZeroMemory(s_Analogs, sizeof(s_Analogs));

	KbmInitialize(m_hwnd);
}


void InputState::Shutdown()
{
	KbmShutdown();
}


void InputState::Update(float frameDelta)
{
	memcpy(s_Buttons[1], s_Buttons[0], sizeof(s_Buttons[0]));
	memset(s_Buttons[0], 0, sizeof(s_Buttons[0]));
	memset(s_Analogs, 0, sizeof(s_Analogs));

	XINPUT_STATE newInputState;
	if (ERROR_SUCCESS == XInputGetState(0, &newInputState))
	{
		if (newInputState.Gamepad.wButtons & (1 << 0)) s_Buttons[0][kDPadUp] = true;
		if (newInputState.Gamepad.wButtons & (1 << 1)) s_Buttons[0][kDPadDown] = true;
		if (newInputState.Gamepad.wButtons & (1 << 2)) s_Buttons[0][kDPadLeft] = true;
		if (newInputState.Gamepad.wButtons & (1 << 3)) s_Buttons[0][kDPadRight] = true;
		if (newInputState.Gamepad.wButtons & (1 << 4)) s_Buttons[0][kStartButton] = true;
		if (newInputState.Gamepad.wButtons & (1 << 5)) s_Buttons[0][kBackButton] = true;
		if (newInputState.Gamepad.wButtons & (1 << 6)) s_Buttons[0][kLThumbClick] = true;
		if (newInputState.Gamepad.wButtons & (1 << 7)) s_Buttons[0][kRThumbClick] = true;
		if (newInputState.Gamepad.wButtons & (1 << 8)) s_Buttons[0][kLShoulder] = true;
		if (newInputState.Gamepad.wButtons & (1 << 9)) s_Buttons[0][kRShoulder] = true;
		if (newInputState.Gamepad.wButtons & (1 << 12)) s_Buttons[0][kAButton] = true;
		if (newInputState.Gamepad.wButtons & (1 << 13)) s_Buttons[0][kBButton] = true;
		if (newInputState.Gamepad.wButtons & (1 << 14)) s_Buttons[0][kXButton] = true;
		if (newInputState.Gamepad.wButtons & (1 << 15)) s_Buttons[0][kYButton] = true;

		s_Analogs[kAnalogLeftTrigger] = newInputState.Gamepad.bLeftTrigger / 255.0f;
		s_Analogs[kAnalogRightTrigger] = newInputState.Gamepad.bRightTrigger / 255.0f;
		s_Analogs[kAnalogLeftStickX] = FilterAnalogInput(newInputState.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		s_Analogs[kAnalogLeftStickY] = FilterAnalogInput(newInputState.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		s_Analogs[kAnalogRightStickX] = FilterAnalogInput(newInputState.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		s_Analogs[kAnalogRightStickY] = FilterAnalogInput(newInputState.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
	}

	KbmUpdate(m_hwnd);

	for (uint32_t i = 0; i < kNumKeys; ++i)
	{
		s_Buttons[0][i] = (s_Keybuffer[s_DXKeyMapping[i]] & 0x80) != 0;
	}

	for (uint32_t i = 0; i < 8; ++i)
	{
		if (s_MouseState.rgbButtons[i] > 0) s_Buttons[0][kMouse0 + i] = true;
	}

	s_Analogs[kAnalogMouseX] = (float)s_MouseState.lX * .0018f;
	s_Analogs[kAnalogMouseY] = (float)s_MouseState.lY * -.0018f;

	if (s_MouseState.lZ > 0)
	{
		s_Analogs[kAnalogMouseScroll] = 1.0f;
	}
	else if (s_MouseState.lZ < 0)
	{
		s_Analogs[kAnalogMouseScroll] = -1.0f;
	}

	// Update time duration for buttons pressed
	for (uint32_t i = 0; i < kNumDigitalInputs; ++i)
	{
		if (s_Buttons[0][i])
		{
			if (!s_Buttons[1][i])
			{
				s_HoldDuration[i] = 0.0f;
			}
			else
			{
				s_HoldDuration[i] += frameDelta;
			}
		}
	}

	for (uint32_t i = 0; i < kNumAnalogInputs; ++i)
	{
		s_AnalogsTC[i] = s_Analogs[i] * frameDelta;
	}
}


bool InputState::IsAnyPressed(void)
{
	return s_Buttons[0] != 0;
}


bool InputState::IsPressed(DigitalInput di)
{
	return s_Buttons[0][di];
}


bool InputState::IsFirstPressed(DigitalInput di)
{
	return s_Buttons[0][di] && !s_Buttons[1][di];
}


bool InputState::IsReleased(DigitalInput di)
{
	return !s_Buttons[0][di];
}


bool InputState::IsFirstReleased(DigitalInput di)
{
	return !s_Buttons[0][di] && s_Buttons[1][di];
}


float InputState::GetDurationPressed(DigitalInput di)
{
	return s_HoldDuration[di];
}


float InputState::GetAnalogInput(AnalogInput ai)
{
	return s_Analogs[ai];
}


float InputState::GetTimeCorrectedAnalogInput(AnalogInput ai)
{
	return s_AnalogsTC[ai];
}