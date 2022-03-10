/*
 * lib.c
 *
 * purpose
 *    contains functions used in lvhv_routine.c
 */

#include "lib.h"
#include "canlib.h"

bool isValidInput(char *usr_inp)
{
  bool valid = true;
  int i, slen;
  slen = strlen(usr_inp);

  if (slen == 1 || usr_inp[0] == '\n') // if 'enter' key was hit
  {
    printf("What was that?\n");
    valid = false;
    return valid;
  }

  if (slen > 10) // this doesn't occur bc of fgets
  {
    printf("Your response is too long (> 10).\n");
    valid = false;
    return valid;
  }

  for (i = 0; i < slen; i++)
  {
    if (isalpha(usr_inp[i]) || usr_inp[i] == '\n') { continue; }
    else { valid = false; }
  }

  return valid;
}

int checkYN(char *usr_inp)
{
  if (usr_inp[0] == 'Y' || usr_inp[0] == 'y') return 1;
  if (usr_inp[0] == 'N' || usr_inp[0] == 'n') return 0;
  else return 3;
}

float readPrevHV(char *fname)
{
  FILE *fp = fopen(fname, "r");
  char line[1024]= "";
  char c;
  int len = 0;

  struct stat stat_record;
  if (stat(fname, &stat_record))
  {
    printf("%s", strerror(errno));
  }
  else if (stat_record.st_size <= 1)
  {
    printf(" %s is empty! Returning 0.\n", fname);
    return 0.;
  }

  if (fp == NULL)
  {
    printf("  !!! Could not open file!\n");
    return -999.;
  }

  fseek(fp, -1, SEEK_END);
  c = fgetc(fp);

  while (c == '\n')
  {
    fseek(fp, -2, SEEK_CUR);
    c = fgetc(fp);
//    printf("DEBUG:  while[1] c = %c (%d)\n", c, c);
  }

  while (c != '\n')
  {
    fseek(fp, -2, SEEK_CUR);
    ++len;
    c = fgetc(fp);
//    printf("DEBUG:  while[2] c = %c (%d)\n", c, c);
  }

  fseek(fp, 0, SEEK_CUR);

  if (fgets(line, len+1, fp) != NULL) puts(line);
  else printf("  !!! Error!\n");
  fclose(fp);

  // extract previously set voltage
  int i = 0;
  char vstr[15] = "";
  float vprev = 0.;

  for (i = 0; i < strlen(line); i++)
  {
    if (line[i] == ' ' && i != 0)
    {
      vstr[i] = '\0';
      break;
    }
    else
    {
      vstr[i] = line[i];
    }
  }

  vprev = (float)atof(vstr);
//  printf("DEBUG: readPrevHV returns %f, vstr = %s\n", vprev, vstr);

  return vprev;
}

void fetchRHT(void)
{
  char *can_msg[] = {"dummy", "can0", "123#0000000000000000"};
  cansend(can_msg);
  delay(MSEC);
}

void statCheck(void)
{
  char user_input[10];
  bool INIT_STAT = false;

  while(!INIT_STAT)
  {
    printf("Do you wish to perform initial checks on the System? (Y/N)> ");
    fgets(user_input, 10, stdin);
    printf("Your answer> %s", user_input);
    if (!isValidInput(user_input))
    {
      printf("Please enter something valid.\n");
      continue;
    }

    if (checkYN(user_input))
    {
      printf("Fetching data... \n");
      // send CAN request to fetch RH&T data, light level data, previous 
      // voltage values (stored in a txt file), print info, and store this in
      // another txt file
      char *can_msg[] = {"dummy", "can0", "123#0000000000000000"};
      cansend(can_msg);

      printf("Data fetched!\n");
      INIT_STAT = true;
    }

    else
    {
      printf("Ok.. Your loss. If something breaks, it will be your fault...\n");
      while (!checkYN(user_input))
      {
        printf("Are you SURE? (Y/N)> ");
        fgets(user_input, 10, stdin);
        printf("Your response> %s", user_input);
        if (!isValidInput(user_input))
        {
          printf("Please enter a valid response.\n");
          continue;
        }

        if (checkYN(user_input))
        {
          printf("OK. I hope you know what you're doing...\n");
          INIT_STAT = true;
          break;
        }

        else
        {
          printf("I'm glad you changed your mind! Fetching data now.. ");
          // fetch data
          printf("Data fetched!");
          INIT_STAT = true;
          break;
        }
      } // end inner INIT_STAT while loop
    }
  } // end INIT_STAT while loop
}

