/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Pieter Noordhuis
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _I2C_H
#define _I2C_H

#include "user_config.h"
#include <util/twi.h>

/* 100kHz */
#define I2C_FREQ 100000

#define I2C_TXN_DONE _BV(0)
#define I2C_TXN_ERR  _BV(1)

typedef struct i2c_op i2c_op_t;
typedef struct i2c_txn i2c_txn_t;

struct i2c_op {
  uint8_t address;
  uint8_t buflen;
  uint8_t bufpos;
  uint8_t *buf;
};

struct i2c_txn {
  struct i2c_txn *next;
  volatile uint8_t flags;
  uint8_t opslen;
  uint8_t opspos;
  struct i2c_op ops[];
};

static inline void i2c_op_init(i2c_op_t *o, uint8_t address, uint8_t *buf, uint8_t buflen) {
  o->address = address;
  o->buflen = buflen;
  o->bufpos = 0;
  o->buf = buf;
}

static inline void i2c_op_init_rd(i2c_op_t *o, uint8_t address, uint8_t *buf, uint8_t buflen) {
  i2c_op_init(o, (address << 1) | TW_READ, buf, buflen);
}

static inline void i2c_op_init_wr(i2c_op_t *o, uint8_t address, uint8_t *buf, uint8_t buflen) {
  i2c_op_init(o, (address << 1) | TW_WRITE, buf, buflen);
}

static inline void i2c_txn_init(i2c_txn_t *t, uint8_t opslen) {
  t->flags = 0;
  t->opslen = opslen;
  t->opspos = 0;
  t->next = NULL;
}

void i2c_init(uint32_t speed);
void i2c_post(i2c_txn_t *t);

#endif
