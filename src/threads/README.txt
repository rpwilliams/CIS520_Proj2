alarm-mega.ck was made as a copy from alarm-multiple.ck and 7 was changed to 70
the line "4	alarm-mega" as added to Rubric.alarm
the line {"alarm-mega", test_alarm_mega}, was added to tests.c
alarm-mega was added to Make.tests
the line "extern test_func test_alarm_mega;" was added to tests.h

the code

void
test_alarm_mega (void)
{
	test_sleep (5,70);
}

was added to alarm-wait.c