void decodeRHT(struct SlowControlsData *sc, char *canmsg)
{
  char hum_data[5], temp_data[5];

  strncpy(hum_data, canmsg+6, 4);   //hum data starts at 3rd byte
  hum_data[4] = '\0';
  strncpy(temp_data, canmsg+10, 4); //temp data starts at 7th byte
  temp_data[4] ='\0';
  printf("hum_data (raw): %s \ntemp_data (raw): %s\n", hum_data, temp_data);  //debug

  //convert str to int
  int hex_hum, hex_temp;
  hex_hum = (int)strtol(hum_data, NULL, 16);
  hex_temp = (int)strtol(temp_data, NULL, 16);

  //shift hex values
  hex_hum &= ~(0x03 << 14);
  hex_temp = hex_temp >> 2;

  //calculate hum & temp
  sc->hum = (double)hex_hum / (pow(2,14) - 2.) * 100.;
  sc->temp = (double)hex_temp / pow(2,14) * 165. - 40.;

  return;
}

void decodeLV(struct SlowControlsData *sc, char *canmsg)
{
  char msgID[4];
  strncpy(msgID, canmsg, 3);
  msgID[3] = '\0';

  if (strcmp(msgID, "220") == 0)
  {
    bool lv_on_off;
    char lv_data[3];
    strncpy(lv_data, canmsg+6, 2);
    lv_data[2] = '\0';
    //printf("lv_data: %s\n", lv_data); //DEBUG

    if ((int)strtol(lv_data, NULL, 16) == 1) { lv_on_off = true; }
    else if ((int)strtol(lv_data, NULL, 16) == 0) { lv_on_off = false; }
    else { printf(" @@@ error reading LV status!! undefined !!!\n"); return; }

    sc->lv_en = lv_on_off;
  }

  if (strcmp(msgID, "3DA") == 0)
  {
    char lvA_data[5], lvB_data[5], lvC_data[5];
    strncpy(lvA_data, canmsg+4, 4);
    lvA_data[4] = '\0';
    strncpy(lvB_data, canmsg+10, 4);
    lvB_data[4] = '\0';
    strncpy(lvC_data, canmsg+16, 4);
    lvC_data[4] = '\0';
    printf(" lv{ABC}_data: %s %s %s\n", lvA_data, lvB_data, lvC_data);

    int hex_lvA = 0, hex_lvB = 0, hex_lvC = 0;
    hex_lvA = (int)strtol(lvA_data, NULL, 16);
    hex_lvB = (int)strtol(lvB_data, NULL, 16);
    hex_lvC = (int)strtol(lvC_data, NULL, 16);

    //calculate low voltages
    sc->lvA = (double)hex_lvA * ADC_CONVERSION;
    sc->lvB = (double)hex_lvB * ADC_CONVERSION;
    sc->lvC = (double)hex_lvC * ADC_CONVERSION;
  }
    
  return;
}

void decodeHV(struct SlowControlsData *sc, char *canmsg)
{
  // get HV enabled status
  bool hv_on_off;
  char hv_data[3];
  strncpy(hv_data, canmsg+6, 2);
  hv_data[2] = '\0';

  if ((int)strtol(hv_data, NULL, 16) == 1) { hv_on_off = true; }
  else if ((int)strtol(hv_data, NULL, 16) == 0) { hv_on_off = false; }
  else { printf(" @@@ error reading HV status!! undefined !!!\n"); return; }

  sc->hv_en = hv_on_off;

  // get actual HV value at photocathode
  char pchv_data[5];
  strncpy(pchv_data, canmsg+10, 4);
  pchv_data[4] = '\0';

  // calculate HV
  int hex_hv = (int)strtol(pchv_data, NULL, 16);
  sc->hv = (double)hex_hv * ADC_CONVERSION;

  return;
}

void decodePhotodiode(struct SlowControlsData *sc, char *canmsg)
{
  char pd_data[5];
  strncpy(pd_data, canmsg+4, 4);
  pd_data[4] = '\0';

  int hex_pd = (int)strtol(pd_data, NULL, 16);
  sc->photodiode = (double)hex_pd * ADC_CONVERSION * 100.;

  return;
}

