//
//  FastTouch.cpp
//
//
//  Created by AdrianFreed on 3/12/18.
//
//

#include "FastTouch.h"

#if defined(CORE_TEENSY)

//#if defined(__IMXRT1052__) || defined(__IMXRT10562__)

#if defined(__IMXRT1052__) || defined(__IMXRT1062__)

FASTRUN
int fastTouchRead(int pin)
{
    int i;
    const struct digital_pin_bitband_and_config_table_struct *p;
    if (pin >= CORE_NUM_DIGITAL)
        return -1;
    p = digital_pin_to_info_PGM + pin;

    pinMode(pin, OUTPUT_OPENDRAIN);
    digitalWrite(pin, LOW);
    delayMicroseconds(1);
    /* disable interrupts */
    noInterrupts();
    pinMode(pin, INPUT_PULLUP);
    i = 0;
    if ((*(p->reg + 2) & p->mask) != 0)
        goto out;
    if ((*(p->reg + 2) & p->mask) != 0)
        goto out;
    if ((*(p->reg + 2) & p->mask) != 0)
        goto out;
    i++;
    if ((*(p->reg + 2) & p->mask) != 0)
        goto out;
    i++;
    if ((*(p->reg + 2) & p->mask) != 0)
        goto out;
    i++;
    if ((*(p->reg + 2) & p->mask) != 0)
        goto out;
    i++;
    if ((*(p->reg + 2) & p->mask) != 0)
        goto out;
    i++;
    if ((*(p->reg + 2) & p->mask) != 0)
        goto out;
    i++;

    for (; i < 256; ++i)
        if ((*(p->reg + 2) & p->mask) != 0)
            break;
    //        v += fastDigitalRead(pin)? 0:1;
    {

        pinMode(pin, OUTPUT_OPENDRAIN);
        digitalWrite(pin, LOW);
    }
out:
    interrupts();
    return i;
}
#elif defined(__MKL26Z64__) || defined(__IMXRT1052__) || defined(__IMXRT1062__) /* Teensy 3LC */
FASTRUN
int fastTouchRead(int pin)
{
    const unsigned m = digitalPinToBitMask(pin);

    pinMode(pin, OUTPUT_OPENDRAIN);
    digitalWrite(pin, LOW);
    delayMicroseconds(50);
    /* disable interrupts */
    noInterrupts();
    pinMode(pin, INPUT_PULLUP);

    //    for(int i=0;i<64;++i)
    //    ft_o += *port&x;
    {
        register unsigned a, b, c, d, e, f;
        register unsigned aa, ba, ca, da;
#ifdef CONDITIONALAPPROACH
        register unsigned i = 0;

        a = *portInputRegister(pin) & m;
        a = *portInputRegister(pin) & m;
        a = *portInputRegister(pin) & m;
        b = *portInputRegister(pin) & m;
        c = *portInputRegister(pin) & m;
        d = *portInputRegister(pin) & m;
        e = *portInputRegister(pin) & m;
        f = *portInputRegister(pin) & m;
        aa = *portInputRegister(pin) & m;
        ba = *portInputRegister(pin) & m;
        ca = *portInputRegister(pin) & m;
        da = *portInputRegister(pin) & m;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
        if (*portInputRegister(pin) & m)
            goto out;
        ++i;
    out:;

#else
        register unsigned i;

        i = *portInputRegister(pin) & m;
        i = *portInputRegister(pin) & m;

        i = *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;

        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;

        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;

        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;

        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;

        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;

        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;

        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;
        i += *portInputRegister(pin) & m;

#endif

        interrupts();

        //        i += ((a & m)?0:1 ) + ((b& m )?0:1 ) +
        // ((c & m)?0:1 ) + ((d& m)?0:1 ) + ((e & m)?0:1 ) + ((f& m)?0:1 );
        //i += ((aa & m)?0:1 ) + ((ba& m )?0:1 ) +
        // ((ca & m)?0:1 ) + ((da & m)?0:1 ) + ((ea & m)?0:1 ) + ((fa& m)?0:1 );
        {
            volatile unsigned t = i, mm = m;
            while (mm >>= 1)
                t >>= 1;
            i = 64 - t;
        } //+(a+b+c+d+e+f+da+ca+ba+aa    10+
        pinMode(pin, OUTPUT_OPENDRAIN);
        digitalWrite(pin, LOW);

        return i;
    }
}
#else
FASTRUN
int fastTouchRead(int pin)
{
    pinMode(pin, OUTPUT_OPENDRAIN);
    digitalWriteFast(pin, LOW);
    delayMicroseconds(50);
    /* disable interrupts */
    noInterrupts();
    pinMode(pin, INPUT_PULLUP);

    //    for(int i=0;i<64;++i)
    //    ft_o += *port&x;
    {
        register unsigned i = 0;
        register uint8_t a, b, c, d, e, f;
        register uint8_t aa, ba, ca, da;

        a = *portInputRegister(pin);
        a = *portInputRegister(pin);
        a = *portInputRegister(pin);
        a = *portInputRegister(pin);
        a = *portInputRegister(pin);
        a = *portInputRegister(pin);
        a = *portInputRegister(pin);
        a = *portInputRegister(pin);
        a = *portInputRegister(pin);
        a = *portInputRegister(pin);
        a = *portInputRegister(pin);
        a = *portInputRegister(pin);
        a = *portInputRegister(pin);
        a = *portInputRegister(pin);

        a = *portInputRegister(pin);
        a = *portInputRegister(pin);
        a = *portInputRegister(pin);
        b = *portInputRegister(pin);
        c = *portInputRegister(pin);
        d = *portInputRegister(pin);
        e = *portInputRegister(pin);
        f = *portInputRegister(pin);
        aa = *portInputRegister(pin);
        ba = *portInputRegister(pin);
        ca = *portInputRegister(pin);
        da = *portInputRegister(pin);

        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
        if (*portInputRegister(pin))
            goto out;
        ++i;
    out:

        interrupts();
        //        i += ((a & m)?0:1 ) + ((b& m )?0:1 ) +
        // ((c & m)?0:1 ) + ((d& m)?0:1 ) + ((e & m)?0:1 ) + ((f& m)?0:1 );
        //i += ((aa & m)?0:1 ) + ((ba& m )?0:1 ) +
        // ((ca & m)?0:1 ) + ((da & m)?0:1 ) + ((ea & m)?0:1 ) + ((fa& m)?0:1 );
        {
            volatile uint8_t t = 10 - (a + b + c + d + e + f + da + ca + ba + aa);
            i += t;
        }
        pinMode(pin, OUTPUT_OPENDRAIN);
        digitalWriteFast(pin, LOW);

        return i;
    }
}
#endif
#endif

int fastTouchMax()
{
    return 60;
}
