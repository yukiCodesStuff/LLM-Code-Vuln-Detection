exif             = year4 ":" monthlz ":" daylz " " hour24lz ":" minutelz ":" secondlz;
firstdayof       = 'first day of';
lastdayof        = 'last day of';
backof           = 'back of ' hour24 (space? meridian)?;
frontof          = 'front of ' hour24 (space? meridian)?;

/* Common Log Format: 10/Oct/2000:13:55:36 -0700 */
clf              = day "/" monthabbr "/" year4 ":" hour24lz ":" minutelz ":" secondlz space tzcorrection;