void decodeTrigBd(struct SlowControlsData *sc, char *canmsg)
{
  //TODO:implement reading power code
  char msgID[4];
  strncpy(msgID, canmsg, 3);
  msgID[3] = '\0';

  char trig_data[4];
  strncpy(trig_data, canmsg+6, 3);
  trig_data[3] = '\0';

  int hex_trig = (int)strtol(trig_data, NULL, 16);
  double trig_thr = hex_trig * 3.0 / 4096; //see MCP4725 datasheet

  if (strcmp(msgID, "0CB") == 0) { sc->trig_dac0 = trig_thr; }
  if (strcmp(msgID, "0FE") == 0) { sc->trig_dac1 = trig_thr; }

  return;
}


bool checkHV(struct SlowControlsData *sc)
{
  FILE *fp = fopen(PREV_HV_FILE, "a+");
  time_t tstamp;
  tstamp = time(NULL);

  bool HV_EN = false;
  int rcvStat = -8;
  char rcv_msg[21];

  printf("\nChecking status of high voltage lines...\n");
  char *can_msg[] = {"dummy", "can0", "034#0000000000000000"};
  cansend(can_msg);
  //delay(100);
  usleep(USLP);
  rcvStat = canread(rcv_msg);
  if (rcvStat == 1)
  {
    decodeCANmsg(sc, rcv_msg);
    HV_EN = sc->hv_en;
    printf("  The high voltage is >>> ");
    printf(HV_EN ? "ENABLED (ON) <<<\n" : "DISABLED (OFF) <<<\n");
    printf(" > high voltage: %.3f V (x1000)\n", sc->hv);

    //write to file
    tstamp = time(NULL);
    fprintf(fp, "%.2f V %s", sc->hv*1000., asctime(localtime(&tstamp)));
  }
  else { printf(" @@@ CAN message receive error code: %d\n", rcvStat); }
  delay(3*MSEC);
  rcv_msg[0] = '\0';

  fclose(fp);

  return HV_EN;
}

int setHV(float vset)
{
  if (vset > HV_MAX)
  {
    vset = HV_MAX;
  }

  float vpct = vset / C40N_MAX;
  printf("    fraction of max HV output (4kV) = %.5f\n", vpct);
  float dac_vout = vpct * DAC_VMAX;
  printf("    DAC output voltage = %.5f\n", dac_vout);

  // convert into DAC input code
  int k = 0;
  k = (int)(pow(2,12) * dac_vout / DAC_VREF);
  //printf("DEBUG:  k = %d,  k = %X (hex),  k << 3 = %X (hex)\n", k, k, (k<<3));

  // prepare CAN msg
  char msg[] = "050#0000000000000000";
  char tmp[17];
  sprintf(tmp, "%X\n", (k<<3));
  //printf("DEBUG:  msg = %s, len = %d\n", msg, strlen(msg));
  //printf("DEBUG:  tmp = %s, len = %d\n", tmp, strlen(tmp));
  if (strlen(tmp) > 5)
  {
    printf("  !!! Unable to set voltage. Please try again.\n");
    return 1;
  }

  int c = 0;
  int delta = 5-strlen(tmp);
  while (c < strlen(tmp)-1)
  {
    msg[4+c+delta] = tmp[c];
//    printf("DEBUG:  msg = %s, len = %d\n", msg, strlen(msg));
    c++;
  }

  // create a can_msg and then send
  char *can_msg[] = {"dummy", "can0", msg};
  printf("  [DEBUG]  can_msg[2] = %s\n\n", can_msg[2]);
  cansend(can_msg);
  delay(3*MSEC);

  return 0;
}

void enableHV(int pwrEn)
{
  if (pwrEn) // ENABLE high voltage
  {
    char *can_msg[] = {"dummy", "can0", "040#BEEFDEAD00000000"};
    cansend(can_msg);
  }
  if (!pwrEn) // DISABLE high voltage
  {
    char *can_msg[] = {"dummy", "can0", "030#00000000BEEFDEAD"};
    cansend(can_msg);
  }
  printf("  The high voltage is >>> ");
  printf(pwrEn ? "ENABLED (ON) <<<\n" : "DISABLED (OFF) <<<\n");

  delay(100);
}


