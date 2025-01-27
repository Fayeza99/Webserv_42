#include <sys/event.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <signal.h>

void infinity() {
    while (true) {
        ; // Infinite loop
    }
}

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        infinity();
    } else if (pid > 0) {
        // Parent process
        int kq = kqueue(); // Create a kqueue
        if (kq == -1) {
            std::cerr << "Failed to create kqueue\n";
            return 1;
        }

        struct kevent event;
        struct kevent triggered_event;

        // Register the child process with kqueue
        EV_SET(&event, pid, EVFILT_PROC, EV_ADD | EV_ENABLE, NOTE_EXIT, 0, nullptr);
        if (kevent(kq, &event, 1, nullptr, 0, nullptr) == -1) {
            std::cerr << "Failed to register process with kqueue\n";
            return 1;
        }

        // Set a timeout of 5 seconds
        struct timespec timeout;
        timeout.tv_sec = 5; 
        timeout.tv_nsec = 0;

        int nev = kevent(kq, nullptr, 0, &triggered_event, 1, &timeout);
        if (nev == -1) {
            std::cerr << "Error in kevent\n";
            return 1;
        } else if (nev == 0) {
            // Timeout occurred
            std::cout << "timeout\n";
            kill(pid, SIGKILL); // Kill the child process
        } else {
            // Child process exited
            std::cout << "finished\n";
        }
		int status;
		waitpid(pid, &status, 0); // Wait for the child to terminate

        if (WIFEXITED(status)) {
            std::cout << "Child exited normally with code: " << WEXITSTATUS(status) << std::endl;
        } else if (WIFSIGNALED(status)) {
            std::cout << "Child was terminated by signal: " << WTERMSIG(status) << std::endl;
        }
        close(kq); // Clean up
    } else {
        std::cerr << "Failed to fork\n";
        return 1;
    }

    return 0;
}
