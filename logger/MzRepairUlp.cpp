

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#include "FilePackType.h"
#include "MzToolsUlp.h"

#ifndef NO_UNITEX_LOGGER


void do_help()
{
	printf("Usage : MzRepairUlp file_corrupted.ulp [dest.ulp] [temp.ulp]\n\n" );
}

int main_MzRepairUlp(int argc,char* const argv[])
{

	int retRepair=0;


    if (argc==1)
    {
        do_help();
        return 0;
    }
    else
    {
		const char*src=argv[1];
		char*dst=NULL;
		char*tmp=NULL;
		uLong nRecovered=0;
		uLong bytesRecovered=0;
		if (argc>2)
		{
			dst=strdup(argv[2]);
		}
		else
		{
			dst=(char*)malloc(strlen(src)+0x10);
			strcpy(dst,src);
			strcat(dst,".repair");
		}

		if (argc>3)
		{
			tmp=strdup(argv[3]);
		}
		else
		{
			tmp=(char*)malloc(strlen(dst)+0x10);
			strcpy(tmp,dst);
			strcat(tmp,".tmp");
		}

		retRepair=ulpRepair(src,(const char*)dst,(const char*)tmp,&nRecovered,&bytesRecovered);
		if (retRepair!=0)
			printf("error in UlpRepair : return value = %d",retRepair);
		free(tmp);
		free(dst);
	}
	return retRepair;
}

#endif
