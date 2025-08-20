
#include <string.h>
#include <getopt.h>

char *optarg = NULL;
int optind = 1;
//int opterr, optopt;
static int _optchk = 0;

int getopt(int argc, char * const argv[], const char *optstring)
{
	int i;
	
	if(_optchk)  {
		optind = 1;
		_optchk++;
	}
	
	for(i=optind; i<argc; i++) {
		char *arg = argv[i];
		char *s;
		if(arg[0] == '-') {
			if((s = strchr(optstring, (int)arg[1]))) {
				optind = i+1;
				if(s[1] == ':')	 {// arg
					optarg = argv[optind];
					optind++;
				}
			}
			return (int)arg[1];
		}
	}
	_optchk = 0;
	return -1;
}

