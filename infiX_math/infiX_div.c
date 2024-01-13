#include "infix.h"

uint32_t *remain = NULL;
#define TESTING_DIV2

#ifdef TESTING_DIV2
/**
 * infiX_div - divides a numbers stored in an array
 * @dividend: dividend
 * @divisor: divisor
 *
 * Description: this function will store the remainder of any division
 * in a global variable "remain". This variable should be freed after
 * this function has returned.
 *
 * Return: array with the result, NULL on failure
 */
uint32_t *infiX_div(uint32_t *dividend, uint32_t *divisor)
{
	size_t len_sor = 0, len_dend = 0, len_rem = 0, len_q = 0, nd = 0, r = 0, q = 0;
	uint32_t *quotient = NULL;
	int64_t hold = 0;

	remain = NULL;
	if (dividend)
	{
		trim_intarr(&dividend);
		len_dend = dividend[0];
	}

	if (divisor)
	{
		trim_intarr(&divisor);
		len_sor = divisor[0];
	}

	if (zero_result_check(dividend, divisor, &quotient))
		return (quotient);

	len_q = (len_dend - len_sor) + 1;
	/*Length of remain will never be greater than len_sor + 1*/
	remain = calloc(len_sor + 3, sizeof(*remain));
	if (!remain)
	{
		perror("Malloc fail");
		return (NULL);
	}

	quotient[0] = len_q;
	len_rem = len_sor;
	remain[0] = len_rem;
	/*Copy dividend into remain*/
	for (nd = len_dend, r = len_rem; r > 0; nd--, r--)
		remain[r] = dividend[nd];

	if (remain[len_rem] < divisor[len_sor])
	{
		if (!mplug_num_low(&remain, dividend[nd]))
		{
			free(quotient);
			return (NULL);
		}

		nd--;
	}

	for (q = len_q; q > 0; q--)
	{
		errno = 0;
		len_rem = remain[0];
		for (; q > 0 && len_rem < len_sor; q--, nd--, len_rem++)
		{
			quotient[q] = 0;
			if (!mplug_num_low(&remain, dividend[nd]))
			{
				free(quotient);
				return (NULL);
			}
		}

		hold = get_quotient(divisor);
		if (hold < 0 || !mplug_num_low(&quotient, hold))
		{
			free(quotient);
			return (NULL);
		}

		if (nd > 0)
		{
			if (!mplug_num_low(&remain, dividend[nd]))
				return (NULL);
		}
		else
			break;

		nd--;
	}

	trim_intarr(&remain);
	trim_intarr(&quotient);
	return (quotient);
}

/**
 * get_quotient - calculates the quotient number at the given index
 * @dvsor: divisor array
 *
 * Description: the current dividend is stored in the global array "remain"
 *
 * Return: the quotient, -1 on failure
 */
int64_t get_quotient(uint32_t *dvsor)
{
	uint32_t *rem_tmp = NULL, *tmp_mul = NULL, *tmp_sub = NULL, quotient_tmp[] = {1, 0, 0};
	int64_t hold = 0, over_shoot = 0;

	if (!dvsor)
		return (-1);

	hold = remain[remain[0]];
	if (remain[0] > dvsor[0])
		hold = (hold * U32_ROLL) + (int64_t)remain[remain[0] - 1];

	quotient_tmp[1] = hold / (int64_t)dvsor[dvsor[0]];
	while (!rem_tmp ||
		   (((rem_tmp[0] > dvsor[0] ||
			  (rem_tmp[0] == dvsor[0] && rem_tmp[rem_tmp[0]] >= dvsor[dvsor[0]]))) &&
			quotient_tmp[1] > 0))
	{
		if (rem_tmp && ((rem_tmp[0] >= dvsor[0]) || (rem_tmp[rem_tmp[0]] & U32_NEGBIT)))
		{
			if (rem_tmp[rem_tmp[0]] & U32_NEGBIT)
			{ /*Decrease the quotient*/
				tmp_sub = infiX_sub(tmp_mul, remain);
				if (!tmp_sub)
				{
					free(rem_tmp);
					free(tmp_mul);
					return (-1);
				}

				hold = tmp_sub[tmp_sub[0]];
				/*How many of dvsor[dvsor[0]] can fit in hold?*/
				if (hold < dvsor[dvsor[0]])
					over_shoot = 1;
				else
					over_shoot = hold / (int64_t)dvsor[dvsor[0]];

				free(tmp_sub);
				quotient_tmp[1] -= over_shoot;
			}
			else
			{ /*Increase the quotient*/
				hold = rem_tmp[rem_tmp[0]];
				/*How many of dvsor[dvsor[0]] can fit in hold?*/
				if (hold < dvsor[dvsor[0]])
					over_shoot = 1;
				else
					over_shoot = hold / (int64_t)dvsor[dvsor[0]];

				quotient_tmp[1] += over_shoot;
			}
		}

		free(tmp_mul);
		tmp_mul = NULL;
		free(rem_tmp);
		rem_tmp = NULL;
		tmp_mul = infiX_mul(dvsor, quotient_tmp);
		if (tmp_mul)
		{
			rem_tmp = infiX_sub(remain, tmp_mul);
			if (!rem_tmp)
				return (-1);
		}
		else
			return (-1);
	}

	_memcpy((char *)remain, (char *)rem_tmp, ((rem_tmp[0] + 2) * sizeof(*rem_tmp)));
	free(rem_tmp);
	free(tmp_mul);
	return (quotient_tmp[1]);
}

