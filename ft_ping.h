//
// Created by cxim1 on 05.06.2021.
//

#ifndef FT_PING__FT_PING_H_
#define FT_PING__FT_PING_H_

#include <stdio.h>
#include <unistd.h>
#include "ft_printf/includes/printf.h"

typedef struct s_params
{
//    t_pckt pckt;
    struct sockaddr_in *sock;
    int 	flag_v;
    int     sock_fd;
    char 	*host_name;
}               t_params;
#endif //FT_PING__FT_PING_H_
