#include "KqueueManager.hpp"

int KqueueManager::kq = -1;
bool KqueueManager::initialized = false;

void KqueueManager::initialize() {
	if (!initialized) {
		kq = kqueue();
		if (kq == -1) {
			throw std::runtime_error("Failed to create kqueue");
		}
		initialized = true;
	}
}

void KqueueManager::registerEvent(int fd, int filter, short flags) {
	struct kevent change;
	EV_SET(&change, fd, filter, flags, 0, 0, nullptr);
	if (kevent(kq, &change, 1, nullptr, 0, nullptr) == -1) {
		std::cerr << "[ERROR] Failed to register event for fd " << fd << ": " << strerror(errno) << std::endl;
	}
}

void KqueueManager::deregisterEvent(int fd) {
	struct kevent change;

	EV_SET(&change, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	if (kevent(kq, &change, 1, NULL, 0, NULL) == -1) {
		std::cerr << "[ERROR] Failed to remove read event for fd " << fd << ": " << strerror(errno) << std::endl;
	}
}

int KqueueManager::getKqFd() {
	return kq;
}
