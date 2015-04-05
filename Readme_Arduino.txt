1. send temperature in Celcius each second. The temperature is of type double so it contains decimal point. A char 'S' is send before each temperature to indicate the start and "E\n" is appended at the end of the temperature to indicate the end of this sending.
i.e 30.1 will be send as "S31.1E\n".
The default mode is Celcius and the temp will be shown.
Use 'f' to turn the mode to Fahrenheit, notice only the display will changed, the temperature send to server is still in Celcius. 
To turn the mode back to Celcius, send 'c' from server.
To turn of the sending and display, send 's' from server.
To resume the sending and displya, send 'r' from the server.