To build:
''
west build -p -b nrf52840dk/nrf52840
''
west build -p -b capstonepcb -- -DBOARD_ROOT=.

moved board file to home
west build -p -b capstonepcb -- -DBOARD_ROOT=~