
#include "ft_ping.h"
void free_params(t_params *params)
{
//	if (params->host_name)
//		free(params->host_name);
    free(params->sock);
    free(params);
}

void init(t_params *params)
{
    params->sock_fd = 0;
}

void get_arguments(int ac, char **av, t_params *params)
{
    int i;

    i = 1;
    while (i < ac)
    {
        if (av[i][0] == '-')
        {
        	if (av[i][1] == 'h')
			{
				ft_printf("Usage: ft_ping [-h help] [-v verbose] hostname");
				exit(0);
			}
        	else if (av[i][1] == 'v')
			{
        		params->flag_v = 1;
			}
        	else
			{
				ft_printf(":smile_2  error: flag ne o4en`\n");
				exit(1);
			}
        }
        else
		{
        	params->host_name = av[i];
		}
        i++;
    }
}

int main(int ac, char **av)
{
    t_params *params;
	if (getuid() != 0)
	{
		ft_printf(":smile_8 ft_ping: need root (sudo -s)!\n");
		exit(1);
	}
	if (ac < 2)
	{
		printf("error: need params!\n");
		exit(1);
	}
    params = (t_params*) ft_memalloc(sizeof(t_params));
	init(params);
	get_arguments(ac, av, params);
	free_params(params);
	return 0;
}
