#ifndef SCPU
#define SCPU

#define COMMAND_VERSION 2

#define BEGIN_PROGRAM_PTR 11

const size_t COMMAND_AND_MASK_SIZE = 2;
const size_t COMMAND_SIZE          = 1;

const char REG_MASK = 2;

const char INT_MASK = 1;

const char MEM_MASK = 4;

const char SIGN_FIRST  = 'K';

const char SIGN_SECOND = 'C';

const size_t MEM_H = 200;

const size_t MEM_W = 320;

const size_t PIXEL_SIZE = 4;

const unsigned int FPS = 10;

const size_t REG_SIZE = 5;

const size_t MAX_CALLOC = 10;


#endif