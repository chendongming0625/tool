/*
 * inet_sr.c       This files contains INET4 related route manipulation methods.
 *
 * Part of net-tools, the Linux base networking tools
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 */

#include "config.h"

#if HAVE_AFINET
#include <asm/types.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <net/route.h> /* realy broken */
#include <sys/ioctl.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <resolv.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "version.h"
#include "net-support.h"
#include "pathnames.h"
#include "intl.h"
#include "net-features.h"
#include "util.h"
#include "../route.h"
#if HAVE_NEW_ADDRT
#define mask_in_addr(x) (((struct sockaddr_in *)&((x).rt_genmask))->sin_addr.s_addr)
#define full_mask(x) (x)
#else
#define mask_in_addr(x) ((x).rt_genmask)
#define full_mask(x) (((struct sockaddr_in *)&(x))->sin_addr.s_addr)
#endif

extern struct aftype inet_aftype;

static int skfd = -1;


static int usage(const int rc)
{
    FILE *fp = rc ? stderr : stdout;
    fprintf(fp, _("Usage: inet_route [-vF] del {-host|-net} Target[/prefix] [gw Gw] [metric M] [[dev] If]\n"));
    fprintf(fp, _("       inet_route [-vF] add {-host|-net} Target[/prefix] [gw Gw] [metric M]\n"));
    fprintf(fp, _("                              [netmask N] [mss Mss] [window W] [irtt I]\n"));
    fprintf(fp, _("                              [mod] [dyn] [reinstate] [[dev] If]\n"));
    fprintf(fp, _("       inet_route [-vF] add {-host|-net} Target[/prefix] [metric M] reject\n"));
    fprintf(fp, _("       inet_route [-FC] flush      NOT supported\n"));
    return (rc);
}

