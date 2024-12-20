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
		int kq;

	public:
		KqueueManager();

		void initializeKq();
		void registerEvent(int fd, int filter, short flags);
		void deregisterEvent(int fd);
		int getKqFd();
};