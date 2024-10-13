#pragma once

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>

//#define DEBUG   1

#define msg(...)    printf(__VA_ARGS__)
#ifdef DEBUG
  #define dbg(...)    printf(__VA_ARGS__)
 #else
  #define dbg
 #endif

void hex ( uint8_t * dt, int size );
int boot_start ( char * port );

#ifndef msg 
#endif

#ifndef dbg

#endif

