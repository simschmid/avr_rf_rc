# avr_rf_rc
avr library for receiving and sending rf codes

Five common protocols of RC-Remotes are included (as defined by rc-switch - https://github.com/sui77/rc-switch).

The code is written for avr atmega88p, but it should be easy to port to other µCs.
The receiver-routine needs one Timer in CTC mode.


For example-usage look at main.c
Feel free to look at the source - it's well documented.

