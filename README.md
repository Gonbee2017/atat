# Auto Attack
usage:atat [*property*[=<i>VALUE</i>]...]

## WHAT IS ATAT?
ATAT is an interpreter for automatically manipulating
keyboard and mouse on Windows.
ATAT reads the script from standard input.
The script is described as a list of commands.
Basically, one operation is executed with one command.
The list of commands are executed in order from the top,
and when it runs to the bottom, it ends.
Press Ctrl-C to exit while running.

## PROPERTIES
Properties are specified by command line arguments.
Specify a name and if it has a value, connect with an equals.
In the following list, a name is indicated in lowercase,
and a value is indicated in uppercase.

* help<br/>
Show this document, and finish without doing anything.
* ready=<i>TIME</i><br/>
Wait for ready before running.
*TIME* is time to wait in milliseconds.
* repeat[=<i>NUMBER</i>]<br/>
Repeat the script.
*NUMBER* is number of repetition.
If it's not specified, it's infinite.
* silent<br/>
Don't show the commands to be executed.
* target=<i>CAPTION</i><br/>
*CAPTION* is the caption of the target window.
Execute the command when this window is active.
Also, specify the relative coordinates of this window
in 'mouse move' command.
If it's not specified, execute the command any window active.

## COMMANDS
Command consists of switches followed by parameters.
Separete switches and parameters with spaces or tabs.
In the following list, switches are indicated in lowercase,
and parameters are indicated in uppercase.
Also, if there're multiple choices on the switch,
separate them with vertical bars.

* key down|up|press *KEY*<br/>
Down or up or both the key on the keyboard.
*KEY* is the key to be operated.
Specify a string following 'DIK_' defined in DirectInput.
* mouse move *X* *Y*<br/>
Move the mouse.
*X* and *Y* are the coordinates to move the mouse.
If target is specified, assumed to be its relative coordinates.
* mouse wheel *AMOUNT*<br/>
Wheel the mouse.
*AMOUNT* is the amount to wheel the mouse.
* mouse left|right|middle down|up|click|doubleclick<br/>
Down or up or click or double click
the left or right or middle button of the mouse.
* sleep *TIME*<br/>
Do nothing for a while.
*TIME* is time to wait in milliseconds.
* loop begin [*NUMBER*]<br/>
Begin the loop block.
*NUMBER* is number of loops.
If it's not specified, it's infinite.
* loop end<br/>
End the loop block.

## EXAMPLE

### How to write?
At the command prompt, type:`$ atat target=GAME`

Since ATAT is waiting for input, it does as follows.
<pre>
sleep 3000
mouse move 100 200
mouse left click
loop begin 5
    key press A
    key press B
loop end
</pre>

### How it works
First, wait 3 seconds,
and then move the mouse pointer to (100,200) in the GAME window.
Next, click the left mouse button.
Then enter the loop block, press the A key, then press the B key.
Return to the beginning of the loop block,
and repeat the A and B five times.
If the GAME window becomes inactive halfway,
execution will be paused.
When it becomes active again, execution resumes.

## REMARKS

### Number format
Follow the literal syntax for number in C.
So, it's octal if it starts with '0',
or hexadecimal if it starts with '0x', otherwise decimal.

### Comment
Script can contains comments, for example:
<pre>
# Script01 to earn experience.

# Wait for ready.
sleep 3000

# Repeat infinitely.
loop begin
    key press TAB # Target an enemy.
    key press 1   # Attack the target.
loop end
</pre>
The string following the '#' is treated as a comment.
Also, empty rows just skip.
