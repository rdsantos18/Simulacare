#include <esp_event.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <esp_system.h>
#include "esp_err.h"
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <nvs_flash.h>
#include "lwip/ip4_addr.h"

#include "mongoose.h"
#include "sdkconfig.h"
#include "ota.h"
#include "www/base_html.h"
#include "www/reboot_html.h"
#include "www/firmware_html.h"
#include "www/status_html.h"
#include "www/calib_html.h"

static char tag []="webserver";

extern esp_err_t save_config(void);

char clientIPAddressStr[INET6_ADDRSTRLEN];

/**
 * Convert a Mongoose event type to a string.
 */
char *mongoose_eventToString(int ev) {
	static char temp[100];
	switch (ev) {
	case MG_EV_CONNECT:
		return "MG_EV_CONNECT";
	case MG_EV_ACCEPT:
		return "MG_EV_ACCEPT";
	case MG_EV_CLOSE:
		return "MG_EV_CLOSE";
	case MG_EV_SEND:
		return "MG_EV_SEND";
	case MG_EV_RECV:
		return "MG_EV_RECV";
	case MG_EV_HTTP_REQUEST:
		return "MG_EV_HTTP_REQUEST";
	case MG_EV_HTTP_REPLY:
		return "MG_EV_HTTP_REPLY";
	case MG_EV_MQTT_CONNACK:
		return "MG_EV_MQTT_CONNACK";
	case MG_EV_MQTT_CONNACK_ACCEPTED:
		return "MG_EV_MQTT_CONNACK";
	case MG_EV_MQTT_CONNECT:
		return "MG_EV_MQTT_CONNECT";
	case MG_EV_MQTT_DISCONNECT:
		return "MG_EV_MQTT_DISCONNECT";
	case MG_EV_MQTT_PINGREQ:
		return "MG_EV_MQTT_PINGREQ";
	case MG_EV_MQTT_PINGRESP:
		return "MG_EV_MQTT_PINGRESP";
	case MG_EV_MQTT_PUBACK:
		return "MG_EV_MQTT_PUBACK";
	case MG_EV_MQTT_PUBCOMP:
		return "MG_EV_MQTT_PUBCOMP";
	case MG_EV_MQTT_PUBLISH:
		return "MG_EV_MQTT_PUBLISH";
	case MG_EV_MQTT_PUBREC:
		return "MG_EV_MQTT_PUBREC";
	case MG_EV_MQTT_PUBREL:
		return "MG_EV_MQTT_PUBREL";
	case MG_EV_MQTT_SUBACK:
		return "MG_EV_MQTT_SUBACK";
	case MG_EV_MQTT_SUBSCRIBE:
		return "MG_EV_MQTT_SUBSCRIBE";
	case MG_EV_MQTT_UNSUBACK:
		return "MG_EV_MQTT_UNSUBACK";
	case MG_EV_MQTT_UNSUBSCRIBE:
		return "MG_EV_MQTT_UNSUBSCRIBE";
	case MG_EV_WEBSOCKET_HANDSHAKE_REQUEST:
		return "MG_EV_WEBSOCKET_HANDSHAKE_REQUEST";
	case MG_EV_WEBSOCKET_HANDSHAKE_DONE:
		return "MG_EV_WEBSOCKET_HANDSHAKE_DONE";
	case MG_EV_WEBSOCKET_FRAME:
		return "MG_EV_WEBSOCKET_FRAME";
	}
	sprintf(temp, "Unknown event: %d", ev);
	return temp;
} //eventToString


// Convert a Mongoose string type to a string.
char *mgStrToStr(struct mg_str mgStr) {
	char *retStr = (char *) malloc(mgStr.len + 1);
	memcpy(retStr, mgStr.p, mgStr.len);
	retStr[mgStr.len] = 0;
	return retStr;
} // mgStrToStr