float movingAvg(float *ptrArrNumbers, float *ptrSum, int pos, int len, float nextNum)
{
  /*
   * from: https://gist.github.com/bmccormack/d12f4bf0c96423d03f82
   */

  //subtract oldest num from prev sum, add new num
  *ptrSum = *ptrSum - ptrArrNumbers[pos] + nextNum;
  //assign nextNum to pos in the array
  ptrArrNumbers[pos] = nextNum;
  //return avg
  return *ptrSum / len;
}


/**************************************************
 *                NOT USED ANYMORE                *
 **************************************************/

void initSequence(void) // NOT USED ANYMORE
{
  char user_input[10];

  /* POWER UP THE LVHV BOARD (SWITCH POWER RELAY) */
  bool PWR_EN = false;

  while (!PWR_EN)
  {
    printf("Do you wish to power up (+12V) the LVHV board? (Y/N)> ");
    fgets(user_input, 10, stdin);
    printf("Your answer> %s", user_input);
    if (!isValidInput(user_input))
    {
      printf("Please enter something valid!\n");
      continue;
    }

    // nuances: any word beginning with Y/y or N/n will make a decision!
    if (checkYN(user_input))
    {
      /* switch power relay ON */
      long long rlyN = 0; // default: all channels lol
      char rly_input[10];
      char *tmp = NULL;

      printf("Which relay do you wish to switch? (1/2/3/0[all])> ");
      fgets(rly_input, 10, stdin);
      rlyN = strtol(rly_input, &tmp, 10);
      printf("Relay Ch: %lld\n", rlyN);

      relayHAT(rlyN, HIGH);

      PWR_EN = true;
      printf("The LVHV is powered up. You can communicate with it now.\n");
    }
    else
    {
      printf("You cannot do anything until the LVHV board is powered up.\n");
      // wait for user to say YES
    }
  } // end PWR_EN while loop


  /* TURN ON LOW VOLTAGE LINES */
  bool LV_EN = false;

  while(!LV_EN)
  {
    printf("Do you wish to turn ON the low voltage lines? (Y/N)> ");
    fgets(user_input, 10, stdin);
    printf("Your final answer> %s", user_input);
    if (!isValidInput(user_input))
    {
      printf("Please enter a valid response.\n");
      continue;
    }
    else if (checkYN(user_input))
    {
      /* send CAN msg to turn ON LV lines */
      // msgID = 0x020
      char *can_msg[] = {"dummy", "can0", "020#DEADBEEF00000000"};

      cansend(can_msg);
      delay(100);

//      canread();
//      for (int i = 0; i < 50; ++i);

      LV_EN = true;
      printf("The low voltage lines are ON.\nYou can now (1) read humidity and temperature, \n(2) set and turn ON/OFF the high voltage, \n(3) set the trigger threshold. \n");
    }
    else
    {
      printf("ok.. :( \nYou can't do much else here either...\n");
      // wait until user says YES
    }
  } // end LV_EN while loop
}

void initRelayHAT(void)
{
  wiringPiSetup();
  pinMode(RLY1, OUTPUT);
  pinMode(RLY2, OUTPUT);
  pinMode(RLY3, OUTPUT);

  // make sure they switched OFF
  digitalWrite(RLY1, LOW);
  delay(RLYDLY);
  digitalWrite(RLY2, LOW);
  delay(RLYDLY);
  digitalWrite(RLY3, LOW);
  delay(RLYDLY);
}

/* turn on/off power relay HAT */
void relayHAT(int rlyN, int rlyEN)
{
  int rlyCH;

  if (rlyN == 1) rlyCH = RLY1;
  if (rlyN == 2) rlyCH = RLY2;
  if (rlyN == 3) rlyCH = RLY3;

  if (rlyN == 0)
  {
    digitalWrite(RLY1, rlyEN);
    delay(RLYDLY);
    digitalWrite(RLY2, rlyEN);
    delay(RLYDLY);
    digitalWrite(RLY3, rlyEN);
    delay(RLYDLY);
    return;
  }
  else
  {
    digitalWrite(rlyCH, rlyEN);
    delay(RLYDLY);
    return;
  }
}
