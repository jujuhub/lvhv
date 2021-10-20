/*
 * canlib.c
 *
 * This source file contains functions copied from
 * can-utils. Copyright (c) 2002-2007 Volkswagen
 * Group Electronic Research. All rights reserved.
 *
 * The original code may have been modified for our
 * purposes.
 *
 * See: https://github.com/linux-can/can-utils
 */

#include "canlib.h"
#include "lib.h"


const char hex_asc_upper[] = "0123456789ABCDEF";

/* CAN DLC to real data length conversion helpers */
static const unsigned char dlc2len[] = {0, 1, 2, 3, 4, 5, 6, 7,
                                        8, 12, 16, 20, 24, 32, 48, 64};

/* get data length from can_dlc with sanitized can_dlc */
unsigned char can_dlc2len(unsigned char can_dlc)
{
  return dlc2len[can_dlc & 0x0F];
}

static const unsigned char len2dlc[] = {0, 1, 2, 3, 4, 5, 6, 7, 8,
                                        9, 9, 9, 9,
                                        10, 10, 10, 10,
                                        11, 11, 11, 11,
                                        12, 12, 12, 12,
                                        13, 13, 13, 13, 13, 13, 13, 13,
                                        14, 14, 14, 14, 14, 14, 14, 14,
                                        14, 14, 14, 14, 14, 14, 14, 14,
                                        15, 15, 15, 15, 15, 15, 15, 15,
                                        15, 15, 15, 15, 15, 15, 15, 15};

/* map the sanitized data length to an appropriate data length code */
unsigned char can_len2dlc(unsigned char len)
{
  if (len > 64)
    return 0xF;

  return len2dlc[len];
}

unsigned char asc2nibble(char c)
{
  if ((c >= '0') && (c <= '9'))
    return c - '0';

  if ((c >= 'A') && (c <= 'F'))
    return c - 'A' + 10;

  if ((c >= 'a') && (c <= 'f'))
    return c - 'a' + 10;

  return 16; /* error */
}

int parse_canframe(char *cs, struct canfd_frame *cf)
{
  /* documentation see canlib.h */

  int i, idx, dlen, len;
  int maxdlen = CAN_MAX_DLEN;
  int ret = CAN_MTU;
  unsigned char tmp;

  len = strlen(cs);
  //printf("'%s' len %d\n", cs, len);

  memset(cf, 0, sizeof(*cf)); /* init CAN FD frame, e.g. LEN = 0 */

  if (len < 4)
    return 0;

  if (cs[3] == CANID_DELIM) // 3 digits
  {
    idx = 4;
    for (i = 0; i < 3; i++)
    {
      if ((tmp = asc2nibble(cs[i])) > 0x0F)
        return 0;
      cf->can_id |= (tmp << (2-i)*4);
    }
  } else if (cs[8] == CANID_DELIM) // 8 digits
  {
    idx = 9;
    for (i = 0; i < 8; i++)
    {
      if ((tmp = asc2nibble(cs[i])) > 0x0F)
        return 0;
      cf->can_id |= (tmp << (7-i)*4);
    }
    if (!(cf->can_id & CAN_ERR_FLAG)) // 8 digits but no error frame?
      cf->can_id |= CAN_EFF_FLAG; // then it is an extended frame
  } else
    return 0;

  if ((cs[idx] == 'R') || (cs[idx] == 'r')) // RTR frame
  {
    cf->can_id |= CAN_RTR_FLAG;

    /* check for optional DLC value for CAN 2.0B frames */
    if (cs[++idx] && (tmp = asc2nibble(cs[idx])) <= CAN_MAX_DLC)
      cf->len = tmp;

    return ret;
  }

  if (cs[idx] == CANID_DELIM) // CAN FD frame escape char '##'
  {
    maxdlen = CANFD_MAX_DLEN;
    ret = CANFD_MTU;

    /* CAN FD frame <canid>##<flags><data> */
    if ((tmp = asc2nibble(cs[idx+1])) > 0x0F)
      return 0;

    cf->flags = tmp;
    idx += 2;
  }

  for (i = 0, dlen = 0; i < maxdlen; i++)
  {
    if (cs[idx] == DATA_SEPARATOR) // skip (optional) separator
      idx++;

    if (idx >= len) // end of string => end of data
      break;

    if ((tmp = asc2nibble(cs[idx++])) > 0x0F)
      return 0;
    cf->data[i] = (tmp << 4);
    if ((tmp = asc2nibble(cs[idx++])) > 0x0F)
      return 0;
    cf->data[i] |= tmp;
    dlen++;
  }
  cf->len = dlen;

  return ret;
}

