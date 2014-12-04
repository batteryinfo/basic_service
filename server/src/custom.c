#include <Ecore.h>
#include <unistd.h>

#include <stdio.h>
#include <dlog.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <systemd/sd-daemon.h>
#include <sysman.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include "ss_log.h"

#define LOG_TAG		"SAMPLE" //DLOG Name
#define DLOG_ERR		DLOG_ERROR
#define PRT_ERR(fmt, arg...) \
	do { SLOG(LOG_ERR, LOG_TAG, fmt, ##arg); } while (0)

#define SAMPLE_SOCKET_PATH "/tmp/sample"

static Ecore_Fd_Handler *custom_efd = NULL;
static int __custom_start(void);

//IPC Structure
struct message_data{
	int val;
};


static int read_message(int fd, struct message_data *msg)
{
	int i;

	read(fd, msg, sizeof(struct message_data));
	
	return 0;
}

static int custom_cb(void *data, Ecore_Fd_Handler * fd_handler)
{
	int fd;
	struct message_data *msg; //-------

	PRT_ERR("[gandan] %s: socket data received\n", __FUNCTION__);

	struct sockaddr_un client_address;
	int client_sockfd;
	int client_len;


	if (!ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ)) {
		PRT_TRACE_ERR
		    ("ecore_main_fd_handler_active_get error , return\n");
		return 1;
	}

	fd = ecore_main_fd_handler_fd_get(fd_handler);
	msg = malloc(sizeof(struct message_data)); //-------
	if (msg == NULL) {
		PRT_TRACE_ERR("%s : Not enough memory", __FUNCTION__);
		return 1;
	}

	client_len = sizeof(client_address);
	client_sockfd = accept(fd, (struct sockaddr *)&client_address, (socklen_t *)&client_len);

	if (client_sockfd == -1) {
		PRT_TRACE_ERR("socket accept error");
		free(msg);
		return -1;
	}

	read_message(client_sockfd, msg);
	PRT_ERR("[gandan] received data : %d\n", msg->val);

	PRT_ERR("[gandan] %s: socket data receive end\n", __FUNCTION__);
	free(msg);

	return 1;
}

static int custom_server_init(void)
{
	int fd;
	struct sockaddr_un serveraddr;

	PRT_ERR("[gandan] custom server init!!\n");
	PRT_ERR("[gandan] %s: custom_server_init\n", __FUNCTION__);

	if (access(SAMPLE_SOCKET_PATH, F_OK) == 0)
		unlink(SAMPLE_SOCKET_PATH);

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0) {
		PRT_ERR("%s: socket create failed\n", __FUNCTION__);
		return -1;
	}

	bzero(&serveraddr, sizeof(struct sockaddr_un));
	serveraddr.sun_family = AF_UNIX;
	strncpy(serveraddr.sun_path, SAMPLE_SOCKET_PATH, sizeof(serveraddr.sun_path));

	if (bind(fd, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr)) < 0) {
		PRT_ERR("%s: socket bind failed\n", __FUNCTION__);
		close(fd);
		return -1;
	}

	if (chmod((SAMPLE_SOCKET_PATH), (S_IRWXU | S_IRWXG | S_IRWXO)) < 0)	
		/* 0777 */
		PRT_ERR("failed to change the socket permission");

	if (listen(fd, 5) < 0) {
		PRT_ERR("failed to listen");
		close(fd);
		return -1;
	}
	PRT_ERR("[gandan]  socket create & listen ok\n");

	return fd;
}


int sample_custom_init(void)
{
	return __custom_start();
}


static int __custom_start(void){
	int fd;

	fd = custom_server_init();
	if ( fd < 0 )
		return -1;

	//ECORE_FD_READ , ECORE_FD_WRITE
	custom_efd = ecore_main_fd_handler_add(fd, ECORE_FD_READ, custom_cb, NULL, NULL, NULL);

	if (!custom_efd ) {
		PRT_TRACE_ERR("error ecore_main_fd_handler_add");
		return -1;
	}
	return fd;

}