// Mongoose event handler.
void mongoose_event_handler(struct mg_connection *nc, int ev, void *evData) {
	struct mbuf *io = &nc->recv_mbuf;
	char *ri = mongoose_eventToString(ev);
    if(strcmp(ri, "MG_EV_MQTT_CONNACK" ) != 0) {
    	printf("Evento: %s\n", ri);
    }
	switch (ev) {
 		case MG_EV_HTTP_REQUEST: {
			struct http_message *message = (struct http_message *) evData;

			char *uri = mgStrToStr(message->uri);

			if (strcmp(uri, "/time") == 0) {
				char payload[256];
				struct timeval tv;
				gettimeofday(&tv, NULL);
				sprintf(payload, "Time since start: %d.%d secs", (int)tv.tv_sec, (int)tv.tv_usec);
				int length = strlen(payload);
				mg_send_head(nc, 200, length, "Content-Type: text/plain");
				mg_printf(nc, "%s", payload);
			} else if (strcmp(uri, "/") == 0) {
				printf("Page Index.html");
				mg_send_head(nc, 200, base_html_len, "Content-Type: text/html");
				mg_send(nc, base_html, base_html_len);
			// Page Reboot
			} else if (strcmp(uri, "/reboot.html") == 0) {
				printf("Page reboot_html\n");
				mg_send_head(nc, 200, reboot_html_len, "Content-Type: text/html");
				mg_send(nc, reboot_html, reboot_html_len);
			} else if (strcmp(uri, "/status.html") == 0) {
				printf("Page status_html\n");
				mg_send_head(nc, 200, status_html_len, "Content-Type: text/html");
				mg_send(nc, status_html, status_html_len);
			} else if (strcmp(uri, "/calib.html") == 0) {
				printf("Page calib_html\n");
				mg_send_head(nc, 200, calib_html_len, "Content-Type: text/html");
				mg_send(nc, calib_html, calib_html_len);
			} else if (strcmp(uri, "/firmware.html") == 0) {
				printf("Page firmware_html\n");
				mg_send_head(nc, 200, firmware_html_len, "Content-Type: text/html");
				mg_send(nc, firmware_html, firmware_html_len);
				xTaskCreate(&ota_example_task, "ota_example_task", 8192, NULL, 5, NULL);
			// Not Found 404
			}	else {
				printf("Page Not Found: %s\n", uri);
				mg_send_head(nc, 404, 0, "Content-Type: text/plain");
			}
			nc->flags |= MG_F_SEND_AND_CLOSE;
			free(uri);
			break;
 		}
 		case MG_EV_RECV:
 			if(strstr(io->buf, "POST /reboot") != 0) {
 				printf("POST Reboot\n");
 				if(strstr(io->buf, "VARRESET=yes") != 0) {
 					printf("Restarting now.\n");
 					esp_err_t err = save_config();
 					if(err != ESP_OK) printf("Erro Save_Config\n");
 					else printf("Save_Config OK!!\n");
 					esp_restart();
 				}
 			}
 			if(strstr(io->buf, "POST / ") != 0) {
 				printf("POST WIFI\n");
 	 			printf("TST:%s\n" ,strstr(io->buf, "Mode"));
 			}
  			break;
 		case MG_EV_SEND:
 			break;
 		case MG_EV_ACCEPT:
 			mg_conn_addr_to_str( nc, clientIPAddressStr, sizeof( clientIPAddressStr ), MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_REMOTE );
 			printf("IP Remote: %s\n", clientIPAddressStr);
 			break;
 		default:
 			break;
	} // End of switch
} // End of mongoose_event_handler


// FreeRTOS task to start Mongoose.
void mongooseTask(void *data) {
	ESP_LOGD(tag, "Mongoose task starting");
	struct mg_mgr mgr;
	ESP_LOGD(tag, "Mongoose: Starting setup");
	mg_mgr_init(&mgr, NULL);
	ESP_LOGD(tag, "Mongoose: Succesfully inited");
	struct mg_connection *c = mg_bind(&mgr, ":80", mongoose_event_handler);
	ESP_LOGD(tag, "Mongoose Successfully bound");
	if (c == NULL) {
		ESP_LOGE(tag, "No connection from the mg_bind()");
		vTaskDelete(NULL);
		return;
	}
	mg_set_protocol_http_websocket(c);

	while (1) {
		mg_mgr_poll(&mgr, 1000);
	}
} // mongooseTask
