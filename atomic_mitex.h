#pragma once
#include <atomic>

class atomic_mutex {
		std::atomic_flag flag = ATOMIC_FLAG_INIT;
	public:
		/*atomic_mutex ():
		{
			;
		}*/

		void lock()
		{
			while (flag.test_and_set(std::memory_order_acquire));
		}

		void unlock()
		{
			flag.clear(std::memory_order_release);
		}
};