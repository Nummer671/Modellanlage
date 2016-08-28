/**************************************************************************************************
	Module:		simCommunication
	Purpose:	Stellt das Kommunikationsinterface zum Modellbahn Simulator bereit
	System:		Visual Studio / Windows - FreeRTOS Simulator
	Date:		21/05/2014
	Log:		
	23/05/2014
	Anwesenheitssensoren gehen von 0-17 und nicht von 1-18 auf der Ereignisid Seite
	Sensorereignisse nur erzeugen wenn er als Belegt gemeldet wurde

**************************************************************************************************/
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "util/simCommunication.h"

// link against ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

// Protokoll Flags
#define DBefehl		0x01
#define DAntwort	0x02
#define DEreignis	0x03


#define DNACK		0x00
#define DACK		0x01

// Protokoll Befehle
#define DIPBekanntmachung	0x10
#define DStoppstellen		0x12
#define DServoAbzweigungen	0x13
#define DAktivitaetStarten	0x14

// Protokoll Ereignisse
#define DSensorEreignis		0x22
#define DAktivitaetBeendet	0x23
#define DEndSequenz			0x24
#define DStartFreigabe		0x25

/* Funktionsprototypen ***************************************************************************/
static int receiveFrom(int *whatHappened);
/* UDP Kommunikation *****************************************************************************/
static int udpOpen(void);
static int recvFromTimeOutUDP(SOCKET socket, long sec, long usec);
static int udpReceive(char *data, int datalen);
static int udpSend(const char *data, int datalen);

/* Protokoll Parser *****************************************************************************/
static int cmdToProtocol(char *Buffer, int BufferLen, int who, int command);
static int protocolToCmd(char *Buffer, int BufferLen);

/* Lokale Konstanten *****************************************************************************/
// IP auf der gelauscht wird
static const char	bindIP[]				= "0.0.0.0";
// IP zu der gesendet wird
static const char	destinationIP[]			= "127.0.0.1";
// port auf dem gelauscht und gesendet wird
static const unsigned short bindPort		= 50001;
// port zu dem gesendet wird
static const unsigned short destinationPort	= 50000;

// Informationen über die Windows Sockets
static WSADATA			wsaData;
// socket auf dem gearbeitet wird
static SOCKET			receivingSocket = INVALID_SOCKET;
// Lausch adressinfo
static SOCKADDR_IN		bindAddr;
// Ziel adressinfo
static SOCKADDR_IN		destinationAddr;

static xQueueHandle		recQueue = NULL;
static xSemaphoreHandle xSopenUdp = NULL;
static xSemaphoreHandle xSisOpenUdp = NULL;


void taskSimCom( void *pvParameters )
{
	char sendBuffer[3] = {DIPBekanntmachung,DBefehl,DACK};
	char receiveBuffer[10];
	int dataLen = 0;
	int data = 0;
	(void*)pvParameters;
	recQueue = xQueueCreate(5, sizeof(int));
	if ( recQueue == 0)
	{
		printf_s("SimCom: Error creating Queue failed");
		return;
	}
	xSopenUdp = xSemaphoreCreateBinary();
	xSisOpenUdp = xSemaphoreCreateBinary();
	xSemaphoreTake(xSopenUdp, portMAX_DELAY);

	if(udpOpen() < 0)
	{
		return;
	}
	udpSend(sendBuffer, sizeof(sendBuffer));
	dataLen = udpReceive(receiveBuffer,sizeof(receiveBuffer));
	if(dataLen > 0)
	{
		if(protocolToCmd(receiveBuffer, dataLen) != 31)
		{
			printf_s("SimCom: Error connectionrequest denied");
			vTaskDelete(NULL);
		}
	}
	else
	{
		printf_s("SimCom: Error connectionrequest denied");
		vTaskDelete(NULL);
	}
	xSemaphoreGive(xSisOpenUdp);
	while(1)
	{
		receiveFrom(&data);
		if(xQueueSend(recQueue,&data, 0) == errQUEUE_FULL)
		{
			printf_s("SimCom: Error receivebuffer overflow\n");
		}
	}
}

