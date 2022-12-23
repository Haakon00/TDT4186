Med all funksjonalitet er dette allerede et komplekst program, så det må testes grundig.
Design fire forskjellige testtilfeller som ikke bare tester en enkelt funksjonalitet (som å planlegge en alarm), men også en kombinasjon av funksjoner i programmet ditt.
Beskriv hvert testtilfelle: hva gjør du for å teste dette tilfellet, hva gjør testen, hva er forventet resultat?

Test 1 - schedule alarm and check that it has been scheduled
- what do you do to test this case - Kompile and run the program, write 's', click enter, write time to schedule alarm (yyyy-mm-dd hh:mm:ss), enter, write 'l', enter, chcek that there is Alarm 1 ....
- what does the test do - After kompiling and running the program we get the message: 'Welcome to the alarm clock! It is currently 2022-02-17 11:07:54 Please enter s (schedule), l (list), c (cancel), x (exit)', after writig s we get the message: 'Schedule alarm at which date and time?', after writing time we want to schedule alarm, example 2022-02-17 11:08:30, the program writes out what we typed in and in the example: '2022-02-17 11:08:30', the program also writes out 'Scheduling alarm in 36 seconds', where 36 is number of seconds to the alarm will go of from the time we started the program til scheduled alarm, the program writes 'New child: 18393' as well, where 18393 is ?, then the program writes out the start message again, but now with updated date and time, after writing l, the program writes out: 'Alarm 1: 2022-02-17 11:08:30'
- what is the expected result? -   

Test 2 - schedule 2 alarms and cancel the first one
- what do you do to test this case - 
- what does the test do - 
- what is the expected result? - 

Test 3 - schedule alarm in 1 minute and listen for the ringtone, check if alarm is listed after ringtone
- what do you do to test this case - 
- what does the test do - 
- what is the expected result? - 

Test 4 - try to scedule an alarm back in time and exit the program
- what do you do to test this case - 
- what does the test do - 
- what is the expected result? - 

Gjøre
- lage ringelyd
- lage 4 test caser
- skjekk så alarm ikke blir planlagt tilbake i tid (kan gjøres ved å ikke avtale i -sekunder)
- skjekke format når schedule alarm


--
Test 1 - schedule alarm
- what do you do to test this case - run the program, write s and write  2022-02-16 20:41:00,
- what does the test do - the program starts, user gets the start message, the program tels the user that alarm is scheduled in ... seconds 
- what is the expected result? - expect the alarm to be scheduled at 20:41:00, and the program giving response to the user in how many seconds til the alarm wil go of