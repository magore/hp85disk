# avr-i2c

A simple, interrupt driven I2C API for Atmel AVR processors.

* Only assumes the I2C master role.
* **Not** multi-master aware.
* Made to run on an ATmega328P without portability in mind.
* Made with avr-libc 1.8.0 (may not work with earlier versions).

## Why?

* Other I2C APIs I found were blocking (e.g. the on in the Arduino SDK).
* I like to learn about the internals of the processor.

## How?

First, call `i2c_init()` to enable and initialize the processor's I2C system.

(the AVR docs call it TWI (Two Wire Interface), but it's really just I2C)

Then, for every set of I2C operations you want to execute while being bus master,
create a struct of type `i2c_tnx_t` and pass it as argument to `i2c_post`:

```c
char hello[6] = "hello";
char world[6];

i2c_txn_t *t;

t = alloca(sizeof(*t) + 2 * sizeof(t->ops[0]));
i2c_txn_init(t, 2);
i2c_op_init_wr(&t->ops[0], I2C_ADDRESS, hello, sizeof(hello));
i2c_op_init_rd(&t->ops[1], I2C_ADDRESS, world, sizeof(world));

/* Post transaction and wait for it to complete. */
i2c_post(t);
while (!(t->flags & I2C_TXN_DONE)) {
  /* You can do whatever you want here. */
  sleep_mode();
}

if (t->flags & I2C_TXN_ERR) {
  /*
   * Some error occured...
   */
} else {
  /*
   * The char[] `world` now contains the response to the read request.
   */
}
```

## Critique

* The transaction struct can take up quite a bit of memory.

## License

MIT (see ``LICENSE``).