/**
 * zero_result_check - checks if dividend < divisor or divisor == 0
 * @dend: dividend
 * @sor: divisor
 * @qt: pointer to the quotient
 *
 * Description: If dividend < divisor quotient will be set to 0 and dividend
 * copied into the global variable "remain".
 * If divisor == 0 will just return 1
 *
 * Return: 1 if true, 0 if false
 */
int zero_result_check(uint32_t *dend, uint32_t *sor, uint32_t **qt)
{
	ssize_t lsor = -1, ldnd = -1, nd = 0;

	if (dend)
		ldnd = dend[0];

	if (sor)
		lsor = sor[0];

	if (qt)
	{
		if (!dend || ldnd < lsor || (ldnd == 1 && dend[1] == 0) ||
			(ldnd == lsor && dend[ldnd] < sor[lsor]))
		{
			if (ldnd < 1)
				ldnd = 1;

			*qt = calloc(3, sizeof(**qt));
			if (*qt)
				(*qt)[0] = 1;

			remain = calloc(ldnd + 2, sizeof(*remain));
			if (remain)
			{
				remain[0] = ldnd;
				for (nd = 1; dend && nd <= ldnd; nd++)
					remain[nd] = dend[nd];
			}

			if (!(*qt) || !remain)
				perror("Malloc fail");

			return (1);
		}
		else if (!sor || (lsor == 1 && sor[1] == 0))
			return (1);
	}

	return (0);
}
#else
/**
 * infiX_div - divides a string on numbers
 * @dividend: dividend
 * @divisor: divisor
 *
 * Description: this function also stores the remainder on any division
 * in a global variable "remain". This memory should be freed before the
 * next call of this function.
 *
 * Return: string with the result, NULL on failure
 */
uint8_t *infiX_div(uint8_t *dividend, uint8_t *divisor)
{
	size_t len_dend = strlen((char *)dividend), len_sor = strlen((char *)divisor);
	size_t len_rem = 0, nd = 0, q = 0, len_q = 0, byte_dend = 0;
	uint8_t *quotient = NULL, *temp = NULL, *new_r = NULL;

	remain = NULL;
	if (len_dend < len_sor || !len_sor || (len_dend == 1 && dividend[0] == '0'))
	{
		remain = calloc(len_dend + 1, sizeof(*remain));
		if (!remain)
		{
			perror("Malloc fail");
			return (NULL);
		}

		strcpy((char *)remain, (char *)dividend);
		return (NULL);
	}
	else if (len_sor == 1 && divisor[0] == '0')
		return (NULL);

	len_q = (len_dend - len_sor) + 1;
	quotient = calloc(len_q + 2, sizeof(*quotient));
	remain = calloc((len_dend > len_sor ? len_dend : len_sor) + 2, sizeof(*remain));
	if (!quotient || !remain)
	{
		perror("Malloc fail");
		return (NULL);
	}

	strncpy((char *)remain, (char *)dividend, len_sor);
	nd = len_sor;
	len_rem = strlen((char *)remain);
	for (q = 0; q < len_q && nd <= len_dend; q++)
	{
		errno = 0;
		if (len_rem < len_sor)
		{
			for (; q < len_q && len_rem < len_sor; q++, nd++, len_rem++)
			{
				remain[len_rem] = dividend[nd];
				quotient[q] = '0';
			}
		}

		remain[len_rem] = '\0';
		if (len_rem > len_sor)
		{
			byte_dend = strtonul((char *)remain, 2);
			quotient[q] = (byte_dend / (divisor[0] - '0')) + '0';
		}
		else
			quotient[q] = ((remain[0] - '0') / (divisor[0] - '0')) + '0';

		while (!new_r || (new_r[0] == '-' && quotient[q] > '0'))
		{
			if (new_r && new_r[0] == '-' && quotient[q] > '0')
				quotient[q] -= 1;

			free(temp);
			free(new_r);
			temp = NULL;
			new_r = NULL;
			temp = infiX_mul(divisor, &quotient[q]);
			if (temp)
			{
				new_r = infiX_sub(remain, temp);
				if (!new_r)
				{
					perror("Malloc fail");
					return (NULL);
				}
			}
			else
			{
				perror("Malloc fail");
				return (NULL);
			}
		}

		strcpy((char *)remain, (char *)(&new_r[pad_char((char *)new_r, "0")]));

		len_rem = strlen((char *)remain);
		remain[len_rem] = dividend[nd];
		len_rem++;
		nd++;
		remain[len_rem] = '\0';

		free(new_r);
		free(temp);
		temp = NULL;
		new_r = NULL;
	}

	return (quotient);
}
#endif
