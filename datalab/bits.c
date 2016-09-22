/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* 
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
  return ~(~x | ~y); // A AND B = ~ (~A OR ~B)
}
/* 
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
  int numShift = n << 3;

  return (x >> numShift) & 0xFF; // Move n-th byte to LSB position, and take mask.
}
/* 
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3 
 */
int logicalShift(int x, int n) {
  int arithShift = x >> n;
  int mask = ~(((1 << 31) >> n) << 1); // n bits of zero from MSB, and all others are one.

  return arithShift & mask;
}
/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int bitCount(int x) {
  int mask55; // Repeat 01010101
  int mask33; // Repeat 00110011
  int mask0f; // Repeat 00001111
  int maskff = 0xff; // Repeat 0000000011111111
  int maskhalf = maskff | (maskff << 8); // 16 0s and 16 1s

  maskff |= maskff << 16;

  mask0f = maskff ^ (maskff << 4);
  mask33 = mask0f ^ (mask0f << 2);
  mask55 = mask33 ^ (mask33 << 1);

  // Parallel, divide and conquer adding of bits

  x = (x & mask55) + ((x >> 1) & mask55);
  x = (x & mask33) + ((x >> 2) & mask33);
  x = (x + (x >> 4)) & mask0f;
  x = (x + (x >> 8)) & maskff;
  x = (x + (x >> 16)) & maskhalf;

  return x;
}
/* 
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int bang(int x) {
  int isNonneg = (x >> 31) + 1;  // 1 if nonneg, 0 otherwise

  int xMinusOne = x + (~0);
  int xMinusOneIsNeg = xMinusOne >> 31; // 0xffffffff if neg, 0 otherwise 

  return isNonneg & xMinusOneIsNeg;
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  return 1 << 31;
}
/* 
 * fitsBits - return 1 if x can be represented as an 
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n) {
  int xPos = x ^ (x >> 31); // Take negation if x is negative
  int nMinusOne = n + ~0;

  // And check whether the most significant one is in n-1 bytes from LSB.
  // (Not n because of the sign bit)
  return !(xPos >> nMinusOne);
}
/* 
 * divpwr2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero
 *   Examples: divpwr2(15,1) = 7, divpwr2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int divpwr2(int x, int n) {
  int signMask = x >> 31; // 1111... if neg, 000... if pos.
  int addTerm = ~(signMask << n);

  return (x + (addTerm & signMask)) >> n;
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return ~x + 1; // Just formula from the lecture note
}
/* 
 * isPositive - return 1 if x > 0, return 0 otherwise 
 *   Example: isPositive(-1) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 3
 */
int isPositive(int x) {
  int isNeg = x >> 31;
  int isZero = !x;

  return !(isNeg | isZero);
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  int xNeg = x >> 31;
  int yNeg = y >> 31;
  int yNonneg = !yNeg;

  int xNegYPos = xNeg & yNonneg;
  int notXPosYNeg = xNeg | yNonneg; // 0 or 0xffffffff

  int yMinusX = y + (~x + 1);
  int yMinusXPos = !(yMinusX >> 31);

  return xNegYPos | (notXPosYNeg & yMinusXPos);
}
/*
 * ilog2 - return floor(log base 2 of x), where x > 0
 *   Example: ilog2(16) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 90
 *   Rating: 4
 */
int ilog2(int x) {
  int result;
  int pivot;

  // Find most significant 1 in similar manner to binary search
 
  pivot = (!!(x >> 16)) << 4;
  result = pivot;
  x >>= pivot;

  pivot = (!!(x >> 8)) << 3;
  result += pivot;
  x >>= pivot;

  pivot = (!!(x >> 4)) << 2;
  result += pivot;
  x >>= pivot;

  pivot = (!!(x >> 2)) << 1;
  result += pivot;
  x >>= pivot;

  return result + (x >> 1);
}
/* 
 * float_neg - Return bit-level equivalent of expression -f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned float_neg(unsigned uf) {
  int uf_nosign = uf & 0x7fffffff;
  int exp_bit = 0x7f800000;

  if (uf_nosign > exp_bit) // If exponent bit is 0x7f800000 and mentissa is nonzero
    return uf;
  else
    return uf ^ (0x80000000);
}
/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_i2f(int x) {
  int sign = x & 0x80000000;
  int value = x - sign;
  int exp = 0x4b000000;

  int round_bit = 0;
  int round_remainder = 0;

  int ment_bit = 0x007fffff;
  int exp_lsb = 0x00800000;

  if (!value) {
    if (sign) return 0xcf000000; // 0x80000000
    else return 0; // 0
  }

  if (sign)
    value = 0x80000000 - value; // Two's complement negation

  while (value > 0xffffff) {
    round_remainder |= round_bit;
    round_bit = value & 0x1;
    value = value >> 1;
    exp += exp_lsb;
  }

  // If remainder is larger than half,
  // or equal to half, LSB is 1.
  if (round_bit)
    value += (round_remainder || (value & 0x1));

  // Exp increased by rounding
  if (value == 0x1000000) {
    value = value >> 1;
    exp += exp_lsb;
  }

  while (value < 0x800000) {
    value = value << 1;
    exp -= exp_lsb;
  }

  return sign + exp + (value & ment_bit);
}
/* 
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {
  int exp_bit = 0x7f800000;
  int ment_bit = 0x007fffff;
  int exp_lsb = 0x00800000;

  int uf_exp = uf & exp_bit;

  if (uf_exp == exp_bit) // Inf or NaN
    return uf;
  else if (!uf_exp) // Denormalized values
    return uf + (uf & ment_bit);
  else // Normalized values
    return (uf + exp_lsb);
}
