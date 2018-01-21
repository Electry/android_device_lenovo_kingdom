/*
 * TfaI2Cdef.h
 *
 *
 *  Created on: Oct 8, 2013
 *  Author: SH Ding
 */

#ifndef _TFAI2CDEF_H_
#define _TFAI2CDEF_H_

#define TFA_I2CDEVICE        "/dev/tfa9890"        // Panda = 4 device driver interface special file
#define TFA_I2CSLAVEBASE        (0x34)              // tfa device slave address of 1st (=left) device
#define I2C_ADDRESS             (TFA_I2CSLAVEBASE<<1)

#endif /* _TFAI2CDEF_H_ */
