#ifndef ZJVL_EVENT_OBSERVER_H
#define ZJVL_EVENT_OBSERVER_H

#include "event.h"

namespace ZJVL
{
	class Observer
	{
	public:
		virtual ~Observer(){};
		virtual void on_notify(Event &e) = 0;
	};
}

#endif