#define d1 13
#define d2 12
#define d3 11
#define d4 10

#define num_tasks 10

typedef struct sched_task_t
{
        int period;         // period in ticks
        int delay;          // ticks until next activation
        void (*func)(void); // function pointer
        int exec;           // activation counter
} sched_task_t;

// Global Variables
sched_task_t tasks[num_tasks];
uint8_t pin_status = 0;

void sched_schedule(void)
{
        for (int x = 0; x < num_tasks; x++)
        {
                if (!tasks[x].func)
                {
                        continue;
                }

                if (tasks[x].delay != 0)
                {
                        tasks[x].delay--;
                }
                else
                { // schedule task
                        tasks[x].exec++;
                        tasks[x].delay = tasks[x].period - 1;
                }
        }
}

vo4id sched_add_t(void (*f)(void), int delay, int period)
{
        for (int i = 0; i < num_tasks; i++)
        {
                if (!tasks[i].func)
                {
                        tasks[i].period = period;
                        tasks[i].delay = delay;
                        tasks[i].exec = 0;
                        tasks[i].func = f;
                        return i;
                }
        }
        return -1;
}

void sched_dispatch(void)
{
        for (int x = 0; x < num_tasks; x++)
        {
                if (!(tasks[x].func && tasks[x].exec))
                {
                        continue;
                }

                tasks[x].exec = 0;
                tasks[x].func();
                /* Delete task if one-shot */
                if (!tasks[x].period)
                {
                        tasks[x].func = 0;
                }
        }
}

int sched_init(void)
{
        for (int i = 0; i < num_tasks; i++)
        {
                tasks[i].func = 0;
        }

        noInterrupts();

        TCCR1A = 0;
        TCCR1B = 0;
        TCNT1 = 0;

        OCR1A = 31;
        TCCR1B |= (1 << WGM12);
        TCCR1B |= (1 << CS12);
        TIMSK1 |= (1 << OCIE1A);

        interrupts();
}

ISR(TIMER1_COMPA_vect) { sched_schedule(); }

void toggle() { digitalWrite(d4, !digitalRead(d4)); }

void turn_off() { digitalWrite(d4, 1); }

void turn_on() { digitalWrite(d4, 0); }

void task_1()
{
        if (digitalRead(A1) != pin_status)
        {
                sched_add_t(toggle, 1000, 0);
        }
        pin_status = digitalRead(A1);
}

void setup()
{
        pinMode(d4, OUTPUT);
        pinMode(d3, !OUTPUT);
        pinMode(d2, !OUTPUT);
        pinMode(d1, !OUTPUT);

        sched_init();

        sched_add_t(task_1, 1, 20);
}

void loop() { sched_dispatch(); }