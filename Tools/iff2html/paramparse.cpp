int dummy = 0;
char *someString = &dummy;

someString += '(';
int currentParamIdx = 0;
unsigned short currentParam;
unsigned short param0;
do
{
	currentParam = *(&param0 + currentParamIdx);
	if (currentParamIdx > 0)
		someString += ' ';
	
	// Now, we iterate through the nibbles, starting at the high nibble and working our way down
	int shiftingAmountForCurrentNibble = 12;
	do
	{
		char nibbleType = 0;
		int currentNibble = (currentParam >> shiftingAmountForCurrentNibble) & 0xF;
		if (currentNibble > 9)
		{
			if (currentNibble > 15)		//((currentNibble - 10) > 5)
				nibbleType = 120;
			else
				nibbleType = currentNibble + 55;
		}
		else
			nibbleType = currentNibble + 48;
		
		char *unk = &nibbleType;
		char oldVal;
		do
		{
			oldVal = *unk;
		}
		while (*unk)
	
	
		bool outOfBounds = shiftingAmountForCurrentNibble - 4 < 0;
		shiftingAmountForCurrentNibble -= 4;
	}
	while (!outOfBounds);
	
}
while (currentParamIdx < 4);
someString += ')';