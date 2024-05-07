#include<stdio.h>
struct st
{
	int id;
	char name[20];
};

void main()
{
	struct st a, a2;

	scanf("%d %s",&a.id, a.name);
	scanf("%d %s",&a2.id, a2.name);
		
	printf("%d, %s\n",a.id, a.name);
	printf("%d, %s\n",a2.id, a2.name);
}