/* public functions ******************************************************************************/
xQueueHandle initSystem(void)
{
	xSemaphoreGive(xSopenUdp);
	xSemaphoreTake(xSisOpenUdp, portMAX_DELAY);
	return recQueue;
}

int	sendTo(int who, int command)
{
	char sendBuffer[10];
	int dataLen = 0;
	int statusCode = -1;
	dataLen = cmdToProtocol(sendBuffer, sizeof(sendBuffer), who, command);
	if(dataLen > 0)
	{
		statusCode = udpSend(sendBuffer, dataLen);
	}
	return statusCode;
}


void shutdownSystem(void)
{
	closesocket(receivingSocket);
	receivingSocket= INVALID_SOCKET;
	WSACleanup();
}


static int receiveFrom(int *whatHappened)
{
	char receiveBuffer[10];
	int statusCode;
	*whatHappened = -1;
	do
	{
		statusCode = udpReceive(receiveBuffer, sizeof(receiveBuffer));
		if(statusCode > 0)
		{
			*whatHappened = protocolToCmd(receiveBuffer, statusCode);
		}
	}while(*whatHappened < 0);
	return statusCode;
}

/* Funktionen zur Udp Kommunikation **************************************************************/
static int udpOpen(void)
{
	taskENTER_CRITICAL();
	// WinSock mit Version 2.2 Initialisieren
	if( WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
	{
		printf_s("SimCom: WSAStartup failed with error %ld\n", WSAGetLastError());
		taskEXIT_CRITICAL();
		return -1;
	}
 
	// Erstelle einen neuen Socket
	receivingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	// falls erstellen nicht funktioniert hat
	if (receivingSocket == INVALID_SOCKET)
	{
          printf_s("SimCom: Error at socket(): %ld\n", WSAGetLastError());
          WSACleanup();
		  taskEXIT_CRITICAL();
          return -1;
	}

	// Erstelle Daten für Adressbindung
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_addr.S_un.S_addr = inet_addr(bindIP);
	bindAddr.sin_port = htons(bindPort);

	// Verbinde die Adressinformationen mit dem Socket
	// Ab jetzt ist senden und empfangen über den Socket möglich
	if (bind(receivingSocket, (SOCKADDR *)&bindAddr, sizeof(bindAddr)) == SOCKET_ERROR)
	{
		printf_s("SimCom: bind() failed! Error: %ld.\n", WSAGetLastError());
		// Socket schließen
		closesocket(receivingSocket);
		WSACleanup();
		taskEXIT_CRITICAL();
		return -1;
	}

	// Erstelle Daten für Ziel
	destinationAddr.sin_family = AF_INET;
	destinationAddr.sin_addr.S_un.S_addr = inet_addr(destinationIP);
	destinationAddr.sin_port = htons(destinationPort);
	taskEXIT_CRITICAL();
	return 0;
}

// Rückgabe werte
// -1: Fehler
//  0: Keine daten verfügbar
// >0: Daten bereit zum lesen
static int recvFromTimeOutUDP(SOCKET socket, long sec, long usec)
{
    // Setup timeval variable
    struct timeval timeout;
    struct fd_set fds;

    timeout.tv_sec = sec;
    timeout.tv_usec = usec;
    // Setup fd_set structure
    FD_ZERO(&fds);
    FD_SET(socket, &fds);
    return select(0, &fds, 0, 0, &timeout);
}

static int udpReceive(char *data, int datalen)
{
	int				statusCode = 0;
	SOCKADDR_IN		senderAddr;
	int				senderAddrSize = sizeof(senderAddr);
	do
	{
		// gehe in kritischen Bereich, damit der FreeRTOS Task nicht innerhalb eines Calls in den Windows Kernel von der CPU genommen wird
		taskENTER_CRITICAL();
		// prüfe ob ein Datagram verfügbar ist
		statusCode = recvFromTimeOutUDP(receivingSocket,0,0);
		if(statusCode > 0)
		{
			// empfange Datagram
			statusCode = recvfrom(receivingSocket, data, datalen, 0, (SOCKADDR *)&senderAddr, &senderAddrSize);
			if ( statusCode < 0 )
			{
				printf_s("SimCom: Failed to receive data: %ld\n",  WSAGetLastError());
				// verlasse kritischen Bereich und die Empfangsfunktion
				taskEXIT_CRITICAL();
				break;
			}
		}
		// verlasse kritischen Bereich
		taskEXIT_CRITICAL();
		if(statusCode == 0)
		{
			// warte 5 Ticks vor erneutem versuch
			vTaskDelay((portTickType)5);
		}
	} while(statusCode == 0);
	return statusCode;
}

static int udpSend(const char *data, int datalen)
{
	int statusCode;
	// gehe in kritischen Bereich, damit der FreeRTOS Task nicht innerhalb eines Calls in den Windows Kernel von der CPU genommen wird
	taskENTER_CRITICAL();
	// sende Datagram
	statusCode = sendto(receivingSocket, data, datalen, 0,(const SOCKADDR *)&destinationAddr, sizeof(destinationAddr));
	if ( statusCode < 0 )
	{
		printf_s("SimCom: Failed to send data: %ld\n",  WSAGetLastError());
	}
	taskEXIT_CRITICAL();
	return statusCode;
}


/* Protokoll Parser Funktionen *******************************************************************/
static int cmdToProtocol(char *Buffer, int BufferLen, int who, int command)
{
	int statusCode = -1;
	// Stopp-Stelle
	if(who >= 0 && who <= 7)
	{
		if(BufferLen >=4)
		{
			// ist das Kommando im richtigen Bereich?
			if(command >= 0 && command <= 1)
			{
				// Stoppstellen von 1-8 und nicht von 0-7
				who +=1; 
				Buffer[0] = DStoppstellen;
				Buffer[1] = DBefehl;
				Buffer[2] = (char)who;
				Buffer[3] = (char)command;
				statusCode = 4;
			}
		}
	}
	// Weiche
	else if(who >= 10 && who <= 12)
	{
		if(BufferLen >= 4)
		{
			// ist das Kommando im richtigen Bereich?
			if(command >= 1 && command <= 3)
			{
				// ServoIds von 1-3
				who -=9;
				Buffer[0] = DServoAbzweigungen;
				Buffer[1] = DBefehl;
				Buffer[2] = (char)who;
				Buffer[3] = (char)command;
				statusCode = 4;
			}
		}
	}
	// Aktivität Starten
	else if(who >= 20 && who <= 25)
	{
		if(BufferLen >= 3)
		{
			// Kommando wird Ignoriert
			// Aktivitäten von 1-6
			who -=19;
			Buffer[0] = DAktivitaetStarten;
			Buffer[1] = DBefehl;
			Buffer[2] = (char)who;
			statusCode = 3;
		}
	}
	return statusCode;
}

static int protocolToCmd(char *Buffer, int BufferLen)
{
	int statusCode = -1;
	if(BufferLen > 0)
	{
		switch(Buffer[0])
		{
		case DIPBekanntmachung:
			if(BufferLen == 3)
			{
				if(Buffer[1] == DAntwort && Buffer[2] == DACK)
				{
					statusCode = 31;
				}
			}
			break;
		case DSensorEreignis:
			if(BufferLen == 4)
			{
				if(Buffer[1] == DEreignis && Buffer[2] >= 1 && Buffer[2] <= 18 && Buffer[3] == 1)
				{
					statusCode = Buffer[2] - 1;
				}
			}
			break;
		case DAktivitaetBeendet:
			if(BufferLen == 3)
			{
				if(Buffer[1] = DEreignis && Buffer[2] >= 1 && Buffer[2] <= 6)
				{
					statusCode = Buffer[2] + 19;
				}
			}
			break;
		case DStartFreigabe:
			if(BufferLen == 2)
			{
				if(Buffer[1] == DEreignis)
				{
					statusCode = 30;
				}
			}
			break;
		case DEndSequenz:
			if (BufferLen == 2)
			{
				if (Buffer[1] == DEreignis)
				{		
						statusCode = 31;
				}
			}
		default:	
			// just unknown
			break;
		}
	}
	return statusCode;
}

