#include <pthread.h>
#include "rpiCAN.h"
#include <semaphore.h>
#include <stdio.h>

#define STR_SIZE  100

int main();
static int commandMessageFromStr();
static void printJsonCanMessage(CanMessage*);
static void* messageReader(void*);

static CanMessage commandMessage;
static char str[STR_SIZE];
static pthread_t readThread;
static uint8_t run = 1;

int main() {
  rpiCAN_init(RPICAN_GPIO_5);
  rpiCAN_setBaud(RPICAN_BAUD_50KBIT);

  rpiCAN_start();

  pthread_create(&readThread, NULL, &messageReader, NULL);

  while(run) {
    fgets(str, STR_SIZE, stdin);
    if (commandMessageFromStr() == 0) {
      rpiCAN_write(&commandMessage);
    }
  }

  return 0;
}

static int commandMessageFromStr() {
  if (str[0] == 0 || str[0] == '\n') return 1;

  uint32_t buffer[12] = {0};
  uint16_t i = 0;
  uint8_t buf_i = 0;
  uint8_t char_val = 0;
  uint8_t flag = 0; // prevents incrementing buf_i multiple times for extra spaces

  while (str[i] != 0 && i < STR_SIZE) {
    char_val = str[i] - '0'; // for chars < '0', the result will be a large num

    if (char_val < 10) {
      if (buf_i >= 12) return 1;
      flag = 1;
      buffer[buf_i] = buffer[buf_i] * 10 + char_val;
    } else if (flag == 1){
      buf_i++;
      flag = 0;
    }

    i++;
  }

  if (buf_i == 0) return 1;

  commandMessage.mtype = buffer[0];
  commandMessage.sid = buffer[1];
  commandMessage.eid = buffer[2];
  commandMessage.length = buffer[3];

  if (commandMessage.length > 8) return 1;

  for (i = 0; i < commandMessage.length; i++) {
    commandMessage.data[i] = buffer[i+4];
  }

  return 0;
}

static void printJsonCanMessage(CanMessage* message) {
  printf("{\"mtype\":0x%02X,\"sid\":0x%04X,\"eid\":0x%08X,\"length\":0x%02X,\"data\":[",
          message->mtype, message->sid, message->eid, message->length);

  uint8_t i = 0;
  while (i < message->length) {
    printf("0x%02X", message->data[i]);
    i++;

    if (i < message->length) {
      printf(",");
    }
  }

  printf("]}\n");
  fflush(stdout);
}

static void* messageReader(void* arg) {
  CanMessage message;

  while(run) {
    rpiCAN_read(&message);
    printJsonCanMessage(&message);
  }

  pthread_exit(0);
}
