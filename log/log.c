/*
 * log.c
 *
 *  Created on: Nov 15, 2015
 *      Author: vty
 */
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "log.h"

//the level.
int v_cwf_log_lvl_ = V_CWF_LOG_D;
void (*v_cwf_log_f_)(const char* file, int line, int lvl, const char* fmt,
		va_list args)=v_cwf_log_print;

void v_cwf_log_(const char* file, int line, int lvl, const char* fmt, ...) {
	if (lvl < v_cwf_log_lvl_) {
		return;
	}
	if (v_cwf_log_f_) {
		va_list args;
		va_start(args, fmt);
		v_cwf_log_f_(file, line, lvl, fmt, args);
		va_end(args);
	} else {
		printf("v_cwf_log_f_ is not setted\n");
	}
}

void v_cwf_log_print(const char* file, int line, int lvl, const char* fmt,
		va_list args) {
	size_t clen = strlen(file) + 30 + strlen(fmt);
	char *buf = malloc(sizeof(char) * clen);
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	size_t blen = strftime(buf, clen, "%Y/%m/%d %H:%M:%S ", timeinfo);
	switch (lvl) {
	case V_CWF_LOG_D:
		blen += sprintf(buf + blen, "%s:%d D %s\n", file, line, fmt);
		break;
	case V_CWF_LOG_I:
		blen += sprintf(buf + blen, "%s:%d I %s\n", file, line, fmt);
		break;
	case V_CWF_LOG_W:
		blen += sprintf(buf + blen, "%s:%d W %s\n", file, line, fmt);
		break;
	case V_CWF_LOG_E:
		blen += sprintf(buf + blen, "%s:%d E %s\n", file, line, fmt);
		break;
	default:
		blen += sprintf(buf + blen, "%s:%d U %s\n", file, line, fmt);
		break;
	}
	buf[blen] = 0;
	vprintf(buf, args);
	free(buf);
}
