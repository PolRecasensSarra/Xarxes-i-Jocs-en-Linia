# Xarxes-i-Jocs-en-Linia
Students:
- Pol Recasens Sarrà
- Víctor Segura Blanco

Game: LaserSpace

LaserSpace is a multiplayer game made for the subject of Xarxes i Jocs en Línea of 4th year of Videogame Desing and Development Bachelor's Degree at CITM-UPC.
The game is about spaceships fighting in a one-for-all enviorement with powerups.

Controls:
KeyBoard:
- A/D: Rotate
- W: Advance
- Space: Shoot Laser
- F1: Toggle UI
- DEBUG KEY: E: Spawn 4 types of power ups to test

Controller:
- Joystick: Rotate
- Y: Advance
- X: Shoot Laser
- DEBUG KEY: R1: Spawn 4 types of power ups to test

List of Features:
Pol:
- UDP Virtual Connection -> Completed
- World State Replication -> Completed with known bug
  -> Bug: When there's a huge packet loss, the game could crush, probably caused by trying to read data now available.
- 1st Part Reliability-> Completed 
- Gameplay Elements (PowerUps, Audio)

Víctor:
- Delivery Manager -> Completed
  -> We don't resend lost packets
- Gameplay Elements (PowerUps, Random Generation of Asteroids and PowerUps)
