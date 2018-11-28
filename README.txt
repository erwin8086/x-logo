This is a simple Xlib logo interpreter

basic usage: <command> <param>...
all angles are in degrees.

parameter types:
  number -> simple number(5, 10.5, 100, ...), variables(:a, :hello, :repcount, ...),
  functions(:hour(), :gt(5, 10), ... )
  strings -> "hello or [ hello world ], variables($a, $hello, ...)
  or functions($var("hello), $tostring(5), $cat([Hello ], "world!)
  command lists -> [ print 5 rt 90 ]

commands are:
  first it check for user defined commands

-> fd <number> : goes forward <number> pixels
-> bd <number> : goes backward <number> pixels
-> rt <number> : goes right <number> pixels
-> lt <number> : goes left <number> pixels
-> repeat <number> <command list>
  repeats <command list> <number> times.
  example: repeat 4 [ fd 100 rt 90 ]
-> fast : disable delay after each command
-> slow : enable delay after each command
-> clear : clears screen and return to 0,0
-> clearconsole : clears the console
-> sleep <number> : waits <number> secounds
-> pu : move the pen up, fd and bd do not paint in
   this mode
-> pd : move the pen down, fd and bd do paint in 
   this mode
-> print <number> : prints <number> to console
-> print <string> : prints <string> to cosnole
-> load <string> : loads the content of the file
   named <string> and executes it.
   example: load "examples/clock
-> to <string> <command list>
   defines the command <string> as <command list>
   example: to "circle [ repeat 36 [ fd 10 rt 10 ] ]

-> when <number> <command list>
   executes <command list> when <number> is not 0
-> while <number> <command list>
   executes <command list> as long as <number>
   is not 0
-> make <string> <number> : defines the variable
   <string> as <number>
-> push <number> : push <number> to stack
-> color <red> <green> <blue>: set color to the rgb code.
-> make$ <string: name> <string: value> : Set string variable $name to value
   example: make "hello [Hello world!] print $hello
-> say <string> : print <string> at current position in current color

functions:
-> :mod(<number 1>, <number 2>)
   performs a division of <number 1> / <number 2>
   and returns the remainder
-> :sin(<number>)
   returns the sinus of <number>
-> :cos(<number>)
   returns the cosinus of <number>
-> :random()
   returns a random number between 0 and 1
-> :random(<number>)
   returns a random number between 0 and <number>
-> :random(<number 1>, <number 2>
   returns a random number between <number 1> and
   <number 2>
-> :gt(<number 1>, <number 2>)
   returns 1 if <number 1> is greater than <number 2>
   in other cases it returns 0
-> :lt(<number 1>, <number 2)
   returns 1 if <number 1> is smaller than <number 2>
   in other cases it returns 0
-> :equ(<number 1>, <number 2>)
   returns 1 if <number 1> equlas <number 2>
   in other cases it returns 0
-> :not(<number>)
   returns 1 if <number> equals 0
   in other cases it returns 0
-> :int(<number>)
   returns number as integer: 5.1 = 5
-> :pop()
   returns last value pushed to stack or 0
-> :year()
   returns current year
-> :mon()
   returns current month
-> :day()
   returns day of month
-> :day(0)
   returns day of week
-> :day(1)
   returns day of month
-> :day(2)
   returns day of year
-> :hour()
   returns hour of day in 24 hour format
-> :min()
   returns minute of hour
-> :sec()
   returns secound of minute
-> the variable :repcount contains the iteration of the current repeat command.
-> :var(<string>)
   returns the variable named by <string>
-> :tonumber(<string>)
   converts <string> to a number: uses atof

string functions:
-> $var(<string>)
   returns the string variable named by <string>
-> $tostring(<number>)
   converts <number> to string
-> $int(<number>)
   converts <number> to string as int
-> $cat(<string: a>, <string: b>)
   concatenates a and b: returns <string: a><string: b> as string

examples for valid numbers:
-> 5
-> 5.50
-> :repcount
-> :a
-> :hour()


examples for valid strings:
-> "hello
-> [hello world]

examples for valid command list:
[fd 100 rt 90]
[clear]
[fd :s rt :a]
