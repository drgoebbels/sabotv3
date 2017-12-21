#include "log.h"
#include "general.h"
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

static FILE *log_file;

static void log_format(const char *format, const char *name, va_list args);
static void log_format_errno(const char *format, const char *name, va_list args);
static int log_create_directory(void);

int log_init_name(const char *name) {
	int result;
	char namebuf[64];
	FILE *file;
	
	result = log_create_directory();
	if(result) {
		fputs("Warning: Failed to create log directory: " LOG_PATH " . Failling back on stdout.\n", stderr);
		return -1;
	}
	sprintf(namebuf, LOG_PATH "/%s", name);
	file = fopen(namebuf, "a");	
	if(!file) {
		perror("Failed to Open file");
		fprintf(stderr, "Warning: Failed to open log file %s for writing. Failling back on stdout.\n", name);
		log_file = stdout;
		return -1;
	}
	log_file = file;
	return 0;
}

void log_init(FILE *file) {
	log_file = file;
}

void log_info(const char *format, ...) {
	va_list args;
	va_start(args, format);
	log_format(format, "INFO", args);
	va_end(args);
}

void log_warn(const char *format, ...) {
	va_list args;
	va_start(args, format);
	log_format(format, "WARN", args);
	va_end(args);
}

void log_error(const char *format, ...) {
	va_list args;
	va_start(args, format);
	log_format(format, "ERROR", args);
	va_end(args);
}

void log_error_errno(const char *format, ...) {
	va_list args;

	va_start(args, format);
	log_format(format, "ERROR", args);
	va_end(args);
}

void log_format(const char *format, const char *name, va_list args) {
	time_t now;
	char buffer[64];
	struct tm *local_time;

	now = time(NULL);
	local_time = localtime(&now);
	strftime(buffer, 64, "%Y-%m-%dT%H:%M:%S%z", local_time);
	fprintf(log_file, "*%s*\t%s\t", name, buffer);
	vfprintf(log_file, format, args);
	fputc('\n', log_file);
}

void log_format_errno(const char *format, const char *name, va_list args) {
	time_t now;
	int err;
	char buffer[ERR_BUF_SIZE + 1];
	struct tm *local_time;

	now = time(NULL);
	local_time = localtime(&now);
	strftime(buffer, 64, "%Y-%m-%dT%H:%M:%S%z", local_time);
	fprintf(log_file, "*%s*\t%s\t", name, buffer);
	vfprintf(log_file, format, args);
	fputs(" : ", log_file);
	buffer[ERR_BUF_SIZE + 1] = '\0';
	while((err = strerror_r(errno, buffer, ERR_BUF_SIZE)) != 0) {
		fputs(buffer, log_file);
	}
	fputc('\n', log_file);
}

void log_end(void) {
		if(log_file != stdout && log_file != stderr) {
			fclose(log_file);
		}
}

int log_create_directory(void) {
	int result;
	struct stat st;
	
	result = stat(LOG_PATH, &st);
	if(result) {
		if(errno == ENOENT) {
			result = mkdir(LOG_PATH, S_IRUSR | S_IWUSR | S_IXUSR);
			if(result) {
				perror("Error while creating log directory - occured during mkdir()");
				return -1;
			}
		}
		else {
			perror("Error while creating log directory - occured during stat()");
			return -1;
		}
	}
	return 0;
}

