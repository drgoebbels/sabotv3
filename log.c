#include "log.h"
#include "general.h"
#include <stdarg.h>
#include <time.h>

static FILE *log_file;

static void log_format(const char *format, const char *name, va_list args);

void log_init_name(const char *name) {
	FILE *f = fopen(name, "a");	
	if(!f) {
			perror("Failed to Open file");
			fprintf(stderr, "Warning: Failed to open log file %s for writing. Failling back on stdout.\n", name);
			log_file = stdout;
	}
	log_file = f;
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

void log_format(const char *format, const char *name, va_list args) {
	time_t now;
	char buffer[64];
	struct tm *local_time;

	now = time(NULL);
	local_time = localtime(&now);
	strftime(buffer, 64, "%Y-%m-%dT%H:%M:%S%z", local_time);

	fprintf(log_file, "*%s* %s\t", name, buffer);
	vfprintf(log_file, format, args);
	fputc('\n', log_file);
}

void log_end(void) {
		if(log_file != stdout && log_file != stderr) {
				fclose(log_file);
		}
}

