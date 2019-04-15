/* FLI code to allow reception and transmission of ipbus UDP packets.

   Dave Newbold, April 2019.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>

#include <mti.h>

#define MYNAME "sim_udp"
#define IPADDR INADDR_ANY /* Always listen on 127.0.0.1:50001 for now */
#define IPPORT 50001
#define BUFSZ 512 /* In 32b words; assuming 1500 octet packets, and datagram smaller than this */
#define WAIT_USECS 50000 /* 50ms */

static uint32_t *rxbuf, *txbuf;
static int fd;
fd_set fds;
static int rxidx, txidx, rxlen;
static uint16_t rxnum, txnum;

__attribute__((constructor)) static void cinit()
{
	
	struct sockaddr_in addr;

    mti_PrintFormatted (MYNAME ": initialising\n");
    
    if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		perror(MYNAME ": socket() failed");
		mti_FatalError();		
		return;
	}
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IPPORT);
	addr.sin_addr.s_addr = htonl(IPADDR); 
	
	if(bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror(MYNAME ": bind() failed");
		mti_FatalError();		
		return;
	}

    if(NULL == (rxbuf = (uint32_t*)malloc(BUFSZ)))
    {
      perror (MYNAME ": rxbuf malloc() failed");
      mti_FatalError();
      return;
    }

    if(NULL == (txbuf = (uint32_t*)malloc(BUFSZ)))
    {
      perror (MYNAME ": txbuf malloc() failed");
      mti_FatalError();
      return;
    }

    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    
    txidx = 0;
    rxlen = 0;
    rxnum = 0;
    txnum = 0;
    
    mti_PrintFormatted(MYNAME ": listening on %s:%d, fd %d\n", inet_ntoa(addr.sin_addr), IPPORT, fd);
}

__attribute__((destructor)) static void cfini()
{
  free(rxbuf);
  free(txbuf);
  close(fd);
  mti_PrintFormatted(MYNAME ": shut down\n");
}

void get_pkt_data (int del_return,
                   int* mac_data_out,
                   int* mac_data_valid )
{
	
	static struct timeval tv;
	int s, i, len;
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);
	uint32_t ip;
	unsigned char buf[BUFSZ * 4];
	
	if (rxlen == 0){
		tv.tv_sec = 0;
		tv.tv_usec = (del_return == 0) ? 0 : WAIT_USECS;
		
		while((s = select(fd + 1, &fds, NULL, NULL, &tv)) == -1){
			if(errno != EINTR){
				perror(MYNAME ": select() failed");
				mti_FatalError();
				return;
			}
		}
		
		if(s == 0){
			*mac_data_out = 0;
			*mac_data_valid = 0;
			mti_PrintFormatted("no data\n");
			return;
		}
		else{
			mti_PrintFormatted("data\n");
			len = recvfrom(fd, buf, BUFSZ * 4, 0, (struct sockaddr *)&addr, &addrlen);
			if(len < 0){
				perror(MYNAME ": recvfrom() failed" );
				mti_FatalError();
				return;
			}
			mti_PrintFormatted(MYNAME ": received packet %d from %s:%d, length %d\n", rxnum, inet_ntoa(addr.sin_addr), addr.sin_port, len );
			if(len % 4 != 0){
				mti_PrintFormatted(MYNAME ": bad length %d\n", len);
				mti_FatalError();
				return;
			}

			rxlen = len / 4;
			*(rxbuf) = 0x20000 + rxlen; /* Packet header */
			*(rxbuf + 1) = ntohl(addr.sin_addr.s_addr); /* Header word 0: return IP address */
			*(rxbuf + 2) = (ntohs(addr.sin_port) << 16) + rxnum; /* Header word 1: return port and packet number */
			for(i = 0; i < rxlen; i++){
				*(rxbuf + i) = ntohl(buf[i * 4] << 24 + buf[i * 4 + 1] << 16 + buf[i * 4 + 2] << 8 + buf[i * 4 + 3]); /* Convert from big-endian network order to local order */
			}
			rxidx = 0;
		}
	}
	
	if(rxidx < rxlen + 3){
		*mac_data_out = *(rxbuf + rxidx);
		*mac_data_valid = 1;
		rxidx++;
	}
	else{
		mti_PrintFormatted(MYNAME ": get_mac_data packet %d finished\n", rxnum);
		rxlen = 0;
		*mac_data_out = 0;
		*mac_data_valid = 0;
		rxnum++;
	}
	
	return;
}

void store_pkt_data(int mac_data_in)
{
	
	*(txbuf + txidx) = (unsigned char)(mac_data_in & 0xff);
	txidx++;
	
	if(txidx == BUFSZ){
		mti_PrintFormatted(MYNAME ": store_pkt_data buffer overflow\n" );
		mti_FatalError();
	}

  return;
}

void send_pkt()
{
	
	struct sockaddr_in addr;
	int txlen, i;
	uint32_t w;
	unsigned char buf[BUFSZ * 4];
	
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(txbuf[1]);
	addr.sin_port = htons(txbuf[2] >> 16);
	
	for(i = 3; i < txidx; i++){
		w = htonl(*(txbuf + i)); /* Convert from local order to big-endian network order */
		buf[i * 4] = w >> 24 & 0xff;
		buf[i * 4 + 1] = w >> 16 & 0xff;
		buf[i * 4 + 2] = w >> 8 & 0xff;
		buf[i * 4 + 3] = w & 0xff;
	}
	
	txlen = sendto(fd, buf, (txlen - 3) * 4, 0, (struct sockaddr *)&addr, sizeof(addr));
	
	if (txlen < 0){
		perror(MYNAME ": sendto() failed");
		mti_FatalError();
		return;
	}
	
	if (txlen != txidx){
		mti_PrintFormatted(MYNAME ": send_pkt send packet %d write error, length sent %d\n", txnum, txlen);
		mti_FatalError();
		return;
	}
	else{
		mti_PrintFormatted(MYNAME ": send_pkt send packet %d, index %d, length %d\n", txnum, txbuf[2] & 0xffff, (txidx - 3) * 4);
		txidx = 0;
		txnum++;
	}
}
