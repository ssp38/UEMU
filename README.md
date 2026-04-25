# UEMU
UEMU is a OMD-Protocol UKC1 E-Bike/E-Scooter Display Emulator. All product rights go to UKRIVER, this software is simply a tool that I used to reverse engineer the OMD protocol.
<img width="2585" height="1604" alt="IMG_9931" src="https://github.com/user-attachments/assets/e34e62b0-02f0-42b9-8f9a-d5c55fb4705a" />

UEMU is capable of emulating:
- Error codes (all besides from communication error), including overlapping errors
- Brake Status / motor cutoff
- Watts used by the motor
- Speed

And can decode information the UKC1 sends to the bike controller, such as:
- Mode
- Amps / Throttle input (unsure of what that is)
- Motor Poles
- Start mode
- Speed limit (KM/h)
- Currennt Limit
- Throttle PWM
- Assist Poles

And can also create checksums and validate packets. 

In order to use UEMU on your own OMD UKC1, you'd need the following:
- CP210x USB to UART bridge board
- Power source (12v minimum, you can find one on amazon that uses AA batteries)
- JST-SM Connector Kit (or whatever your display uses)
- Parts, Tools, and Wires

See the following image for instructions.
<img width="3840" height="2036" alt="IMG_9934" src="https://github.com/user-attachments/assets/f68ce306-e399-4c66-a84c-9a9d970af48b" />

UEMU will not be updated, but instead be used to create a custom display which will be released in the near future, so stay tuned!
