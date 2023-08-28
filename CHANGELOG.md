**1.8.1**
- Make QR code a bit bigger on big displays
- Workaround partial display refresh on GDEM displays
- Don't send unnecessary HTTP headers

**1.8.0**
- Ensure compatibility with both 2.13 and 2.66 inch ePaper DEPG

**1.7.6**
- Detect when battery/no battery is connected from few/lots of VBAT (battery voltage) fluctuations.
- Display "NOBAT" instead of battery voltage when no battery is connected.

**1.7.2**
- Cleanly disconnect wifi before hibernate

**1.7.1**
- Show "Connecting to SSIDNAME..." at startup

**1.7.0**
- Show wifi signal strength (%)
- Reduce max font size of payment comments for esthetics
- Use partial display updates to increase speed and eliminate flickering

**1.6.0**
- Fix sat/sats plural
- Draw a line under the total sats amount
- Fix overflow for amounts bigger than 999999 sats
- Improve LOW BATTERY warning and cleanups

**1.5.0**
- Add health and status values to bottom right (battery voltage etc)
- Make sure second payment always fits
- Scale text based on available space
- Fix issue with empty comment payments (reported by Bitcoin3us)

**1.4.3**
- Speed up by eliminating unneeded lnbits URL call
- Speed up chunked reader with buffer
- Rewrite chunked HTTP parser
- Add update checker

**1.3.0**
- Add support for 2.13 inch DEPG ePaper board
- Add shake-to-update
- Add unified screen with total sats balance, LNURLp QR code for receiving, and 2 latest payments
- Add logo's at startup