int cansend(char **cs)
{
  int s; // can raw socket
  int required_mtu;
  int mtu;
  int enable_canfd = 1;
  struct sockaddr_can addr;
  struct canfd_frame frame;
  struct ifreq ifr;

  /* check command line options */
//  if (argc != 3)
//  {
//    // print usage
//    return 1;
//  }

  /* parse CAN frame */
  required_mtu = parse_canframe(cs[2], &frame);
  if(!required_mtu)
  {
    fprintf(stderr, "\nWrong CAN frame format!\n\n");
    // print usage
    return 1;
  }

  /* open socket */
  if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
  {
    perror("socket");
    return 1;
  }

  strncpy(ifr.ifr_name, cs[1], IFNAMSIZ - 1);
  ifr.ifr_name[IFNAMSIZ - 1] = '\0';
  ifr.ifr_ifindex = if_nametoindex(ifr.ifr_name);
  if (!ifr.ifr_ifindex)
  {
    perror("if_nametoindex");
    return 1;
  }

  memset(&addr, 0, sizeof(addr));
  addr.can_family = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;

  if (required_mtu > (int)CAN_MTU)
  {
    /* check if frame fits into the CAN netdevice */
    if (ioctl(s, SIOCGIFMTU, &ifr) < 0)
    {
      perror ("SIOCGIFMTU");
      return 1;
    }
    mtu = ifr.ifr_mtu;

    if (mtu != CANFD_MTU)
    {
      printf("CAN interface is not CAN FD capable - sorry.\n");
      return 1;
    }

    /* interface is ok - try to switch the socket into CAN FD mode */
    if (setsockopt(s, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable_canfd, 
    sizeof(enable_canfd)))
    {
      printf("error when enabling CAN FD support\n");
      return 1;
    }

    /* ensure discrete CAN FD length values 0..8, 12, 16, 20, 24, 32, 
    64 */
    frame.len = can_dlc2len(can_len2dlc(frame.len));
  }

  /* disable default receive filter on this RAW socket */
  // This is obsolete as we do not read from the socket at all, but for
  // this reason we can remove the receive list in the Kernal to save a
  // little (really a very little!) CPU usage.
  setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

  if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    perror("bind");
    return 1;
  }

  /* send frame */
  if (write(s, &frame, required_mtu) != required_mtu)
  {
    perror("write");
    return 1;
  }

  close(s);

  return 0;
}

