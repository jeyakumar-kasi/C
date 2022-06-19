// C Program (Utils)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char* split(char* str, char* delimiter, unsigned int pos)
{
	// Get matched string length
	unsigned int matchStrLen = 0;
	unsigned int matchStrStartPos = 0;
	unsigned int matchDelimiterCount = 0;
	for (int i = 0; i < strlen(str); i++) {
		if (str[i] == delimiter) {
			if (++matchDelimiterCount == pos) {
				break;
			}

			// Reset match string info.
			matchStrLen = 0;
			matchStrStartPos = i+1; // Ignore current delimiter position.
		}
		else {
			matchStrLen++;
		}
	}

	// Create the matched String
	char* matchStr;
	matchStr = (char*)malloc(matchStrLen); // Include for "trailing space" as well.
	printf("Start Position: %d\t", matchStrStartPos);
	printf("Length: %d\n", matchStrLen);
	for (int i = matchStrStartPos, j=0; i < (matchStrStartPos+matchStrLen); i++,j++) {
		matchStr[j] = str[i];
	}

	// Add trailing space
	matchStr[matchStrLen] = '\0';
	return matchStr;
}



char* substr(char* str, signed int totalChars) 
{
	char* newStr, * p1, *p2;
	newStr = (char*)malloc(totalChars);
	p1 = str;
	p2 = newStr;

	for (int i = 0; i < totalChars; i++) {
		*p2++ = *p1++;
	}

	*p2 = '\0';
	return newStr;
}

int main(int argc, char* argv) 
{
	printf("Hello C Program!\n");
	//char s1[20] = "Thursday";
	//char* s2;

	//s2 = copyString("Friday");
	//printf("%s", substr("Friday", 4));
	
	/*printf("%s\n", split("YES_2022-06-16_20:10:45", '_', 1));
	printf("%s\n", split("YES_2022-06-16_20:10:45", '_', 2));
	printf("%s\n", split("YES_2022-06-16_20:10:45", '_', 3));*/

	char* s;
	s = split("YES_2022-06-16_20:10:45", '_', 3);
	printf("Matched: %s\tLength: %d\n", s, strlen(s));

	s = split(s, ':', 2); // Minute
	printf("Matched: %s\tLength: %d\n", s, strlen(s));

	return 0;
}