# EE120BCustomLab
120B Jukebox 
High level description of custom lab:
The microprocessor takes in button inputs from PORT A which will change the outputs 
on the LCD, speaker and LEDs. When playing a song the LEDs will play a special sequence 
for each song. The LCD will display different things depending on the state that it is 
currently in. The speaker will output different frequencies to play the songs that the users 
want. There is a welcome state that will also list the possible song options. Depending on the 
input of the buttons the microprocessor will decide what the LCD displays, LEDs display and 
the speakers output. The LCD is connected to PORTC and PD6 and PD7. The speaker is connected 
to PORTB while the LEDs are connected to both PORTB and PORTD. PORTA is the only input while PORTB,
C and D are all outputs. The only input the microprocessor is recieving is from the buttons while 
the outputs is going to the LCD, speaker and LEDs.

User guide:
The LCD initial displays a Welcome screen which request the user to push any button. It will then
list the first list of songs that you can play. If you press the first button it will play jingle
bells, the second button will play Mary had a little lamb and the third one will play I love you 
by barney. The fourth button will then go through the list again. The last will take you to the 
next list of songs. If you press the first button in the new list then it will play Twinkle Twinkle 
little star and if you press the second on it will take you to row row row your boat. The third 
button will repeat the list that you are currently in a and the fourth button will take you to the
pervious list. Nothing will happen if you press the fifth button or if you press multiple buttons 
at the same time. When you are in the song and it is currently playing if you press any of the first
three buttons in the first list nothing will happen. If you press the 4th button then it will stop 
the song and go back to the welcome screen. If you press the 5th button it will take you to the next 
song acting as a skip button. Once it reaches row row row your boat it will go back to the welcome screen.
Rules: 
You can’t press two buttons at the same time. You can’t play two songs at the same time. By holding 
down skip and not letting go it will take you back to the welcome screen. The LEDs will display a 
different sequence while playing the song and will be all off if no song is being played.

Technologies and components used in custom lab:
The technologies that we used are the AVR studio 6 which is where we wrote the C code, which we then compiled and pushed it to the board. We also used the ATmega1284, debugger, LEDs, resistors, 
potentiometer. We also used a speaker, buttons, LCD, breadboard and microprocessor.
Which were all provided by IEEE.
