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
// - format of a cron string : "S M H d m w [Y] cmd" - (Year is optional; default limit values of the year: +/- 8 years relative to the current year).

int main(int argc, char **argv) {
	std::vector<std::string> crontab = {
		"* */20 * mon,tue,thu,fri * * reqular",
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