///////////////////////////////////////////////////start////////////////////////////////////////////////////////////////////////
/*
@author:chendongming
@date:2018 10 19
@function:for slb add system route
@example:
char *args[] = {"net","10.21.21.227","netmask",
"255.255.255.255","gw","192.168.68.2","dev","eno16777736",NULL};
set_route(1,args,NULL);
*/
int set_route(int action,char**args,char *err_msg)
{
	if (err_msg == NULL)
		return -1;
	struct rtentry rt;
	char target[128], gateway[128] = "NONE", netmask[128] = "default";
	int xflag, isnet;
	long clk_tck = ticks_per_second();
	struct sockaddr_storage sas;

	xflag = 0;

	if (!strcmp(*args, "net")) {
		xflag = 1;
		args++;
	}
	else if (!strcmp(*args, "host")) {
		xflag = 2;
		args++;
	}
	if (*args == NULL)
	{
		snprintf(err_msg, 128, _("Invalid parameter"));
		return -2;
	}

	safe_strncpy(target, *args++, (sizeof target));//目标地址 destination

												   /* Clean out the RTREQ structure. */
	memset((char *)&rt, 0, sizeof(struct rtentry));

	/* Special hack for /prefix syntax */
	{
		union {
			struct sockaddr_storage sas;
			struct sockaddr_in m;
			struct sockaddr d;
		} mask;
		int n;
		//fun()->INET_getnetmask() 查找是否存在192.168../24类型的掩码 0表示不存在
		n = inet_aftype.getmask(target, &mask.sas, netmask);//
		if (n < 0)
		{
			snprintf(err_msg, 128, _("Invalid parameter"));
			return -3;
		}
		else if (n)
		{
			rt.rt_genmask = full_mask(mask.d);
		}
	}

	/* Prefer hostname lookup is -host flag was given
	目标地址赋值给sas*/
	if ((isnet = inet_aftype.input((xflag != 2 ? 0 : 256), target, &sas)) < 0) {
		inet_aftype.herror(target);
		return (E_LOOKUP);
	}
	memcpy(&rt.rt_dst, &sas, sizeof(rt.rt_dst));

	switch (xflag) {
	case 1:
		isnet = 1;
		break;
	case 2:
		isnet = 0;
		break;
	}

	/* Fill in the other fields. */
	rt.rt_flags = (RTF_UP | RTF_HOST);//UH
	if (isnet)
		rt.rt_flags &= ~RTF_HOST;//如果是网段则取消host flags

	while (*args) {
		if (!strcmp(*args, "metric")) //暂时不考虑
		{
			int metric;
			args++;
			if (!*args || !isdigit(**args))
			{
				snprintf(err_msg, 128, _("Invalid parameter"));
				return -4;
			}
				
			metric = atoi(*args);
#if HAVE_NEW_ADDRT
			rt.rt_metric = metric + 1;
#else
			snprintf(err_msg, 128, _("inet_setroute feature NEW_ADDRT not supported"));
			ENOSUPP("inet_setroute", "NEW_ADDRT (metric)");
#endif
			args++;
			continue;
		}
		if (!strcmp(*args, "netmask"))
		{
			struct sockaddr_storage sas;
			struct sockaddr *mask = (struct sockaddr *)&sas;

			args++;
			if (!*args || mask_in_addr(rt))
			{
				snprintf(err_msg, 128, _("Invalid parameter"));
				return -5;
			}
			safe_strncpy(netmask, *args, (sizeof netmask));
			if ((isnet = inet_aftype.input(0, netmask, &sas)) < 0) {
				inet_aftype.herror(netmask);
				return (E_LOOKUP);
			}
			rt.rt_genmask = full_mask(*mask);
			args++;
			continue;
		}
		if (!strcmp(*args, "gw") || !strcmp(*args, "gateway"))
		{
			struct sockaddr_storage sas;
			args++;
			if (!*args)
			{
				snprintf(err_msg, 128, _("Invalid parameter"));
				return -6;
			}
			if (rt.rt_flags & RTF_GATEWAY)
			{
				snprintf(err_msg, 128, _("Invalid parameter"));
				return -7;
			}
			safe_strncpy(gateway, *args, (sizeof gateway));
			if ((isnet = inet_aftype.input(256, gateway, &sas)) < 0) {
				inet_aftype.herror(gateway);
				return (E_LOOKUP);
			}
			memcpy(&rt.rt_gateway, &sas, sizeof(rt.rt_gateway));
			if (isnet) {
				snprintf(err_msg,128, _("route: %s: cannot use a NETWORK as gateway!\n"),
					gateway);
				return -1;
			}
			rt.rt_flags |= RTF_GATEWAY;
			args++;
			continue;
		}
		if (!strcmp(*args, "mss") || !strcmp(*args, "mtu")) //暂时不考虑
		{
			args++;
			rt.rt_flags |= RTF_MSS;
			if (!*args)
				return usage(E_OPTERR);
			rt.rt_mss = atoi(*args);
			args++;
			if (rt.rt_mss < 64 || rt.rt_mss > 65536) {
				snprintf(err_msg,128, _("route: Invalid MSS/MTU.\n"));
				return -1;
			}
			continue;
		}
		if (!strcmp(*args, "window"))//暂时不考虑
		{
			args++;
			if (!*args)
				return usage(E_OPTERR);
			rt.rt_flags |= RTF_WINDOW;
			rt.rt_window = atoi(*args);
			args++;
			if (rt.rt_window < 128) {
				snprintf(err_msg,128, _("route: Invalid window.\n"));
				return -1;
			}
			continue;
		}
		if (!strcmp(*args, "irtt")) {
			args++;
			if (!*args)
			{
				snprintf(err_msg, 128, _("Invalid parameter"));
				return -8;
			}
			args++;
#if HAVE_RTF_IRTT
			rt.rt_flags |= RTF_IRTT;
			rt.rt_irtt = atoi(*(args - 1));
			rt.rt_irtt *= (clk_tck / 100);	/* FIXME */
#if 0				/* FIXME: do we need to check anything of this? */
			if (rt.rt_irtt < 1 || rt.rt_irtt >(120 * clk_tck)) {
				snprintf(err_msg,128, _("route: Invalid initial rtt.\n"));
				return -1;
			}
#endif
#else
			snprintf(err_msg, 128, _("inet_setroute feature RTF_IRTT not supported"));
			ENOSUPP("inet_setroute", "RTF_IRTT");
#endif
			continue;
		}
		if (!strcmp(*args, "reject")) {
			args++;
#if HAVE_RTF_REJECT
			rt.rt_flags |= RTF_REJECT;
#else
			snprintf(err_msg, 128, _("inet_setroute feature RTF_REJECT not supported"));
			ENOSUPP("inet_setroute", "RTF_REJECT");
#endif
			continue;
		}
		if (!strcmp(*args, "mod")) {
			args++;
			rt.rt_flags |= RTF_MODIFIED;
			continue;
		}
		if (!strcmp(*args, "dyn")) {
			args++;
			rt.rt_flags |= RTF_DYNAMIC;
			continue;
		}
		if (!strcmp(*args, "reinstate")) {
			args++;
			rt.rt_flags |= RTF_REINSTATE;
			continue;
		}
		if (!strcmp(*args, "device") || !strcmp(*args, "dev")) {
			args++;
			if (rt.rt_dev || *args == NULL)
				return usage(E_OPTERR);
			rt.rt_dev = *args++;//网卡赋值
			continue;
		}
		/* nothing matches */
		if (!rt.rt_dev) {
			rt.rt_dev = *args++;
			if (*args)
			{
				snprintf(err_msg, 128, _("Invalid parameter,must be last to catch typos"));
				return -1;
			}
		}
		else
		{
			snprintf(err_msg, 128, _("Invalid parameter"));
			return -9;
		}
	}

#if HAVE_RTF_REJECT
	if ((rt.rt_flags & RTF_REJECT) && !rt.rt_dev)
		rt.rt_dev = "lo";
#endif

	/* sanity checks.. */
	if (mask_in_addr(rt)) {
		__u32 mask = ~ntohl(mask_in_addr(rt));
		if ((rt.rt_flags & RTF_HOST) && mask != 0xffffffff) {
			snprintf(err_msg,128, _("route: netmask %.8x doesn't make sense with host route\n"), mask);
			return -1;
		}
		if (mask & (mask + 1)) {
			snprintf(err_msg, 128, _("route: bogus netmask %s\n"), netmask);
			return -1;
		}
		mask = ((struct sockaddr_in *) &rt.rt_dst)->sin_addr.s_addr;
		if (mask & ~mask_in_addr(rt)) {
			snprintf(err_msg, 128, _("route: netmask doesn't match route address\n"));
			return -1;
		}
	}
	/* Fill out netmask if still unset */
	if ((action == RTACTION_ADD) && rt.rt_flags & RTF_HOST)
		mask_in_addr(rt) = 0xffffffff;

	/* Create a socket to the INET kernel. */
	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		snprintf(err_msg, 128, _("socket error"));
		perror("socket");
		return (E_SOCK);
	}
	/* Tell the kernel to accept this route. */
	if (action == RTACTION_DEL) {
		if (ioctl(skfd, SIOCDELRT, &rt) < 0) {
			snprintf(err_msg, 128, _("SIOCDELRT ERROR"));
			perror("SIOCDELRT");
			close(skfd);
			return (E_SOCK);
		}
	}
	else {
		if (ioctl(skfd, SIOCADDRT, &rt) < 0) {
			snprintf(err_msg, 128, _("SIOCADDRT ERROR"));
			perror("SIOCADDRT");
			close(skfd);
			return (E_SOCK);
		}
	}
	/* Close the socket. */
	(void)close(skfd);
	return (0);
}
///////////////////////////////////////////////////////end////////////////////////////////////////////////////////////////////////

