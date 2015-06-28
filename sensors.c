#include <wiringPi.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define PIR_PIN 0
#define LM393_SOUND_PIN 2
#define DHT11_PIN 1
#define LDR_PIN 4
#define LED_PIN 7
#define RELAY1_PIN 5
#define RELAY2_PIN 6

#define NIGHT_LIGHT_DELAY 15

#define BILLION 1000000000L
#define MILLION 1000000L

FILE *fd = NULL;
char fname [30];
time_t rawtime;
struct tm *timeinfo;
char buffer[32];

// #define USE_LED_FOR_NIGHT_LIGHT 0

#ifdef USE_LED_FOR_NIGHT_LIGHT
#define night_light_on() led_write_val(1)
#define night_light_off() led_write_val(0)
#else
#define night_light_on() relay_write_val(RELAY1_PIN, 1)
#define night_light_off() relay_write_val(RELAY1_PIN, 0)
#endif

void relay_write_val(int relay, int val)
{
    pinMode (relay, OUTPUT);
    digitalWrite (relay, val);
}

int relay_read_val(int relay)
{
    pinMode (relay, OUTPUT);
    return digitalRead (relay);
}

void led_write_val(int val)
{
    pinMode (LED_PIN, OUTPUT);
    digitalWrite (LED_PIN, val);
}

// Shared variable between the lm393 thread and main
int last_lm393_sound_val = 1;

// Following function runs in a separate thread; returns 0 on sound
int lm393_thread_sound_read_val()
{
    return digitalRead(LM393_SOUND_PIN);
}

// Following function runs in a separate thread
void *lm393_thread (void *none)
{
    // initialize LM393 PIN as INPUT
    pinMode (LM393_SOUND_PIN, INPUT);
    while (1) {
        if (0 == last_lm393_sound_val) {
            // sound detected, but not read by main thread. stop polling
            delay (100); // sleep for 0.1 second
        } else if (0 == lm393_thread_sound_read_val()) {
            // if sound detected, set global variable so main thread can read it
            last_lm393_sound_val = 0;
        }
    }
}

// these functions runs in main thread; 
void launch_lm393_thread ()
{
    pthread_t thread_id;
    pthread_create (&thread_id, NULL, &lm393_thread, NULL);
}

// reads whether a sound has been detected and clears it
int lm393_sound_read_val()
{
    int tmp_val = last_lm393_sound_val;
    if (0 == tmp_val) {
        // a sound was detected
        if (fname[0] != '\0') {
            time (&rawtime);
            timeinfo = localtime (&rawtime);
            strftime (buffer, 26, "%H:%M:%S", timeinfo);

            fd = fopen (fname, "a");
            fprintf(fd, "%s,,,,, %d\n", buffer, tmp_val);
            fclose (fd);
        }
        // reset the last read
        last_lm393_sound_val = 1;
    }
    return tmp_val;
}

int pir_read_val()
{
    static int last_pir_val=2;

    int pir_val=0;
    uint8_t lststate=HIGH;
    uint8_t counter=0;
    uint8_t j=0,i;

    pir_val = digitalRead(PIR_PIN);
    // verify cheksum and print the verified data
    printf("%d ", pir_val); 
    last_pir_val = pir_val;
    if (pir_val != 0) {
        if (fname[0] != '\0') {
            time (&rawtime);
            timeinfo = localtime (&rawtime);
            strftime (buffer, 26, "%H:%M:%S", timeinfo);

            fd = fopen (fname, "a");
            fprintf(fd, "%s,,, %d\n", buffer, pir_val);
            fclose (fd);
        }
    }
    return last_pir_val;
}


int ldr_read_val()
{
#define MAX_LDR_LOOPS 100000
// tested in the room with lights off
// for day time testing, use lower values such as 5
#define LOW_LIGHT_CUTOFF 10

    static int last_ldr_diff = 2000;

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
        diff = 1000;
    } else {
        clock_gettime (CLOCK_MONOTONIC, &endt);
        diff = BILLION * (endt.tv_sec - startt.tv_sec) + endt.tv_nsec - startt.tv_nsec;
        diff /= MILLION; // to get milli-seconds
    }

    if (diff != last_ldr_diff && fname[0] != '\0') {
        last_ldr_diff = diff;
        time (&rawtime);
        timeinfo = localtime (&rawtime);
//      strftime (buffer, 26, "%Y-%m-%d, %H:%M:%S", timeinfo);
        strftime (buffer, 26, "%H:%M:%S", timeinfo);

        fd = fopen (fname, "a");
        fprintf (fd, "%s,,,, %llu\n", buffer, (long long unsigned int) diff);
        fclose (fd);
    }
    return last_ldr_diff;
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

void night_light_oper (int last_ldr_diff, int last_pir_val)
{
    static struct timespec nl_startt, currentt;
    static int clap_on = 0;

    // clap is ON and OFF on alternate CLAPs; turn LED on/off on claps
    if (clap_on) {
       if (0 == lm393_sound_read_val()) {
           clap_on = 0; led_write_val (0);
       }
    } else {
       if (0 == lm393_sound_read_val()) {
           clap_on = 1; led_write_val (1);
       }
    }
    
    /*
     * switch off night-light if 
     * there's sufficient light i.e. (last_ldr_diff <= LDR_LIGHT_CUTOFF) OR 
     * no movement for NIGHT_LIGHT_DELAY # seconds
     */
    if (last_ldr_diff > LOW_LIGHT_CUTOFF) {
       if (last_pir_val) {
           night_light_on();
           clock_gettime (CLOCK_MONOTONIC, &nl_startt);
       } else {
           clock_gettime (CLOCK_MONOTONIC, &currentt);
           if (currentt.tv_sec > nl_startt.tv_sec + NIGHT_LIGHT_DELAY)
               night_light_off();
       }
    } else {
       night_light_off();
    }
}

int main(int argc, char *argv[])
{
    int last_pir_val, last_ldr_diff;

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

    night_light_off ();

    launch_lm393_thread ();
    while(1)
    {
        // read all sensors */
        dht11_read_val();
        last_pir_val = pir_read_val();
        last_ldr_diff = ldr_read_val();

        night_light_oper (last_ldr_diff, last_pir_val);
    }
    return 0;
}
