#pragma once

#include <sys/event.h>
#include <exception>
#include <sys/types.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

class KqueueManager {
	private:
		static int kq;
		static bool initialized;

		KqueueManager() = delete;

	public:
		static void initialize();
		static void registerEvent(int fd, int filter, short flags);
		static void deregisterEvent(int fd);
		static int getKqFd();
};