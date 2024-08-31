#define _X_OPEN_SOURCE_EXTENDED

#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "datetime/datetime.h"
#include "par_easycurl.h"
#include "simpleini/SimpleIni.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#define APPVERSION "0.4"
#define APPNAME "main(v" APPVERSION ")"

#define PRINTCMD "my-word-memo"
#define PRINTARG "-p -1\x00"

#define f_ssprintf(...) \
	({ int _ss_size = snprintf(0, 0, ##__VA_ARGS__);    \
    char *_ss_ret = (char*)alloca(_ss_size+1);          \
    snprintf(_ss_ret, _ss_size+1, ##__VA_ARGS__);       \
    _ss_ret; })

using namespace datetime_utils::crontab;

// Reference:
// ----------
// - https://github.com/peychart/croncpp/blob/main/main.cpp
// - format of a cron string : "S M H d m w [Y] cmd" - (Year is optional; default limit values of the year: +/- 8 years relative to the current year).

extern char **environ;

int run_cmd(const char *cmd, char *const *args) {
	pid_t pid;
	int status = posix_spawnp(&pid, cmd, nullptr, nullptr, args, environ);
	if (status == 0) {
		if (waitpid(pid, &status, 0) != -1) {
			if (WIFEXITED(status)) {
				return WEXITSTATUS(status);
			} else {
				if (WIFSIGNALED(status)) {
					if (WCOREDUMP(status)) {
						fprintf(stderr, APPNAME ": the child process produced a core dump\n");
					}
					if (WTERMSIG(status)) {
						fprintf(stderr, APPNAME ": the child process was terminated\n");
					}
				}
			}
		} else {
			fprintf(stderr, APPNAME ": waitpid error\n");
		}
	} else {
		fprintf(stderr, APPNAME ": posix_spawn: %s\n", strerror(status));
	}
	return -1;
}

std::vector<std::string> read_file(const char *filename) {
	std::vector<std::string> r;
	std::ifstream file(filename);
	if (file.is_open()) {
		std::string line;
		while (std::getline(file, line)) {
			if (!line.empty())
				r.push_back(line);
		}
		file.close();
	}
	return r;
}

int main(int argc, char **argv) {
	std::vector<std::string> crontab = {
		"* */15 9-17 * * mon,tue,thu,fri daily",
		"* */30 10-18 * * sat weekend1",
		"* */20 15-17 * * sun weekend2",
	};

	for (;;) {
		time_t Now(time(NULL));
		cron c;

		unsigned pause = 0;

		for (int i(0); i < crontab.size(); i++) {
			c.clear() = crontab[i];
			time_t rawtime = c.next_date(Now);

			char buffer[80];
			strftime(buffer, 80, "%Y/%m/%d %H:%M:%S", localtime(&rawtime));
			int schedule = rawtime - Now;
			std::cout << "The job \"" << c.expression() << "\" lanched at: " << rawtime << " (" << buffer << "), in " << schedule << " sec. - \"" << crontab[i] << "\"" << std::endl;
			if (schedule <= 1 || (Now - c.previous_date(Now)) <= 1) {
				run_cmd(PRINTCMD, (char *const *)PRINTARG);
			} else if (!pause || schedule < pause) {
				pause = schedule;
			}
		}

		std::cout << "Waiting for " << pause << " sec." << std::endl;
		sleep(pause - 1);
	}
}

// vim: expandtab tabstop=5 softtabstop=5 shiftwidth=5
