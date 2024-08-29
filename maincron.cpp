#define _X_OPEN_SOURCE_EXTENDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "datetime/datetime.h"
#include "par_easycurl.h"
#include "simpleini/SimpleIni.h"

#include <iostream>
#include <string>
#include <vector>

#define WORDSURL "https://raw.githubusercontent.com/ppiecuch/shared-assets/master/words.txt"
#define LOCALCACHE "/tmp/words-memo.txt"
#define APPVERSION "0.2"

#define f_ssprintf(...) \
	({ int _ss_size = snprintf(0, 0, ##__VA_ARGS__);    \
    char *_ss_ret = (char*)alloca(_ss_size+1);          \
    snprintf(_ss_ret, _ss_size+1, ##__VA_ARGS__);       \
    _ss_ret; })

using namespace datetime_utils::crontab;

// Reference:
// ----------
// - https://github.com/peychart/croncpp/blob/main/main.cpp

int main(int argc, char **argv) {
	std::vector<std::string> crontab = {
		"5-55 55 * * apr * myCmd1",
		"* 25-30 * * * * myCmd2",
		"*/10 55 * * apr * 2021,2025 myCmd3",
		"* 05-30 * * * 2-3 * myCmd4",
		"* * * 31 * * myCmd5"
	};

	time_t Now(time(NULL));
	cron c;

	for (int i(0); i < crontab.size(); i++) {
		c.clear() = crontab[i];
		time_t rawtime = c.next_date(Now);

		char buffer[80];
		strftime(buffer, 80, "%Y/%m/%d %H:%M:%S", localtime(&rawtime));
		std::cout << "The job \"" << c.expression() << "\" lanched at: " << rawtime << " (" << buffer << "), in " << rawtime - Now << " sec. - \"" << crontab[i] << "\"" << std::endl;
	}
}

// vim: expandtab tabstop=5 softtabstop=5 shiftwidth=5
