#pragma once

#define BUFFER_SIZE 1024

#include <string.h>
#include <ctype.h>
#include "myfunc.h"
#include "stm32f10x.h"
#include "stdbool.h"

void update_SI5351(void);
void send_data(u8* arr);
void reaction(void);
void operation(u8 *arr);

typedef enum{
    STATE_SEQUENTIAL_SAMPLE,
    STATE_SINGLE_SAMPLE,
    STATE_STOP,
    STATE_SAVE,
    STATE_RECALL
  } State;

  typedef enum{
    AMP_10 = 5,
    AMP_5 = 4,
    AMP_1 = 3,
    AMP_0_4 = 2,
    AMP_0_2 = 1,
    AMP_0_04 = 0
  } AMP_TIMES;
  