int canread(char *rcv_msg)
{
  int s;
  struct sockaddr_can addr;
  struct ifreq ifr;
  struct canfd_frame frame;

  // open can socket
  if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
  {
    perror("socket");
    return -1;
  }

  strcpy(ifr.ifr_name, "can0"); // hardcoded device name
  ioctl(s, SIOCGIFINDEX, &ifr);

  memset(&addr, 0, sizeof(addr));
  addr.can_family = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;

  if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    perror("bind");
    return -1;
  }


  // read can msg
  int nbytes = 0;
  int read_attempts = 0;
  fd_set readfds;
  struct timeval tv;
  int selret;

  char rcvID[5];
  char rcv_tmp[3];
  //char *rcv_msg; rcv_msg = (char *) malloc(256);
  //char *empty; empty = (char *) malloc(256);

  bool msgReceived = false;
  while (!msgReceived)
  {
    if (read_attempts == MAX_READ_ATTEMPTS)
    {
      fprintf(stderr, "\n @@@ TOO MANY READ ATTEMPTS!! Moving on..\n\n");
      close(s);
      return -2;
    }

    // prepare for select()
    FD_ZERO(&readfds);
    FD_SET(s, &readfds);
    // wait for x [s] and y [us]
    tv.tv_sec = 0;
    tv.tv_usec = 1000;

    // monitor socket s, for a time tv
    selret = select(s+1, &readfds, NULL, NULL, &tv);
    if (selret == -1) { perror("select() returned -1"); }
    else if (selret)
    {
      printf(" Data is available now. Reading data...\n"); //debug
      nbytes = read(s, &frame, sizeof(struct can_frame));
    }
    else
    {
      printf(" @@@ No data found. Trying again...\n");
      //read_attempts++;
      continue;
    }

    //nbytes = read(s, &frame, sizeof(struct can_frame));

    if (nbytes <= 0)
    {
      perror("canread");
      read_attempts++;
      continue;
    }
    else
    { // put together the msg
      sprintf(rcvID, "%03X%c", frame.can_id, '#');
      //sprintf(rcvID, "%03X%c", 0x0D0, '#'); //test
      rcvID[4] = '\0';
      strcpy(rcv_msg, rcvID);
      unsigned int frlen = frame.len;
      //unsigned int frlen = 8; //test
      for (int i = 0; i < frlen; i++)
      {
        sprintf(rcv_tmp, "%02X", frame.data[i]);
        //sprintf(rcv_tmp, "%02X", 0x11); //test
        strcat(rcv_msg, rcv_tmp);
      }
      rcv_msg[20] = '\0';

      msgReceived = true;
      //close(s);
    }

    //return 1;
  } //end while loop

  // print received message (above return needs to be commented out)
  //printf("0x%03X [%d] ", frame.can_id, CAN_DLC);
  //for (int i = 0; i < CAN_DLC; i++) { printf("%02X ", frame.data[i]); }
  //printf("\n");

  close(s);

  return 1;
}

int decodeCANmsg(struct SlowControlsData *sc, char *rcv_msg)
{
  char msgID[4];
  strncpy(msgID, rcv_msg, 3);
  msgID[3] = '\0';

  if (strcmp(msgID, "321") == 0)
  { // hum & temp
    decodeRHT(sc, rcv_msg);
  }
  else if ((strcmp(msgID, "220") == 0) || (strcmp(msgID, "3DA") == 0))
  { // low voltage
    decodeLV(sc, rcv_msg);
  }
  else if (strcmp(msgID, "035") == 0)
  { // high voltage
    decodeHV(sc, rcv_msg);
  }
  else if (strcmp(msgID, "0D0") == 0)
  { // photodiode
    decodePhotodiode(sc, rcv_msg);
  }
  else if ((strcmp(msgID, "0CB") == 0) || (strcmp(msgID, "0FE") == 0))
  { // trigger bd
    decodeTrigBd(sc, rcv_msg);
  }
  else { printf(" @@@ unrecognized msg ID: %s !!!\n", msgID); return -3; }

  return 1;
}

int readcanlog(char *fname)
{
  bool new_msg = false;

  FILE *fp = fopen(fname, "r");
  char line[1024] = "";
  char c;
  int len = 0;

  struct stat stat_record;
  if (stat(fname, &stat_record))
  {
    printf("%s", strerror(errno));
  }
  else if (stat_record.st_size <= 1)
  {
    printf(" %s is empty!\n", fname);
    return -99;
  }

  if (fp == NULL)
  {
    printf("  !!! Could not open file!\n");
    return -99;
  }

  fseek(fp, -1, SEEK_END);
  c = fgetc(fp);

  while (c == '\n')
  {
    fseek(fp, -2, SEEK_CUR);
    c = fgetc(fp);
  }

  while (c != '\n')
  {
    fseek(fp, -2, SEEK_CUR);
    ++len;
    c = fgetc(fp);
  }

  fseek(fp, 0, SEEK_CUR);

  if (fgets(line, len+1, fp) != NULL) puts(line); 
  else
  {
    printf("  !!! Error reading line!\n");
    return -99;
  }
  fclose(fp);

  // extract CAN msg
  int l = 0;
  char decdate[15] = "";
  char msgID[5] = ""; // uint32_t
  char canmsg[20] = "";

  for (l = 0; l < strlen(line); l++)
  {
    // extract date
    decdate[l] = line[l];

    // extract msg ID
    // extract can msg
  }
  
//  switch(msgID):

  return 0;

}
