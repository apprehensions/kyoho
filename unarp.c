#include <archive.h>
#include <archive_entry.h>
#include <linux/limits.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIR_MODE 0755

void
die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (fmt[0] && fmt[strlen(fmt)-1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}

	exit(1);
}

static void
extract_archive_file(struct archive *ar, struct archive *aw)
{
	int r;
	const void *buff;
	size_t size;
	int64_t offset;

	while ((r = archive_read_data_block(ar, &buff, &size, &offset)) != ARCHIVE_EOF) {
		if (r != ARCHIVE_OK)
			die("%s: %s", "archive_read_data_block()", archive_error_string(ar));
		if (archive_write_data_block(aw, buff, size, offset) != ARCHIVE_OK)
			die("%s: %s", "archive_write_data_block()", archive_error_string(aw));
	}
}

int
main(int argc, char *argv[]) {
	struct archive *a;
	struct archive *ext;
	struct archive_entry *entry;
	int r;

	if (argc != 3) {
		fprintf(stderr, "usage: unarp archive directory\n");
		exit(1);
	}

	if (mkdir(argv[2], DIR_MODE) < 0 && errno != EEXIST)
		die("mkdir:");

	a = archive_read_new();
	ext = archive_write_disk_new();

	archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_PERM);
	archive_read_support_format_zip(a);

	if ((r = archive_read_open_filename(a, argv[1], 10240)))
		die("%s: %s", "archive_read_open_filename()", archive_error_string(a));

	while ((r = archive_read_next_header(a, &entry)) != ARCHIVE_EOF) {
		if (r != ARCHIVE_OK)
			die("%s: %s", "archive_read_next_header()", archive_error_string(a));

		const char *file_path = archive_entry_pathname(entry);
		if (file_path[0] == '/' && file_path[1] == '\0') {
			continue;
		}
		
		char full_path[strlen(file_path) + strlen(argv[2]) + 2];
		snprintf(full_path, sizeof(full_path), "%s/%s", argv[2], file_path);

		printf("extracting %s\n", full_path);

		if (file_path[0] == '/') {
			if (mkdir(full_path, DIR_MODE) < 0 && errno != EEXIST)
				die("mkdir:");
			continue;
		}

		archive_entry_set_pathname(entry, full_path);
		if (archive_write_header(ext, entry) != ARCHIVE_OK)
			die("%s: %s", "archive_write_header()", archive_error_string(a));

		extract_archive_file(a, ext);

		r = archive_write_finish_entry(ext);
		if (r != ARCHIVE_OK)
			die("%s: %s", "archive_write_finish_entry()", archive_error_string(a));
	}

	archive_read_close(a);
	archive_read_free(a);
	archive_write_close(ext);
	archive_write_free(ext);

	return EXIT_SUCCESS;
}
