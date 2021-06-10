
#include "ft_ping.h"

t_parametrs *g_parametrs;

void rtt_info()
{
	long double	rtt;

	if (gettimeofday(&g_parametrs->time.r, NULL) < 0)
	{
		ft_putstr_fd("Error: timeofday\n", 2);
		exit(0);
	}
	g_parametrs->received++;
	rtt = (g_parametrs->time.r.tv_usec - g_parametrs->time.s.tv_usec) / 1000000.0;
	rtt += (g_parametrs->time.r.tv_sec - g_parametrs->time.s.tv_sec);
	rtt *= 1000.0;
	g_parametrs->time.rtt = rtt;
	if (rtt > g_parametrs->time.max)
		g_parametrs->time.max = rtt;
	g_parametrs->time.avg += rtt;
	g_parametrs->time.sum_sqrt += rtt * rtt;
}

void init_receive_param()
{
	t_response *response;

	response = &g_parametrs->response;
	ft_bzero((void *)g_parametrs->pack.buff, 84);
	ft_bzero(response, sizeof(t_response));
	response->iovec->iov_base = (void *)g_parametrs->pack.buff;
	response->iovec->iov_len = sizeof(g_parametrs->pack.buff);
	response->msghdr.msg_iov = response->iovec;
	response->msghdr.msg_name = NULL;
	response->msghdr.msg_iovlen = 1;
	response->msghdr.msg_namelen = 0;
	response->msghdr.msg_flags = MSG_DONTWAIT;
}

void receive_from_host()
{
	int 	rec;

	init_receive_param();
	while (!g_parametrs->sig.sin_end)
	{
		rec = recvmsg(g_parametrs->sock_fd, &g_parametrs->response.msghdr, MSG_DONTWAIT);
		if (rec > 0)
		{
			g_parametrs->byte_received = rec;
			if (g_parametrs->pack.icmp->un.echo.id == g_parametrs->pid)
			{
				rtt_info();
				if (g_parametrs->host_name != g_parametrs->addr_str)
					printf("%d bytes from %s (%s): icmp_seq=%d ttl=%d time=%.2Lf ms\n", g_parametrs->byte_received - (int) sizeof(struct iphdr), g_parametrs->host_name,
						   g_parametrs->addr_str, g_parametrs->pack.icmp->un.echo.sequence, g_parametrs->pack.ip->ttl, g_parametrs->time.rtt);
				else
					printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.2Lf ms\n", g_parametrs->byte_received - (int) sizeof(struct iphdr),
						   g_parametrs->addr_str, g_parametrs->pack.icmp->un.echo.sequence, g_parametrs->pack.ip->ttl, g_parametrs->time.rtt);
			}
			else if (g_parametrs->flag_v)
			{
				char str[50];

				printf("%d bytes from %s: type=%d code=%d\n", g_parametrs->byte_received - (int) sizeof(struct iphdr),
					   inet_ntop(AF_INET, (void *)&g_parametrs->pack.ip->saddr, str, 100),
					   g_parametrs->pack.icmp->type, g_parametrs->pack.icmp->code);
			}
			return;
		}
//		if (rec == -1)
//		{
////			ft_putstr_fd("Error: recmsg\n", 2);
////			exit(1);
//			return;
//		}
	}
}

unsigned short checksum(unsigned short *icmp, int len_struct)
{
	unsigned long	res;

	res = 0;
	while (len_struct > 1)
	{
		res = res + *icmp++;
		len_struct = len_struct - sizeof(unsigned short);
	}
	if (len_struct)
	{
		res = res + *(unsigned char *)icmp;
	}
	res = (res >> 16) + (res & 0xffff);
	res = res + (res >> 16);
	return (unsigned short)(~res);
}

void send_to_host()
{
	ft_bzero((void *)g_parametrs->pack.buff, 84);
	g_parametrs->pack.ip->version = 4;
	g_parametrs->pack.ip->protocol = IPPROTO_ICMP;
	g_parametrs->pack.ip->ttl = g_parametrs->ttl;
	g_parametrs->pack.ip->ihl = sizeof(*g_parametrs->pack.ip) >> 2;
	inet_pton(AF_INET, g_parametrs->addr_str, &g_parametrs->pack.ip->daddr);
	g_parametrs->pack.icmp->code = 0;
	g_parametrs->pack.icmp->type = ICMP_ECHO;
	g_parametrs->pack.icmp->un.echo.id = g_parametrs->pid;
	g_parametrs->pack.icmp->un.echo.sequence = g_parametrs->seq++;
	g_parametrs->pack.icmp->checksum = checksum((unsigned short *)g_parametrs->pack.icmp, sizeof(struct icmphdr));
	if (sendto(g_parametrs->sock_fd, (void *)&g_parametrs->pack, 84, 0, (void *)g_parametrs->sock, sizeof(struct sockaddr_in)) < 0)
	{
		ft_putstr_fd("Error: sendto\n", 2);
		exit(0);
	}
	if (gettimeofday(&g_parametrs->time.s, NULL) < 0)
	{
		ft_putstr_fd("Error: gettimeofday\n", 2);
		exit(0);
	}
	g_parametrs->send > 1 ? gettimeofday(&g_parametrs->time.start, NULL) : 0;
	g_parametrs->send++;
	g_parametrs->sig.sin_send = 0;
}

