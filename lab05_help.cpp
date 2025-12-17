#include "tpl_os.h"
#include "Arduino.h"

#define HAL_PIN 2 // D2 as input PIN

//   STATE VARIABLES
int prev_level = LOW;
static int level = LOW;

int high_ticks = 0;
int low_ticks = 0;

int pulse_count = 0;
int digit_error = 0;

//   Tick conversion (1.024ms per tick)
#define PULSE_MIN_TICKS 8  // 40 ms
#define PULSE_MAX_TICKS 12 // 60 ms

#define DIGIT_GAP_MIN 20 // 100 ms
#define DIGIT_GAP_MAX 39 // 200 ms

#define NUMBER_GAP_MIN 40 // >200 ms → number ends

void printDigit()
{
    char c;
    if (pulse_count == 10)
        c = '0';
    else
        c = '0' + pulse_count;

    Serial.print(c);

    if (pulse_count > 0 && digit_error)
        Serial.print("*");
}

void setup()
{
    Serial.begin(115200);
    pinMode(HAL_PIN, INPUT);
    Serial.println("Start!");

    StartOS(stdAppmode);
}

DeclareAlarm(a5msec);

TASK(TaskL)
{
    level = digitalRead(HAL_PIN);
    // Serial.print(level);
    if (level == HIGH)
    {
        high_ticks++;
        low_ticks = 0;
        // Serial.print(" high_ticks=");
        // Serial.println(high_ticks);
    }
    else
    {
        // LOW LEVEL
        low_ticks++;
        // Serial.print(" low_ticks=");
        // Serial.println(low_ticks);
        // Detect falling edge
        if (prev_level == HIGH)
        {
            // Pulse width violation (should be 40–60 ms)
            if (high_ticks < PULSE_MIN_TICKS || high_ticks > PULSE_MAX_TICKS)
                digit_error = 1;

            pulse_count++;
            // Serial.print("pulse_count=");
            // Serial.println(pulse_count);
            high_ticks = 0;
        }

        //     DIGIT GAP (100–200 ms)
        if (pulse_count > 0 &&
            low_ticks >= DIGIT_GAP_MIN &&
            low_ticks <= DIGIT_GAP_MAX)
        {
            // Serial.print("last digit:");
            printDigit();
            // Serial.println(' '); // digit terminator
            pulse_count = 0;
            digit_error = 0;
            low_ticks = 0;
        }

        // NUMBER GAP (>200 ms)
        if (low_ticks >= NUMBER_GAP_MIN)
        {
            // Serial.print("last digit:");
            Serial.println('/'); // number terminator
            Serial.print("next number:");
            pulse_count = 0;
            digit_error = 0;
            low_ticks = 0;
        }
    }

    prev_level = level;

    TerminateTask();
}
