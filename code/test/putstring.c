#include "syscall.h"

void print_string(char *s)
{
	PutString(s);
}

int
main()
{
	char *s1 = "string\n";
	char *s2 = "can\n";
	char *s3 = "be\n";
	char *s4 = "printed\n";
	char *s5 = "whathappenswhenastringistoolongletssee...whathappenswhenastringistoolongletssee... \
				whathappenswhenastringistoolongletssee...whathappenswhenastringistoolongletssee... \
				whathappenswhenastringistoolongletssee...whathappenswhenastringistoolongletssee...\n";
	print_string(s1);
	print_string(s2);
	print_string(s3);
	print_string(s4);
	print_string(s5);
	Halt();
}