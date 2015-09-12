/* vim: tabstop=4 shiftwidth=4 noexpandtab
 * This file is part of ToaruOS and is released under the terms
 * of the NCSA / University of Illinois License - see LICENSE.md
 * Copyright (C) 2011-2014 Kevin Lange
 *
 * Kernel printf implementation
 *
 * Simple, painfully lacking, implementation of printf(),
 * for the kernel of all things.
 */
#include <types.h>
#include <va_list.h>

/*
 * Integer to string
 */
static void print_dec(unsigned long long value, unsigned long long width, char * buf, long long * ptr ) {
	unsigned long long n_width = 1;
	unsigned long long i = 9;
	while (value > i && i < UINT64_MAX) {
		n_width += 1;
		i *= 10;
		i += 9;
	}

	long long printed = 0;
	while (n_width + printed < width) {
		buf[*ptr] = '0';
		*ptr += 1;
		printed += 1;
	}

	i = n_width;
	while (i > 0) {
		unsigned long long n = value / 10;
		int r = value % 10;
		buf[*ptr + i - 1] = r + '0';
		i--;
		value = n;
	}
	*ptr += n_width;
}

/*
 * Hexadecimal to string
 */
static void print_hex(unsigned long long value, unsigned long long width, char * buf, long long * ptr) {
	long long i = width;

	if (i == 0) i = 16;

	unsigned long long n_width = 1;
	unsigned long long j = 0x0F;
	while (value > j && j < UINT64_MAX) {
		n_width += 1;
		j *= 0x10;
		j += 0x0F;
	}

	while (i > (long long)n_width) {
		buf[*ptr] = '0';
		*ptr += 1;
		i--;
	}

	i = (long long)n_width;
	while (i-- > 0) {
		buf[*ptr] = "0123456789abcdef"[(value>>(i*4))&0xF];
		*ptr += + 1;
	}
}

/*
 * vasprintf()
 */
size_t vasprintf(char * buf, const char * fmt, va_list args) {
	long long i = 0;
	char * s;
	char * b = buf;
	for (const char *f = fmt; *f; f++) {
		if (*f != '%') {
			*b++ = *f;
			continue;
		}
		++f;
		unsigned int arg_width = 0;
		while (*f >= '0' && *f <= '9') {
			arg_width *= 10;
			arg_width += *f - '0';
			++f;
		}
		/* fmt[i] == '%' */
		switch (*f) {
			case 's': /* String pointer -> String */
				s = (char *)va_arg(args, char *);
				if (s == NULL) {
					s = "(null)";
				}
				while (*s) {
					*b++ = *s++;
				}
				break;
			case 'c': /* Single character */
				*b++ = (char)va_arg(args, int);
				break;
			case 'x': /* Hexadecimal number */
				i = b - buf;
				print_hex((unsigned long long)va_arg(args, unsigned long long), arg_width, buf, &i);
				b = buf + i;
				break;
			case 'd': /* Decimal number */
				i = b - buf;
				print_dec((unsigned long long)va_arg(args, unsigned long long), arg_width, buf, &i);
				b = buf + i;
				break;
			case '%': /* Escape */
				*b++ = '%';
				break;
			default: /* Nothing at all, just dump it */
				*b++ = *f;
				break;
		}
	}
	/* Ensure the buffer ends in a null */
	*b = '\0';
	return b - buf;

}

#if 0
int fprintf(fs_node_t * device, char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char buffer[1024];
	vasprintf(buffer, fmt, args);
	va_end(args);

	return write_fs(device, 0, strlen(buffer), (uint8_t *)buffer);
}
#endif

extern void writech(unsigned char c);

int printf(char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	char buffer[1024];
	vasprintf(buffer, fmt, args);
	va_end(args);

	char * c = buffer;
	while (*c) {
		writech(*c);
		c++;
	}

	return 0;
}

int sprintf(char * buf, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int out = vasprintf(buf, fmt, args);
	va_end(args);
	return out;
}