void get_socket_fd()
{
	int opt_val;

	opt_val = 1;
	g_parametrs->sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (g_parametrs->sock_fd == -1)
	{
		ft_putstr_fd("Socket file descriptor not received!\n", 2);
		exit(1);
	}
	if (setsockopt(g_parametrs->sock_fd, IPPROTO_IP, IP_HDRINCL, &opt_val, sizeof(int)) < 0)
	{
		ft_putstr_fd("Error: setsockopt\n", 2);
		exit(0);
	}
//	ft_printf("%d\n", g_parametrs->sock_fd);
}

void ft_ping()
{
	get_socket_fd();
	printf("PING %s (%s) %d(%d) bytes of data.\n", g_parametrs->host_name, g_parametrs->addr_str, 56, 84);
	while (!g_parametrs->sig.sin_end)
	{
//		printf("new str\n");
		send_to_host();
		alarm(10);
		receive_from_host();
		usleep(1000000);
	}
}

void get_stat()
{
	struct timeval start;
	struct timeval end;
	long double time;
	long double mdev;
	double loss;

//	printf("stoped\n"); //signal worked | todo floating exception
	gettimeofday(&g_parametrs->time.end, NULL);
	start = g_parametrs->time.start;
	end = g_parametrs->time.end;
	loss = (g_parametrs->send - g_parametrs->received) / g_parametrs->send * 100.0;
	time = (end.tv_usec - start.tv_usec) / 1000000.0;
	time += (end.tv_sec - start.tv_sec);
	time *= 1000.0;

	g_parametrs->time.avg /= g_parametrs->send;
	mdev = (g_parametrs->time.sum_sqrt / g_parametrs->send) - g_parametrs->time.avg * g_parametrs->time.avg;
	mdev = sqrt(mdev);
	ft_printf("\n--- %s ping statistics ---\n", g_parametrs->host_name);
	ft_printf("%d packets transmitted, %d received, ", g_parametrs->send, g_parametrs->received);
	printf("%.0f%% packet loss, time %.0Lfms\n", loss, time);
	if (g_parametrs->time.rtt > 0.0)
		printf("rtt min/avg/max/mdev = %.3Lf/%.3Lf/%.3Lf/%.3Lf ms\n", g_parametrs->time.min, g_parametrs->time.avg, g_parametrs->time.max, mdev);
}

void sig_handler(int dummy)
{
	if (dummy == SIGINT)
	{
		g_parametrs->sig.sin_end = 1;
		get_stat();
	}
	if (dummy == SIGALRM)
		g_parametrs->sig.sin_send = 1;
}

void free_params()
{
//	if (params->host_name)
//		free(params->host_name);
//    free(params->sock);
//	free(params->addr_str); 	g_parametrs->sock = (struct sockaddr_in *)res->ai_addr;

//	free(g_parametrs);

}

void init()
{
	g_parametrs = (t_parametrs*)malloc(sizeof(t_parametrs));
	ft_bzero(g_parametrs, sizeof(t_parametrs));
	g_parametrs->sock_fd = 0;
	g_parametrs->sig.sin_send = 0;
	g_parametrs->sig.sin_end = 0;
	g_parametrs->pack.ip = (struct iphdr *)g_parametrs->pack.buff;
	g_parametrs->pack.icmp = (struct icmphdr *)(g_parametrs->pack.ip + 1);
	g_parametrs->ttl = 17;
	g_parametrs->pid = getpid();
	g_parametrs->seq = 1;
	g_parametrs->time.min = 0.0;
	g_parametrs->time.max = 0.0;
	g_parametrs->time.sum_sqrt = 0;
	g_parametrs->received = 0;
}

int get_host_info()
{
	struct addrinfo start;
	struct addrinfo *res;

	ft_bzero(&start, sizeof(start));
	start.ai_family = AF_INET;
	start.ai_socktype = SOCK_RAW;
	start.ai_protocol = IPPROTO_ICMP;
	if (getaddrinfo(g_parametrs->host_name, NULL, &start, &res) != 0)
		return (1);
	g_parametrs->sock = (struct sockaddr_in *)res->ai_addr;
	return (0);
}

void get_arguments(int ac, char **av)
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
				g_parametrs->flag_v = 1;
			}
        	else
			{
				ft_printf(":smile_2  error: flag ne o4en`\n");
				exit(1);
			}
        }
        else
		{
			g_parametrs->host_name = av[i];
        	if (get_host_info())
			{
				ft_printf("ft_ping: %s : ", g_parametrs->host_name);
				ft_printf(":smile_2  Name or service not known\n");
//				free_params(g_parametrs);
				exit(0);
			}
        	inet_ntop(AF_INET, (void *)&g_parametrs->sock->sin_addr, g_parametrs->addr_str, INET6_ADDRSTRLEN);
		}
        i++;
    }
}

int main(int ac, char **av)
{
//    t_parametrs *params;
	if (getuid() != 0)
	{
		ft_printf(":smile_8 ft_ping: need root (sudo -s)!\n");
		exit(1);
	}
	if (ac < 2)
	{
		ft_printf("error: need params!\n");
		exit(1);
	}
//	in();
//    g_parametrs = (t_parametrs*) ft_memalloc(sizeof(t_parametrs));
	init();
	get_arguments(ac, av);
	signal(SIGALRM, sig_handler);
	signal(SIGINT, sig_handler);

	ft_ping();
//	ft_printf("%s\n", g_parametrs->addr_str);
	free_params();
	return 0;
}
