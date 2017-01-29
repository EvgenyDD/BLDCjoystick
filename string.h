/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STRING_H
#define STRING_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int strlen(uint8_t *);
int strlenNum(uint8_t *pText, int begin);
void itoa_(int32_t, uint8_t s[]);
void itoa_zeros(uint32_t n, uint8_t s[], uint8_t end);
void ftoaPseudo_(int32_t n, uint8_t s[], uint8_t pointNumbers);

void itoa_hex(int64_t n, uint8_t s[]);
void itoa_zeros2(uint32_t n, uint8_t s[], uint8_t end);

void dtoa_(int64_t n, uint8_t s[]);
void dtoaPositive_(int64_t n, uint8_t s[]);
void ftoa_(float, uint8_t str[], uint8_t precision);
void reverse(uint8_t s[]);
void strcat_(uint8_t first[], uint8_t second[]);
void strcat_NO_EOL(uint8_t first[], uint8_t second[]);
void strcat_white(uint8_t first[], uint8_t first_limit, uint8_t second[], uint8_t second_limit);
void strcpy_(uint8_t first[], uint8_t second[]);
void strcatNum2(uint8_t first[], uint8_t second[], int begin);

void strcatNum(uint8_t first[], uint8_t second[], int begin, int end);
void strcatnum(uint8_t first[], uint8_t second[], uint8_t num);

uint64_t pow10_(uint8_t x);
float log10_(int v);
float pow_(float x, float y);
void ftoa_(float num, uint8_t str[], uint8_t precision);

float atof(uint8_t* num);
uint64_t stoi_(uint8_t *s);
uint64_t atoi_BCD(uint8_t *s, uint8_t start, uint8_t end);
uint64_t atoi_special(uint8_t *s, uint8_t start, uint8_t end);
uint64_t atoi_special2(uint8_t *s, uint8_t start, uint8_t end);
uint64_t atoi_special_BCD(uint8_t *s, uint8_t start, uint8_t end);
uint64_t atoi_special_BCDinv(uint8_t *s, uint8_t start, uint8_t end);

#endif //STRING_H
