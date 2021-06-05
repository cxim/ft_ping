
#include "ft_ping.h"

void init(int ac, char **av)
{
	char *str = ft_strjoin("1", "2");
	ft_printf("%s\n", str);
}

int main(int ac, char **av)
{
	if (getuid() != 0)
	{
		printf("ft_ping: need root (sudo -s)!\n");
	}
	if (ac < 2)
		printf("error: need params!\n");
	init(ac, av);
	return 0;
}