static int INET_setroute(int action, int options, char **args)
{
    struct rtentry rt;
    char target[128], gateway[128] = "NONE", netmask[128] = "default";
    int xflag, isnet;
    long clk_tck = ticks_per_second();
    struct sockaddr_storage sas;

    xflag = 0;

    if (!strcmp(*args, "#net")) {
		printf("net...\n");
	xflag = 1;
	args++;
    } else if (!strcmp(*args, "#host")) {
		printf("host...\n");
	xflag = 2;
	args++;
    }
    if (*args == NULL)
	return usage(E_OPTERR);

    safe_strncpy(target, *args++, (sizeof target));//目标地址 destination

    /* Clean out the RTREQ structure. */
    memset((char *) &rt, 0, sizeof(struct rtentry));

    /* Special hack for /prefix syntax */
    {
	union {
	    struct sockaddr_storage sas;
	    struct sockaddr_in m;
	    struct sockaddr d;
	} mask;
	int n;
	//fun()->INET_getnetmask() 查找是否存在192.168../24类型的掩码 0表示不存在
	n = inet_aftype.getmask(target, &mask.sas, netmask);//
	if (n < 0)
	    return usage(E_OPTERR);
	else if (n)
	{
		rt.rt_genmask = full_mask(mask.d);
	}
    }

    /* Prefer hostname lookup is -host flag was given
	目标地址赋值给sas*/
    if ((isnet = inet_aftype.input((xflag!=2? 0: 256), target, &sas)) < 0) {
	inet_aftype.herror(target);
	return (E_LOOKUP);
    }
    memcpy(&rt.rt_dst, &sas, sizeof(rt.rt_dst));

    switch (xflag) {
    case 1:
       isnet = 1; 
	   break;
    case 2:
       isnet = 0; 
	   break;
    }

    /* Fill in the other fields. */
    rt.rt_flags = (RTF_UP | RTF_HOST);//UH
    if (isnet)
	rt.rt_flags &= ~RTF_HOST;//如果是网段则取消host flags

    while (*args) {
	if (!strcmp(*args, "metric")) //暂时不考虑
	{
	    int metric;
	    args++;
	    if (!*args || !isdigit(**args))
		return usage(E_OPTERR);
	    metric = atoi(*args);
#if HAVE_NEW_ADDRT
	    rt.rt_metric = metric + 1;
#else
	    ENOSUPP("inet_setroute", "NEW_ADDRT (metric)");
#endif
	    args++;
	    continue;
	}
	if (!strcmp(*args, "netmask"))
	{
	    struct sockaddr_storage sas;
	    struct sockaddr *mask = (struct sockaddr *)&sas;

	    args++;
	    if (!*args || mask_in_addr(rt))
		return usage(E_OPTERR);
	    safe_strncpy(netmask, *args, (sizeof netmask));
	    if ((isnet = inet_aftype.input(0, netmask, &sas)) < 0) {
		inet_aftype.herror(netmask);
		return (E_LOOKUP);
	    }
	    rt.rt_genmask = full_mask(*mask);
	    args++;
	    continue;
	}
	if (!strcmp(*args, "gw") || !strcmp(*args, "gateway"))
	{
	    struct sockaddr_storage sas;
	    args++;
	    if (!*args)
		return usage(E_OPTERR);
	    if (rt.rt_flags & RTF_GATEWAY)
		return usage(E_OPTERR);
	    safe_strncpy(gateway, *args, (sizeof gateway));
	    if ((isnet = inet_aftype.input(256, gateway, &sas)) < 0) {
		inet_aftype.herror(gateway);
		return (E_LOOKUP);
	    }
	    memcpy(&rt.rt_gateway, &sas, sizeof(rt.rt_gateway));
	    if (isnet) {
		fprintf(stderr, _("route: %s: cannot use a NETWORK as gateway!\n"),
			gateway);
		return usage(E_OPTERR);
	    }
	    rt.rt_flags |= RTF_GATEWAY;
	    args++;
	    continue;
	}
	if (!strcmp(*args, "mss") || !strcmp(*args,"mtu")) //暂时不考虑
	{
	    args++;
	    rt.rt_flags |= RTF_MSS;
	    if (!*args)
		return usage(E_OPTERR);
	    rt.rt_mss = atoi(*args);
	    args++;
	    if (rt.rt_mss < 64 || rt.rt_mss > 65536) {
		fprintf(stderr, _("route: Invalid MSS/MTU.\n"));
		return usage(E_OPTERR);
	    }
	    continue;
	}
	if (!strcmp(*args, "window"))//暂时不考虑
	{
	    args++;
	    if (!*args)
		return usage(E_OPTERR);
	    rt.rt_flags |= RTF_WINDOW;
	    rt.rt_window = atoi(*args);
	    args++;
	    if (rt.rt_window < 128) {
		fprintf(stderr, _("route: Invalid window.\n"));
		return usage(E_OPTERR);
	    }
	    continue;
	}
	if (!strcmp(*args, "irtt")) {
	    args++;
	    if (!*args)
		return usage(E_OPTERR);
	    args++;
#if HAVE_RTF_IRTT
	    rt.rt_flags |= RTF_IRTT;
	    rt.rt_irtt = atoi(*(args - 1));
	    rt.rt_irtt *= (clk_tck / 100);	/* FIXME */
#if 0				/* FIXME: do we need to check anything of this? */
	    if (rt.rt_irtt < 1 || rt.rt_irtt > (120 * clk_tck)) {
		fprintf(stderr, _("route: Invalid initial rtt.\n"));
		return usage(E_OPTERR);
	    }
#endif
#else
	    ENOSUPP("inet_setroute", "RTF_IRTT");
#endif
	    continue;
	}
	if (!strcmp(*args, "reject")) {
	    args++;
#if HAVE_RTF_REJECT
	    rt.rt_flags |= RTF_REJECT;
#else
	    ENOSUPP("inet_setroute", "RTF_REJECT");
#endif
	    continue;
	}
	if (!strcmp(*args, "mod")) {
	    args++;
	    rt.rt_flags |= RTF_MODIFIED;
	    continue;
	}
	if (!strcmp(*args, "dyn")) {
	    args++;
	    rt.rt_flags |= RTF_DYNAMIC;
	    continue;
	}
	if (!strcmp(*args, "reinstate")) {
	    args++;
	    rt.rt_flags |= RTF_REINSTATE;
	    continue;
	}
	if (!strcmp(*args, "device") || !strcmp(*args, "dev")) {
	    args++;
	    if (rt.rt_dev || *args == NULL)
		return usage(E_OPTERR);
	    rt.rt_dev = *args++;//网卡赋值
	    continue;
	}
	/* nothing matches */
	if (!rt.rt_dev) {
	    rt.rt_dev = *args++;
	    if (*args)
		return usage(E_OPTERR);	/* must be last to catch typos */
	} else
	    return usage(E_OPTERR);
    }

#if HAVE_RTF_REJECT
    if ((rt.rt_flags & RTF_REJECT) && !rt.rt_dev)
	rt.rt_dev = "lo";
#endif

    /* sanity checks.. */
    if (mask_in_addr(rt)) {
	__u32 mask = ~ntohl(mask_in_addr(rt));
	if ((rt.rt_flags & RTF_HOST) && mask != 0xffffffff) {
	    fprintf(stderr, _("route: netmask %.8x doesn't make sense with host route\n"), mask);
	    return usage(E_OPTERR);
	}
	if (mask & (mask + 1)) {
	    fprintf(stderr, _("route: bogus netmask %s\n"), netmask);
	    return usage(E_OPTERR);
	}
	mask = ((struct sockaddr_in *) &rt.rt_dst)->sin_addr.s_addr;
	if (mask & ~mask_in_addr(rt)) {
	    fprintf(stderr, _("route: netmask doesn't match route address\n"));
	    return usage(E_OPTERR);
	}
    }
    /* Fill out netmask if still unset */
    if ((action == RTACTION_ADD) && rt.rt_flags & RTF_HOST)
	mask_in_addr(rt) = 0xffffffff;

    /* Create a socket to the INET kernel. */
    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	perror("socket");
	return (E_SOCK);
    }
    /* Tell the kernel to accept this route. */
    if (action == RTACTION_DEL) {
	if (ioctl(skfd, SIOCDELRT, &rt) < 0) {
	    perror("SIOCDELRT");
	    close(skfd);
	    return (E_SOCK);
	}
    } else {
	if (ioctl(skfd, SIOCADDRT, &rt) < 0) {
	    perror("SIOCADDRT");
	    close(skfd);
	    return (E_SOCK);
	}
    }
	printf("route add success\n");
    /* Close the socket. */
    (void) close(skfd);
    return (0);
}

int INET_rinput(int action, int options, char **args)
{
    if (action == RTACTION_FLUSH) {
	fprintf(stderr, _("Flushing `inet' routing table not supported\n"));
	return usage(E_OPTERR);
    }
    if (options & FLAG_CACHE) {
	fprintf(stderr, _("Modifying `inet' routing cache not supported\n"));
	return usage(E_OPTERR);
    }
    if ((*args == NULL) || (action == RTACTION_HELP))
	return usage(E_USAGE);

    return (INET_setroute(action, options, args));
}
#endif				/* HAVE_AFINET */
