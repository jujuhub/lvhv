/*
 * canlib.h
 *
 * This header file contains functions copied from
 * can-utils. Copyright (c) 2002-2007 Volkswagen
 * Group Electronic Research. All rights reserved.
 *
 * The original code may have modified for our
 * purposes.
 *
 */

#ifndef CANLIB_H
#define CANLIB_H

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <linux/can.h>
#include <linux/can/error.h>
#include <linux/can/raw.h>

#include <canlib.h>

#define CANID_DELIM '#'
#define DATA_SEPARATOR '.'
#define CAN_DLC 8

#define MAX_READ_ATTEMPTS 50
#define CANLOG "canlog.txt"


/* CAN DLC to real data length conversion helpers especially for CAN FD 
*/
/* get data length from can_dlc with sanitized can_dlc */
unsigned char can_dlc2len(unsigned char can_dlc);

/* map the sanitized data length to an appropriate data length code */
unsigned char can_len2dlc(unsigned char len);

unsigned char asc2nibble(char c);
/*
 * Returns the decimal value of a given ASCII hex character
 *
 * While 0..9, a..f, A..F are valid ASCII hex characters.
 * On invalid characters the value 16 is returned for error handling.
 */

int parse_canframe(char *cs, struct canfd_frame *cf);

int cansend(char **cs);

int canread();

int readcanlog(char *fpath);

#endif
