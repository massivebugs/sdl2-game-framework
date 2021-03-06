#ifndef ZJVL_EVENT_KEY_EVENT_H
#define ZJVL_EVENT_KEY_EVENT_H

#include "event.h"

namespace ZJVL
{
	enum class Key
	{
		NONE,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		D1,
		D2,
		D3,
		D4,
		D5,
		D6,
		D7,
		D8,
		D9,
		D10,
		TAB,
		ENTER,
		CAPS_LOCK,
		L_SHIFT,
		R_SHIFT,
		L_CTRL,
		R_CTRL,
		PAGE_UP,
		PAGEDOWN,
		BACK,
		INSERT,
		ESC,
		SPACE,
		L_ALT,
		R_ALT,
		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z
	};

	class KeyEvent : public Event
	{
	public:
		virtual Key get_key() const = 0;

	private:
		Key m_key;
	};

	class KeyDownEvent : public KeyEvent
	{
	public:
		KeyDownEvent(Key key);

		EventType get_type() const override;
		Key get_key() const override;

	private:
		Key m_key;
	};
}

#endif