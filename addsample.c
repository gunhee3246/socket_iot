/* This is sample C code as an example */
/* Example of loading stats data into InfluxDB in its Line Protocol format over a network using HTTP POST */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h> 
#include <wiringPi.h> 
#include <wiringPiSPI.h> 

/* YOU WILL HAVE TO CHANGE THESE FIVE LINES TO MATCH YOUR INFLUXDB CONFIG */
#define PORT        PORT        /* Port number as an integer - web server default is 80 */
#define IP_ADDRESS IP_ADDRESS    /* IP Address as a string */
#define DATABASE DATABASE
#define USERNAME USERNAME
#define PASSWORD PASSWORD

/* client endpoint details for a tag: replace with your hostname or use gethostname() */
#define HOSTNAME "admin"

#define SECONDS 2
#define LOOPS   1000
#define BUFSIZE 8196
#define CS_MCP3208 8 //GPIO 8 
#define SPI_CHANNEL 0 
#define SPI_SPEED 1000000 //1Mhz
 
// spi communication with Rpi and get sensor data 
 
int read_mcp3208_adc(unsigned char adcChannel) 
{
    unsigned char buff[3];
    int adcValue = 0;
    
    buff[0] = 0x06 | ((adcChannel & 0x07) >> 2);
    buff[1] = ((adcChannel & 0x07) << 6);
    buff[2] = 0x00;
    
    digitalWrite(CS_MCP3208, 0);
    wiringPiSPIDataRW(SPI_CHANNEL, buff, 3);
    
    buff[1] = 0x0f & buff[1];
    adcValue = (buff[1] << 8 ) | buff[2];
    
    digitalWrite(CS_MCP3208, 1);
    
    return adcValue;
}

int pexit(char * msg)
{
    perror(msg);
    exit(1);
}

int main()
{
    int i;
    int sockfd;
    int loop;
    int ret;
    char header[BUFSIZE];
    char body[BUFSIZE];
    char result[BUFSIZE];
    unsigned char adcChannel_light = 0;
	int adcValue_light = 0;
	float cout_light;
	float vout_oftemp;
	float percentrh = 0;
	float supsiondo = 0;
    static struct sockaddr_in serv_addr; /* static is zero filled on start up */
    
    printf("start\n");
    if(wiringPiSetupGpio() == -1)
    {
       fprintf(stdout, "Unable to start wiringPi :%s\n", strerror(errno));
        return 1;
    }
    
    if(wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1)
    {
        fprintf(stdout, "wiringPiSPISetup Failed :%s\n", strerror(errno));
        return 1;
    }
    
    pinMode(CS_MCP3208, OUTPUT);

    printf("Connecting socket to %s and port %d\n", IP_ADDRESS, PORT);
    if((sockfd = socket(AF_INET, SOCK_STREAM,0)) <0) 
        pexit("socket() failed");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    serv_addr.sin_port = htons(PORT);
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <0) 
        pexit("connect() failed");

    for(loop=0;i<LOOPS; i++) {    
        adcValue_light = read_mcp3208_adc(adcChannel_light);
        printf("==============================\n");
        printf("light sensor = %u\n", adcValue_light);
        sprintf(body, "light,host=%s data=%.3f   \n", HOSTNAME, (double)adcValue_light);

        /* Note spaces are important and the carriage-returns & newlines */
        /* db= is the datbase name, u= the username and p= the password */
        sprintf(header, 
            "POST /write?db=%s&u=%  s&p=%s HTTP/1.1\r\nHost: influx:8086\r\nContent-Length: %ld\r\n\r\n", 
             DATABASE, USERNAME, PASSWORD, strlen(body));

        printf("Send to InfluxDB the POST request bytes=%d \n\n%s\n",strlen(header), header);
        write(sockfd, header, strlen(header));
        if (ret < 0)
            pexit("Write Header request to InfluxDB failed");

        printf("Send to InfluxDB the data bytes=%d \n\n%s\n",strlen(body), body);
        ret = write(sockfd, body, strlen(body));
        if (ret < 0)
            pexit("Write Data Body to InfluxDB failed");

        /* Get back the acknwledgement from InfluxDB */
        /* It worked if you get "HTTP/1.1 204 No Content" and some other fluff */
        ret = read(sockfd, result, sizeof(result));
        if (ret < 0)
            pexit("Reading the result from InfluxDB failed");
        result[ret] = 0; /* terminate string */
        printf("Result returned from InfluxDB. Note:204 is Success\n\n%s\n",result);

        printf(" - - - sleeping for %d secs\n\n",SECONDS);
        sleep(SECONDS);
    }
    close(sockfd);
}

