#include "infix.h"
/*#define TESTING_CONVERTERS*/

/**
 * str_u32 - convert a string of numbers to a 32 bit int array
 * @num: a string of only integers
 *
 * Description: This function converts a string of numbers to an array of
 * unsigned 32 bit integers.
 * The array shall be in little endian style whereby the lower value numbers
 * will be placed in the lower indices. Index 0 will have a value indicating
 * the size of the array.
 *
 * Return: an array of 32 bit ints, NULL on failure
 */
uint32_t *str_u32(uint8_t *num)
{
	size_t u32arr_sz = 0, len = 0, h = 0;
	int i = 0, g = 0;
	uint32_t *u32arr = NULL;

	if (!num)
		return (NULL);

	num += pad_char((char *)num, "0");
	len = strlen((char *)num);
	u32arr_sz = (len / U32_DIGITS) + ((len % U32_DIGITS) ? 1 : 0);
	u32arr = calloc((u32arr_sz + 2), sizeof(*u32arr));
	if (!u32arr)
	{
		perror("Malloc Fail");
		return (NULL);
	}

	/*Index 0 will have the size of the array*/
	u32arr[0] = u32arr_sz;
	/*The number in the string shall be read from the least significant digit*/
	for (h = 1, g = (len - 1); h <= u32arr_sz && g >= 0; h++)
	{
		for (i = 0; i < U32_DIGITS && (g - i) >= 0; i++)
			u32arr[h] += (num[g - i] - '0') * (uint32_t)(pow(10, i));

		g -= i;
	}

	return (u32arr);
}

/**
 * u32_str - convert a 32 bit int array to a string
 * @u32a: a 32 bit array
 *
 * Description: This function converts an array of unsigned 32 bit integers
 * to a string.
 * The array shall be in little endian style whereby the lower value numbers
 * will be placed in the lower indices. Index 0 will have a value indicating
 * the size of the array.
 *
 * Return: a string of numbers, NULL on failure
 */
uint8_t *u32_str(uint32_t *u32a)
{
	size_t u32arr_sz = 0, len = 0, g = 0, h = 0, i = 0;
	uint8_t *num = NULL;
	ssize_t temp = 0, div = 1;

	if (!u32a)
		return (NULL);

	u32arr_sz = u32a[0];
	while (!u32a[u32arr_sz] && u32arr_sz > 1)
		--u32arr_sz;

	len = u32arr_sz * U32_DIGITS;
	num = calloc((len + 1), sizeof(*num));
	if (!num)
	{
		perror("Malloc Fail");
		return (NULL);
	}

	temp = u32a[u32arr_sz];
	while (temp / div >= 10)
		div *= 10;

	for (h = u32arr_sz, g = 0; h > 0 && g < len; h--)
	{
		temp = u32a[h];
		for (i = 0; div && (g + i) < len; i++)
		{
			num[g + i] = (temp / div) + '0';
			temp %= div;
			div /= 10;
		}

		g += i;
		div = (U32_ROLL / 10);
	}

	if (i)
		num[g] = '\0';

	return (num);
}

#ifdef TESTING_CONVERTERS
/**
 * pad_char - calculates length of initial padding characters in a string
 * @str: the string to check
 * @ch: the character
 *
 * Return: number of padding characters
 */
size_t pad_char(char *str, char *ch)
{
	size_t zeros = 0, len = 0;

	if (str)
	{
		len = strlen(str);
		zeros = strspn(str, ch);
		if (len && (zeros == len))
			zeros--;
	}

	return (zeros);
}

/**
 * main - testing entry point
 *
 * Return: 0 on success, 1 on failure
 */
int main(void)
{
	size_t i = 0, g = 0;
	uint32_t *ntemp = NULL, len = 0;
	uint8_t *stemp = NULL;
	char *nstr[] = {
		"123456789",
		"12345678912",
		"1000000000",
		"909897004000000000078234587",
		"0",
		"12",
		"00000678",
		"00000678912",
		"000000000003456",
		"00000100000000200000000300000000004000000",
		NULL,
	};

	while (nstr[g])
	{
		printf("%s\n", &nstr[g][pad_char(nstr[g], "0")]);
		/*ntemp = str_u32((uint8_t *)(&nstr[g][pad_char(nstr[g], "0")]));*/
		/*printf("%s\n", nstr[g]);*/
		ntemp = str_u32((uint8_t *)nstr[g]);
		if (!ntemp)
			return (EXIT_FAILURE);

		len = ntemp[0];
		for (i = len; i > 0; i--)
		{
			if (i < len)
				printf("%09d", (uint32_t)ntemp[i]);
			else
				printf("%d", (uint32_t)ntemp[i]);
		}

		printf(" [%d: Blocks]\n", (int)len);
		stemp = u32_str(ntemp);
		free(ntemp);
		if (stemp)
			printf("%s\n\n", (char *)stemp);
		else
			return (EXIT_FAILURE);

		free(stemp);
		ntemp = NULL;
		stemp = NULL;
		g++;
	}

	return (EXIT_SUCCESS);
}
#endif
