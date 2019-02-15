#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <signal.h>

#include <iostream>
#include <chrono>
#include <thread>


// Daemon name
static char		daemonName[256]	= {};
// Daemon run condition
static bool		runDaemon	= true;

// PID holder file descriptor
static int		pidFileDesc	= -1;
// PID holder file directory name
static const char*	pidFileDir	= "./daemon/";
// PID holder file name
static const char*	pidFileName	= "example-d.pidfile";

// "/dev/null" file descriptor (all writed in it goes to global universe void and loses forever)
static int		nullFile	= -1;


// Deamonization function
void deamonizer() {

	// Fork from parent
	pid_t pID = fork();

	// Check if pID is greater than 0
	// which means we are in child process
	if (pID > 0) {

                exit(EXIT_SUCCESS);

	// If pID is less than 0 - we are sucks
	} else if (pID < 0) {

                exit(EXIT_FAILURE);

        }        

	// Create new sID
	pid_t sID = setsid();

	// Check if session id is greater than 0
        if (sID < 0) {

                exit(EXIT_FAILURE);

        }

	// Ignore child signals
	signal(SIGCHLD,	SIG_IGN);
	// Ignore terminal lost signal
	signal(SIGHUP,	SIG_IGN);

	// Fork from parent (second time)
	// This prevents child #1 (after fork() #1)
	// from aquiring terminal
	// which may lead to "zombie" process
        pID = fork();

	// Check if pID is greater than 0
	// which means we are in child process
	if (pID > 0) {

                exit(EXIT_SUCCESS);

	// If pID is less than 0 - we are sucks
	} else if (pID < 0) {

                exit(EXIT_FAILURE);

        }

/*
	// Open system logs for the child process
	openlog(daemonName, LOG_NOWAIT | LOG_PID, LOG_USER);
	syslog(LOG_NOTICE, "Successfully forked %s", daemonName);
*/

	// Write PID file
	if (pidFileName != nullptr) {

		// Create daemon directory for daemon runtime data
		if (mkdir(pidFileDir, 0777)	<  0	&&
		    errno			!= EEXIST) {

/*
			// Log failure and exit
			syslog(LOG_ERR, "Could not create %s directory", pidFileDir);
			closelog();
*/

			exit(EXIT_FAILURE);

		}

		// Change working directory to root
		if (chdir(pidFileDir) < 0) {

/*
			// Log failure and exit
			syslog(LOG_ERR, "Could not change working directory to %s", pidFileDir);
			closelog();
*/

			exit(EXIT_FAILURE);

		}

		// Open PID holder file (read/write + create anyway)
		pidFileDesc = open(pidFileName, O_RDWR | O_CREAT, 0640);

		// Check PID holder file descriptor
		if (pidFileDesc < 0) {

/*
			// Log failure and exit
			syslog(LOG_ERR, "Could not open PID holder file %s", pidFileName);
			closelog();
*/

			exit(EXIT_FAILURE);

		}

		// Lock PID file descriptor
		if (lockf(pidFileDesc, F_TLOCK, 0) < 0) {

/*
			// Log failure and exit
			syslog(LOG_ERR, "Could not lock PID holder file %s", pidFileName);
			closelog();
*/

			// Close PID file
			close(pidFileDesc);

			exit(EXIT_FAILURE);

		}

		// PID string holder
		char pidString[16];

		// Create PID string from PID
		sprintf(pidString, "%d\n", getpid());

		// Write PID to file
		write(pidFileDesc, pidString, strlen(pidString));

	}

	// Change file mask
        umask(0);

	// Change working directory to root
        if (chdir("/") < 0) {

/*
		// Log failure and exit
		syslog(LOG_ERR, "Could not change working directory to /");
		closelog();
*/

		exit(EXIT_FAILURE);

	}

	// Reopen stdin (fd 0), stdout (fd 1) and stderr(fd 2)
	// because we are not gonnna use them
	if (nullFile = open("/dev/null", O_RDWR)) {

		stdin	= fopen("/dev/null", "r");
		stdout	= fopen("/dev/null", "w+");
		stderr	= fopen("/dev/null", "w+");
		close(nullFile);

	} else {

/*
		// Log failure and exit
		syslog(LOG_ERR, "Could not open /dev/null... Does it even exists !?");
		closelog();
*/

		exit(EXIT_FAILURE);

	}

}

// Signal handler
void signal_handler_func(int sigNumber) {

	// Check signal type
	switch (sigNumber) {

	// Ctrl + C signal
	case SIGINT:

		// Log failure and exit
		syslog(LOG_NOTICE, "%s got SIGINT", daemonName);
		closelog();

		// Check if PID holder file is still opened
		if (pidFileDesc > 0) {

			// Unlock it
			lockf(pidFileDesc, F_ULOCK, 0);
			// And close
			close(pidFileDesc);

		}

		// Stop the daemon
		runDaemon = false;

		// Restore default signal handler for Ctrl + C signal
		signal(SIGINT, SIG_DFL);

		break;
	
	// Default handling
	default:
		// HOW THE HACK DID YOU DO THIS?!
		syslog(LOG_NOTICE, "%s got unhandled signal", daemonName);
		break;


	}

}

// Main deamon actions
void daemon_func() {

	// Say that daemon is up & runnin` in da house
	syslog(LOG_NOTICE, "Alive %s", daemonName);

	//
	//	TODO: daemon actions
	//

}


// Entry point
int main(int argc, const char* argv[]) {

	// Deamonize process
        deamonizer();

	//
	//	TODO: daemon init
	//

	// Copy daemon name to variable
	strncpy(daemonName, basename(argv[0]), sizeof(daemonName) - 1);

	// Open system logs for the child process
	openlog(daemonName, LOG_NOWAIT | LOG_PID | LOG_CONS, LOG_USER);
	syslog(LOG_NOTICE, "%s successfully started", daemonName);

	// Register signal handler for Ctrl + C signal
	signal(SIGINT, signal_handler_func);
	// Register signal handler for terminal lost signal
	signal(SIGHUP, signal_handler_func);

        // Main loop
        while (runDaemon) {

		// Do daemonish things :D
		daemon_func();

		// Sleep a bit
		std::this_thread::sleep_for(std::chrono::seconds(30));

        }

	//
	//	TODO: daemon release
	//

	// Close system logs for the child process
	syslog(LOG_NOTICE, "Stopping %s", daemonName);
	closelog();

	// Daemon done
	exit(EXIT_SUCCESS);

} 
