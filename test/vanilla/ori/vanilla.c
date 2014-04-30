#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#ifndef SEED
#define SEED 0
#endif

#ifndef TYPE
// default type
#define TYPE 1
#endif

#if TYPE == 1
  #define TYPE1 int8_t
  #define TYPE2 int8_t
#elif TYPE == 2
  #define TYPE1 int8_t
  #define TYPE2 int16_t
#elif TYPE == 3
  #define TYPE1 int8_t
  #define TYPE2 intptr_t
#elif TYPE == 4
  #define TYPE1 int8_t
  #define TYPE2 int32_t
#elif TYPE == 5
  #define TYPE1 int8_t
  #define TYPE2 int64_t
#elif TYPE == 6
  #define TYPE1 int8_t
  #define TYPE2 float
#elif TYPE == 7
  #define TYPE1 int8_t
  #define TYPE2 double
#elif TYPE == 8
  #define TYPE1 int32_t
  #define TYPE2 float
#elif TYPE == 9
  #define TYPE1 int32_t
  #define TYPE2 double
#elif TYPE == 10
  #define TYPE1 int32_t
  #define TYPE2 intptr_t
#elif TYPE == 11
  #define TYPE1 float
  #define TYPE2 double
#elif TYPE == 12
  #define TYPE1 int64_t
  #define TYPE2 double
#elif TYPE == 13
  #define TYPE1 int64_t
  #define TYPE2 float
#elif TYPE == 14
  #define TYPE1 int64_t
  #define TYPE2 int32_t
#endif

struct TwoFields {
  TYPE1 f1;
  TYPE2 f2;
};

double affinity = 1.f;

typedef enum {STREAM, RANDOM} Pattern;
Pattern pattern = STREAM;

size_t range1 = 16;
size_t range2 = 16;
long long offset1= 0;
long long offset2= 0;
double sizeInMB = 2.5;
size_t size = 312500000;

size_t iter = 1000000;

void usage(char *program);
void parse_args(int argc, char **argv);
double frand() { return (double)(rand()) / RAND_MAX; }

int main(int argc, char **argv) {
  srand(SEED);
  parse_args(argc, argv);
  struct TwoFields data[size];

  //initialization
  printf("size: %d %d \n", sizeof(TYPE1), sizeof(TYPE2));
  size_t i;
  for (i=0; i<size; i++) {
    data[i].f1 = offset1 + (double)rand() / ((double)RAND_MAX/range1);
    data[i].f2 = offset2 + (double)rand() / ((double)RAND_MAX/range2);
    printf("%d,%d\n",data[i].f1,data[i].f2);
  }
  printf("Initialization done!\n");

  //access
  size_t index = 0;
  size_t sum = 0;
  double coin;
  while (iter--) {
    coin = frand();
    if (coin <= affinity) {
      sum += data[index].f1;
      sum += data[index].f2;
    } else {
      coin = frand();
      if (coin < 0.5f) {
        sum += data[index].f1;
      } else {
        sum += data[index].f2;
      }
    }
    if (pattern == STREAM) {
      //index = (index++) % size;
	  index ++;
	  index %= size;
    } else if (pattern == RANDOM) {
      index = rand() % size;
    }
  }
  return 0;
}

void usage(char *program) {
  printf("Usage: %s [options]\n"
      "  microbenchmark that generates random value in the stack and then access it. \n"
      "\n"
      "Program Options:\n"
      "  -i  --access iter            Number of acccesses to data DEFAULT=1000000\n"
      "  -a  --affinity affinity      Probability that two fields are accessed together. (Affinity knob) DEFAULT=1\n"
      "  -r  --range range1,range2    Maximum of the value. (Compressibility knob) DEFAULT=16,16\n"
      "  -f  --offset offset1,offset2 Offset of the value. (Compressibility knob) DEFAULT=0,0\n"
      //      "  -t  --type int/char          Type of the value. (Compressibility knob DEFAULT=int)\n"
      "  -s  --size size              Size of the value. (Size knob) DEFAULT=2.5MB\n"
      "  -p  --pattern stream/random  Access pattern. (Pattern knob) DEFAULT=stream\n"
      "  -?  --help                   This message\n", program);
}

void parse_args(int argc, char **argv) {
  int opt;

  static struct option long_opts[] = {
    {"access", 1, 0, 'i'},
    {"affinity", 1, 0, 'a'},
    {"range", 1, 0, 'r'},
    {"offset", 1, 0, 'f'},
    //    {"type", 1, 0, 't'},
    {"size", 1, 0, 's'},
    {"pattern", 1, 0, 'p'},
    {"help", 0, 0, '?'},
    {0, 0, 0, 0}
  };

  while ((opt = getopt_long(argc, argv, "i:a:r:f:s:p:?h", long_opts, NULL)) != EOF) {
    switch (opt) {
      case 'i':
        iter = atoll(optarg);
        break;
      case 'a':
        if (sscanf(optarg, "%lf", &affinity) == 1) {
          if (affinity < 0.f || affinity > 1.f) {
            printf("Error: Invalid affinity: %s \n", optarg);
            exit(EXIT_SUCCESS);
          }
        } else {
          printf("Error: Invalid affinity: %s \n", optarg);
          exit(EXIT_SUCCESS);
        }
        break;
      case 'r':
        if (sscanf(optarg, "%u,%u", &range1, &range2) == 2) {
        } else {
          printf("Error: Unable to parse range: %s \n", optarg);
          exit(EXIT_SUCCESS);
        }
      case 'f':
        if (sscanf(optarg, "%lld,%lld", &offset1, &offset2) == 2) {
        } else {
          printf("Error: Unable to parse offset: %s \n", optarg);
          exit(EXIT_SUCCESS);
        }
        break;
        /*      case 't':
                if (strcmp(optarg, "int") == 0) {
                type = INT;
                } else if (strcmp(optarg, "char") == 0) {
                type = CHAR;
                printf("Unimplemented type: %s \n", optarg);
                exit(EXIT_SUCCESS);
                } else {
                printf("Error: Unknown type: %s \n", optarg);
                exit(EXIT_SUCCESS);
                }
                break;*/
      case 's':
        if (sscanf(optarg, "%lfMB", &sizeInMB) == 1) {
        } else {
          printf("Error: Unable to parse size: %s \n", optarg);
          exit(EXIT_SUCCESS);
        }
        break;
      case 'p':
        if (strcmp(optarg, "stream") == 0) {
          pattern = STREAM;
        } else if (strcmp(optarg, "random") == 0) {
          pattern = RANDOM;
        } else {
          printf("Error: Unknown pattern: %s \n", optarg);
          exit(EXIT_SUCCESS);
        }
        break;
      case '?':
      default:
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
  }
  size = (sizeInMB * 1000000) / sizeof(struct TwoFields);
}
