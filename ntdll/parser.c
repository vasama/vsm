#define _CRT_SECURE_NO_WARNINGS

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define stringize_(...) #__VA_ARGS__
#define stringize(...) stringize_(__VA_ARGS__)

#define parse_error() \
	(fprintf(stderr, "Parse error (" stringize(__LINE__) ")\n"), EXIT_FAILURE)

#define check_scanf(...) \
	if (0 != scanf(__VA_ARGS__)) return parse_error()

int main()
{
	check_scanf("\nDump of file C:\\Windows\\System32\\ntdll.dll\n");
	check_scanf("\nFile Type: DLL\n");
	check_scanf("\n  Section contains the following exports for ntdll.dll\n");
	check_scanf("\n  %*" PRIX32 " characteristics\n");
	check_scanf("    %*" PRIX32 " time date stamp\n");
	check_scanf("    %*" PRIu32 ".%*" PRIu32 " version\n");
	check_scanf("    %*" PRIu32 " ordinal base\n");
	check_scanf("    %*" PRIu32 " number of functions\n");
	check_scanf("    %*" PRIu32 " number of names\n");
	check_scanf("    ordinal hint RVA      name\n");

	printf("EXPORTS\n");
	while (1)
	{
		char name[256 + 1];
		if (1 != scanf("    %*" PRIu32 " %*" PRIX32 " %*" PRIX32 " %s\n", name)) break;
		
		if (name == strstr(name, "Nt") || name == strstr(name, "Rtl"))
		{
			printf("%s\n", name);
		}
	}

	check_scanf("  Summary\n");

	return EXIT_SUCCESS;
}
