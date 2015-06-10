#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define PIR_PIN 0
#define DHT11_PIN 1
#define LDR_PIN 4
#define LED_PIN 7

#define BILLION 1000000000L
#define MILLION 1000000L

FILE *fd = NULL;
char fname [30];
time_t rawtime;
struct tm *timeinfo;
char buffer[32];

void led_write_val(int val)
{
    pinMode (LED_PIN, OUTPUT);
    digitalWrite (LED_PIN, val);
}

void pir_read_val()
{
    static int last_pir_val=2;

    int pir_val=0;
    uint8_t lststate=HIGH;
    uint8_t counter=0;
    uint8_t j=0,i;

    pir_val = digitalRead(PIR_PIN);
    // verify cheksum and print the verified data
    printf("%d ", pir_val); 
    if (pir_val != 0) {
        last_pir_val = pir_val;
        if (fname[0] != '\0') {
            time (&rawtime);
            timeinfo = localtime (&rawtime);
            strftime (buffer, 26, "%H:%M:%S", timeinfo);

            fd = fopen (fname, "a");
            fprintf(fd, "%s,,, %d\n", buffer, pir_val);
            fclose (fd);
        }
    }
}

void ldr_read_val()
{
#define MAX_LDR_LOOPS 1000000
// tested in the room with lights off
// for day time testing, use lower values such as 5
#define LOW_LIGHT_CUTOFF 100

    /* C = 1uF, R = (2.2K+PhotoResistor)Ohm */
    struct timespec startt, endt;

    int i, ldr = 0;
    uint64_t diff = 0;

    // discharge the capacitor (drive the GPIO to GND)
    pinMode (LDR_PIN, OUTPUT);
    digitalWrite (LDR_PIN, LOW);
    delay (1000); // sleep for 1 second

    // start time and change mode to input, capacitor starts charging
    clock_gettime (CLOCK_MONOTONIC, &startt);
    pinMode (LDR_PIN,INPUT);

    // poll the LDR PIN to see how long it takes to get to 2v
    // RPi input pin goes high at approx 2v (~ 63% of 3.3v)
    for (i = 0; i < MAX_LDR_LOOPS/1000; i++) {
        ldr = digitalRead(LDR_PIN);
        if (ldr == 1)  break;
        delayMicroseconds(1000);
    }

    if (!ldr) {
        printf("*** Something wrong, LDR = 0!! ***\n");
        led_write_val (1);
        return;
    }

    clock_gettime (CLOCK_MONOTONIC, &endt);
    diff = BILLION * (endt.tv_sec - startt.tv_sec) + endt.tv_nsec - startt.tv_nsec;
    diff /= MILLION; // to get milli-seconds

    if (diff > LOW_LIGHT_CUTOFF) 
        led_write_val (1);
    else 
        led_write_val (0);

    if (fname[0] != '\0') {
        time (&rawtime);
        timeinfo = localtime (&rawtime);
//      strftime (buffer, 26, "%Y-%m-%d, %H:%M:%S", timeinfo);
        strftime (buffer, 26, "%H:%M:%S", timeinfo);

        fd = fopen (fname, "a");
        fprintf (fd, "%s,,,, %llu\n", buffer, (long long unsigned int) diff);
        fclose (fd);
    }
}

void dht11_read_val()
{
#define MAX_DHT11_LOOPS 85
    static int dht11_val[5]={0,0,0,0,0};
    static int last_dht11_val[5]={0,0,0,0,0};

    uint8_t lststate=HIGH;
    uint8_t counter=0;
    uint8_t j=0,i;

    for(i=0;i<5;i++)
        dht11_val[i]=0;
    pinMode(DHT11_PIN,OUTPUT);
    digitalWrite(DHT11_PIN,LOW);
    delay(18);
    digitalWrite(DHT11_PIN,HIGH);
    delayMicroseconds(40);
    pinMode(DHT11_PIN,INPUT);
    for(i=0;i<MAX_DHT11_LOOPS;i++)
    {
        counter=0;
        while(digitalRead(DHT11_PIN)==lststate){
            counter++;
            delayMicroseconds(1);
            if(counter==255)
                break;
        }
        lststate=digitalRead(DHT11_PIN);
        if(counter==255) break;

        // top 3 transistions are ignored
        if((i>=4)&&(i%2==0)){
            dht11_val[j/8]<<=1;
            if(counter>16)
                dht11_val[j/8]|=1;
            j++;
        }
    }
    // verify cheksum and print the verified data
    if ((j>=40) &&
        (dht11_val[4]==((dht11_val[0]+dht11_val[1]+dht11_val[2]+dht11_val[3])& 0xFF))) {
        printf("Humidity = %d.%d %% Temperature = %d.%d *C \n",
                   dht11_val[0], dht11_val[1], dht11_val[2], dht11_val[3]);
        if (!(dht11_val[0] == last_dht11_val[0] &&
            dht11_val[1] == last_dht11_val[1] &&
            dht11_val[2] == last_dht11_val[2] &&
            dht11_val[3] == last_dht11_val[3])) {
            last_dht11_val[0] = dht11_val[0];
            last_dht11_val[1] = dht11_val[1];
            last_dht11_val[2] = dht11_val[2];
            last_dht11_val[3] = dht11_val[3];
            if (fname[0] != '\0') {
                time (&rawtime);
                timeinfo = localtime (&rawtime);
//                strftime (buffer, 26, "%Y-%m-%d, %H:%M:%S", timeinfo);
                strftime (buffer, 26, "%H:%M:%S", timeinfo);

                fd = fopen (fname, "a");
                fprintf(fd, "%s, %d.%d, %d.%d\n", buffer,
                        dht11_val[0], dht11_val[1], dht11_val[2], dht11_val[3]);
                fclose (fd);
            }
        }
    } else {
        printf("Invalid Data!!\n");
    }
}

int main(int argc, char *argv[])
{
    printf("Interfacing Sensors With Raspberry Pi\n");
    if(wiringPiSetup()==-1)
        exit(1);

    fname[0] = '\0';
    if (argc > 1) {
        fd = fopen (argv[1], "a");
        if (fd) {
            strncpy (fname, argv[1], 29);
            fclose (fd);
        }
    }

    while(1)
    {
            dht11_read_val();
            pir_read_val();
            ldr_read_val();
    }
    return 0;
}
